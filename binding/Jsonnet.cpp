// SPDX-License-Identifier: MIT
#include <cassert>
#include <condition_variable>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include "Jsonnet.hpp"
#include "JsonnetWorker.hpp"

namespace nodejsonnet {

  Napi::FunctionReference Jsonnet::constructor;

  Napi::Object Jsonnet::init(Napi::Env env) {
    auto const func = DefineClass(env, "Jsonnet", {
        StaticAccessor("version", &Jsonnet::getVersion, nullptr),
        InstanceMethod("evaluateFile", &Jsonnet::evaluateFile),
        InstanceMethod("evaluateSnippet", &Jsonnet::evaluateSnippet),
        InstanceMethod("extString", &Jsonnet::extString),
        InstanceMethod("extCode", &Jsonnet::extCode),
        InstanceMethod("tlaString", &Jsonnet::tlaString),
        InstanceMethod("tlaCode", &Jsonnet::tlaCode),
        InstanceMethod("addJpath", &Jsonnet::addJpath),
        InstanceMethod("nativeCallback", &Jsonnet::nativeCallback),
      });

    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();
    return func;
  }

  Jsonnet::Jsonnet(const Napi::CallbackInfo &info)
    : Napi::ObjectWrap<Jsonnet>(info) {
  }

  Napi::Value Jsonnet::getVersion(const Napi::CallbackInfo& info) {
    auto const env = info.Env();
    return Napi::String::New(env, ::jsonnet_version());
  }

  Napi::Value Jsonnet::evaluateFile(const Napi::CallbackInfo& info) {
    auto const env = info.Env();
    auto const filename = info[0].As<Napi::String>().Utf8Value();

    auto vm = createVm(env);
    auto const worker = new JsonnetWorker(env, vm, std::make_unique<JsonnetWorker::EvaluateFileOp>(filename));
    auto const promise = worker->Promise();
    worker->Queue();
    return promise;
  }

  Napi::Value Jsonnet::evaluateSnippet(const Napi::CallbackInfo& info) {
    auto const env = info.Env();
    auto const snippet = info[0].As<Napi::String>().Utf8Value();
    auto const filename = info.Length() < 2 ? "(snippet)" : info[1].As<Napi::String>().Utf8Value();

    auto vm = createVm(env);
    auto const worker = new JsonnetWorker(env, vm, std::make_unique<JsonnetWorker::EvaluateSnippetOp>(snippet, filename));
    auto const promise = worker->Promise();
    worker->Queue();
    return promise;
  }

  Napi::Value Jsonnet::extString(const Napi::CallbackInfo& info) {
    auto const key = info[0].As<Napi::String>().Utf8Value();
    auto const val = info[1].As<Napi::String>().Utf8Value();

    ext.push_back({false, key, val});

    return info.This();
  }

  Napi::Value Jsonnet::extCode(const Napi::CallbackInfo& info) {
    auto const key = info[0].As<Napi::String>().Utf8Value();
    auto const val = info[1].As<Napi::String>().Utf8Value();

    ext.push_back({true, key, val});

    return info.This();
  }

  Napi::Value Jsonnet::tlaString(const Napi::CallbackInfo& info) {
    auto const key = info[0].As<Napi::String>().Utf8Value();
    auto const val = info[1].As<Napi::String>().Utf8Value();

    tla.push_back({false, key, val});

    return info.This();
  }

  Napi::Value Jsonnet::tlaCode(const Napi::CallbackInfo& info) {
    auto const key = info[0].As<Napi::String>().Utf8Value();
    auto const val = info[1].As<Napi::String>().Utf8Value();

    tla.push_back({true, key, val});

    return info.This();
  }

  Napi::Value Jsonnet::addJpath(const Napi::CallbackInfo& info) {
    auto const path = info[0].As<Napi::String>().Utf8Value();

    jpath.push_back(path);

    return info.This();
  }

  namespace {

    Napi::Value toNapiValue(Napi::Env const &env, std::shared_ptr<JsonnetVm> vm, JsonnetJsonValue const *json) {
      if(vm->extractJsonNull(json)) {
        return env.Null();
      }
      if(auto const b = vm->extractJsonBool(json); b) {
        return Napi::Boolean::New(env, *b);
      }
      if(auto const n = vm->extractJsonNumber(json); n) {
        return Napi::Number::New(env, *n);
      }
      if(auto const s = vm->extractJsonString(json); s) {
        return Napi::String::New(env, s->data(), s->size());
      }

      return env.Undefined();
    }

    JsonnetJsonValue *toJsonnetJson(std::shared_ptr<JsonnetVm> vm, Napi::Value v) {
      if(v.IsBoolean()) {
        return vm->makeJsonBool(v.As<Napi::Boolean>());
      }
      if(v.IsNumber()) {
        return vm->makeJsonNumber(v.As<Napi::Number>());
      }
      if(v.IsString()) {
        return vm->makeJsonString(v.As<Napi::String>().Utf8Value().c_str());
      }
      return vm->makeJsonNull();
    }

    struct TsfnWrap {
      TsfnWrap(Napi::ThreadSafeFunction &&tsfn): tsfn{tsfn} {
      }

      ~TsfnWrap() {
        if(tsfn) {
          tsfn.Release();
        }
      }

      TsfnWrap(TsfnWrap const &other) {
        *this = other;
      }

      TsfnWrap(TsfnWrap &&other) {
        *this = std::move(other);
      }

      TsfnWrap &operator=(TsfnWrap const &other) {
        tsfn = other.tsfn;
        if(tsfn) {
          tsfn.Acquire();
        }
        return *this;
      }

      TsfnWrap &operator=(TsfnWrap &&other) {
        tsfn = other.tsfn;
        other.tsfn = {};
        return *this;
      }

      Napi::ThreadSafeFunction const *operator->() const {
        return &tsfn;
      }

    private:
      Napi::ThreadSafeFunction tsfn;
    };

  }

  Napi::Value Jsonnet::nativeCallback(const Napi::CallbackInfo &info) {
    auto const name = info[0].As<Napi::String>().Utf8Value();
    auto const fun = info[1].As<Napi::Function>();
    auto const params =
      [&]{
        std::vector<std::string> params;
        for(size_t i = 2; i < info.Length(); ++i) {
          params.push_back(std::move(info[i].As<Napi::String>().Utf8Value()));
        }
        return params;
      }();

    nativeCallbacks.push_back({name, Napi::Persistent(fun), std::move(params)});

    return info.This();
  }

  std::shared_ptr<JsonnetVm> Jsonnet::createVm(Napi::Env const &env) {
    auto vm = JsonnetVm::make();

    for(auto const &x: ext) {
      if(x.isCode) {
        vm->extCode(x.key, x.value);
      } else {
        vm->extVar(x.key, x.value);
      }
    }

    for(auto const &x: tla) {
      if(x.isCode) {
        vm->tlaCode(x.key, x.value);
      } else {
        vm->tlaVar(x.key, x.value);
      }
    }

    for(auto const &x: jpath) {
      vm->jpathAdd(x);
    }

    struct Payload {
      Payload(std::shared_ptr<JsonnetVm> vm, std::vector<JsonnetJsonValue const *> &&args)
        : m{}, cv{}, args{std::move(args)}, vm{vm}, result{nullptr} {
      }

      std::vector<JsonnetJsonValue const *> const &getArgs() const {
        return args;
      }

      std::shared_ptr<JsonnetVm> getVm() const {
        return vm;
      }

      void setResult(JsonnetJsonValue *value) {
        {
          std::lock_guard<std::mutex> lk(m);
          result = value;
        }
        cv.notify_one();
      }

      JsonnetJsonValue *getResult() {
        std::unique_lock<std::mutex> lk(m);
        cv.wait(lk, [this] { return result; });
        return result;
      }

    private:
      std::mutex m;
      std::condition_variable cv;
      std::vector<JsonnetJsonValue const *> args;
      std::shared_ptr<JsonnetVm> vm;
      JsonnetJsonValue *result;
    };

    for(auto const &x: nativeCallbacks) {
      auto const name = x.name;
      auto const fun = x.fun;
      auto const params = x.params;

      TsfnWrap tsfn = Napi::ThreadSafeFunction::New(
        env,
        *fun,
        "jsonnet native callback",
        0,
        1
        );

      auto callback =
        [](Napi::Env env, Napi::Function fun, Payload *payload) {
          // This functions runs in the Node main thread.

          auto const vm = payload->getVm();

          std::vector<napi_value> args;
          args.reserve(payload->getArgs().size());
          for(auto const arg: payload->getArgs()) {
            args.push_back(toNapiValue(env, vm, arg));
          }

          auto const result = toJsonnetJson(vm, fun.Call(args));
          payload->setResult(result);
        };

      auto const native_callback =
        [=, tsfn = std::move(tsfn)](std::shared_ptr<JsonnetVm> &&vm, std::vector<JsonnetJsonValue const *> &&args) {
          // This functions runs in a worker thread and cannot access Node VM.

          Payload payload(std::move(vm), std::move(args));
          tsfn->BlockingCall(&payload, callback);
          return payload.getResult();
        };

      vm->nativeCallback(name, std::move(native_callback), params);
    }

    return vm;
  }
}
