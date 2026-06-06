// SPDX-License-Identifier: MIT
#include "JsonnetWorker.hpp"
#include <memory>
#include <string_view>
#include <utility>
#include <vector>
#include "JsonnetAddon.hpp"
#include "JsonnetImportCallback.hpp"
#include "JsonnetNativeCallback.hpp"

namespace nodejsonnet {

  JsonnetWorker::JsonnetWorker(Napi::Env env, JsonnetVmParam const &param, std::unique_ptr<Op> op)
    : Napi::AsyncWorker(env, "jsonnet"), vm(createVm(env, param)), op(std::move(op)),
      deferred(Napi::Promise::Deferred::New(env)) {
  }

  std::shared_ptr<JsonnetVm> JsonnetWorker::createVm(
    Napi::Env const &env, JsonnetVmParam const &param) {
    auto vm = JsonnetVm::make();

    if(param.maxStack) {
      vm->maxStack(*param.maxStack);
    }
    if(param.maxTrace) {
      vm->maxTrace(*param.maxTrace);
    }
    if(param.gcMinObjects) {
      vm->gcMinObjects(*param.gcMinObjects);
    }
    if(param.gcGrowthTrigger) {
      vm->gcGrowthTrigger(*param.gcGrowthTrigger);
    }
    vm->stringOutput(param.stringOutput);
    vm->trailingNewline(param.trailingNewline);

    for(auto const &[name, var]: param.ext) {
      if(var.isCode) {
        vm->extCode(name, var.value);
      } else {
        vm->extVar(name, var.value);
      }
    }

    for(auto const &[name, var]: param.tla) {
      if(var.isCode) {
        vm->tlaCode(name, var.value);
      } else {
        vm->tlaVar(name, var.value);
      }
    }

    for(auto const &x: param.jpath) {
      vm->jpathAdd(x);
    }

    for(auto const &[name, cb]: param.nativeCallbacks) {
      auto const &fun = cb.fun;
      auto const &params = cb.params;

      vm->addNativeCallback(
        name,
        [this, callback = std::make_shared<JsonnetNativeCallback>(env, fun.Value())](
          std::shared_ptr<JsonnetVm> vm, std::vector<JsonnetJsonValue const *> args) {
          try {
            return callback->call(std::move(vm), std::move(args));
          } catch(CallbackError &e) {
            this->jsError = std::move(e.jsError);
            throw;
          }
        },
        params);
    }

    if(param.importCallbackParam) {
      vm->setImportCallback(
        [this, callback = std::make_shared<JsonnetImportCallback>(
                 env, param.importCallbackParam->fun.Value())](
          std::shared_ptr<JsonnetVm> vm, std::string const &base, std::string const &rel) {
          try {
            return callback->call(std::move(vm), base, rel);
          } catch(CallbackError &e) {
            this->jsError = std::move(e.jsError);
            throw;
          }
        });
    }

    return vm;
  }

  void JsonnetWorker::Execute() {
    try {
      result = op->execute(*vm);
    } catch(JsonnetError const &e) {
      errorType = ErrorType::Jsonnet;
      throw;
    }
  }

  void JsonnetWorker::OnOK() {
    deferred.Resolve(op->toValue(Env(), *std::exchange(result, std::nullopt)));
  }

  void JsonnetWorker::OnError(Napi::Error const &error) {
    auto e = error.Value();

    switch(errorType) {
    case ErrorType::Generic:
      break;
    case ErrorType::Jsonnet: {
      auto const env = e.Env();
      auto const ctor =
        JsonnetAddon::getInstance(env).getExport("JsonnetError").As<Napi::Function>();
      auto options = Napi::Object::New(env);
      if(jsError) {
        options.Set("cause", jsError->Value());
      }
      e = ctor.New({e.Get("message"), options});
      break;
    }
    default:
      abort();  // unreachable
    }

    deferred.Reject(e);
  }

  JsonnetWorker::EvaluateFileOp::EvaluateFileOp(std::string filename)
    : filename(std::move(filename)) {
  }

  JsonnetVm::Buffer JsonnetWorker::EvaluateFileOp::execute(JsonnetVm const &vm) {
    return vm.evaluateFile(filename);
  }

  Napi::Value JsonnetWorker::EvaluateFileOp::toValue(Napi::Env env, JsonnetVm::Buffer buffer) {
    return Napi::String::New(env, buffer.get());
  }

  JsonnetWorker::EvaluateSnippetOp::EvaluateSnippetOp(std::string snippet, std::string filename)
    : snippet(std::move(snippet)), filename(std::move(filename)) {
  }

  JsonnetVm::Buffer JsonnetWorker::EvaluateSnippetOp::execute(JsonnetVm const &vm) {
    return vm.evaluateSnippet(filename, snippet);
  }

  Napi::Value JsonnetWorker::EvaluateSnippetOp::toValue(Napi::Env env, JsonnetVm::Buffer buffer) {
    return Napi::String::New(env, buffer.get());
  }

  namespace {
    Napi::Value parseMultiValue(Napi::Env env, JsonnetVm::Buffer buffer) {
      auto result = Napi::Object::New(env);
      result.Set("__proto__", env.Null());  // TODO: Use node_api_set_prototype when stabilized.

      for(auto p = buffer.get(); *p;) {
        std::string_view const name(p);
        p += name.size() + 1;
        std::string_view const json(p);
        p += json.size() + 1;

        result.Set(Napi::String::New(env, name), Napi::String::New(env, json));
      }

      return result;
    }
  }

  JsonnetVm::Buffer JsonnetWorker::EvaluateFileMultiOp::execute(JsonnetVm const &vm) {
    return vm.evaluateFileMulti(filename);
  }

  Napi::Value JsonnetWorker::EvaluateFileMultiOp::toValue(Napi::Env env, JsonnetVm::Buffer buffer) {
    return parseMultiValue(env, std::move(buffer));
  }

  JsonnetVm::Buffer JsonnetWorker::EvaluateSnippetMultiOp::execute(JsonnetVm const &vm) {
    return vm.evaluateSnippetMulti(filename, snippet);
  }

  Napi::Value JsonnetWorker::EvaluateSnippetMultiOp::toValue(
    Napi::Env env, JsonnetVm::Buffer buffer) {
    return parseMultiValue(env, std::move(buffer));
  }

  namespace {
    Napi::Value parseStreamValue(Napi::Env env, JsonnetVm::Buffer buffer) {
      std::vector<napi_value> jsons;
      for(auto p = buffer.get(); *p;) {
        std::string_view const json(p);
        p += json.size() + 1;
        jsons.push_back(Napi::String::New(env, json));
      }

      auto result = Napi::Array::New(env, jsons.size());
      for(size_t i = 0; i < jsons.size(); ++i) {
        result.Set(i, jsons[i]);
      }
      return result;
    }
  }

  JsonnetVm::Buffer JsonnetWorker::EvaluateFileStreamOp::execute(JsonnetVm const &vm) {
    return vm.evaluateFileStream(filename);
  }

  Napi::Value JsonnetWorker::EvaluateFileStreamOp::toValue(
    Napi::Env env, JsonnetVm::Buffer buffer) {
    return parseStreamValue(env, std::move(buffer));
  }

  JsonnetVm::Buffer JsonnetWorker::EvaluateSnippetStreamOp::execute(JsonnetVm const &vm) {
    return vm.evaluateSnippetStream(filename, snippet);
  }

  Napi::Value JsonnetWorker::EvaluateSnippetStreamOp::toValue(
    Napi::Env env, JsonnetVm::Buffer buffer) {
    return parseStreamValue(env, std::move(buffer));
  }

}
