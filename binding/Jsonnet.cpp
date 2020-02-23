// SPDX-License-Identifier: MIT
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
      });

    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();
    return func;
  }

  Jsonnet::Jsonnet(const Napi::CallbackInfo &info):
    Napi::ObjectWrap<Jsonnet>(info), vm() {
  }

  Napi::Value Jsonnet::getVersion(const Napi::CallbackInfo& info) {
    auto const env = info.Env();
    return Napi::String::New(env, ::jsonnet_version());
  }

  Napi::Value Jsonnet::evaluateFile(const Napi::CallbackInfo& info) {
    auto const env = info.Env();
    auto const filename = info[0].As<Napi::String>();

    auto const worker = new JsonnetWorker(env, vm, std::make_unique<JsonnetWorker::EvaluateFileOp>(filename));
    auto const promise = worker->Promise();
    worker->Queue();
    return promise;
  }

  Napi::Value Jsonnet::evaluateSnippet(const Napi::CallbackInfo& info) {
    auto const env = info.Env();
    auto const snippet = info[0].As<Napi::String>();
    auto const filename = info.Length() < 2 ? "(snippet)" : info[1].As<Napi::String>().Utf8Value();

    auto worker = new JsonnetWorker(env, vm, std::make_unique<JsonnetWorker::EvaluateSnippetOp>(snippet, filename));
    auto promise = worker->Promise();
    worker->Queue();
    return promise;
  }

  Napi::Value Jsonnet::extString(const Napi::CallbackInfo& info) {
    auto const env = info.Env();
    auto const key = info[0].As<Napi::String>();
    auto const val = info[1].As<Napi::String>();

    vm.extVar(key, val);

    return env.Undefined();
  }

  Napi::Value Jsonnet::extCode(const Napi::CallbackInfo& info) {
    auto const env = info.Env();
    auto const key = info[0].As<Napi::String>();
    auto const val = info[1].As<Napi::String>();

    vm.extCode(key, val);

    return env.Undefined();
  }

  Napi::Value Jsonnet::tlaString(const Napi::CallbackInfo& info) {
    auto const env = info.Env();
    auto const key = info[0].As<Napi::String>();
    auto const val = info[1].As<Napi::String>();

    vm.tlaVar(key, val);

    return env.Undefined();
  }

  Napi::Value Jsonnet::tlaCode(const Napi::CallbackInfo& info) {
    auto const env = info.Env();
    auto const key = info[0].As<Napi::String>();
    auto const val = info[1].As<Napi::String>();

    vm.tlaCode(key, val);

    return env.Undefined();
  }

  Napi::Value Jsonnet::addJpath(const Napi::CallbackInfo& info) {
    auto const env = info.Env();
    auto const path = info[0].As<Napi::String>();

    vm.jpathAdd(path);

    return env.Undefined();
  }

}
