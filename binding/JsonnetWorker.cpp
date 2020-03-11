// SPDX-License-Identifier: MIT
#include <memory>
#include <string_view>
#include <utility>
#include "JsonnetWorker.hpp"

namespace nodejsonnet {

  JsonnetWorker::JsonnetWorker(Napi::Env env, std::shared_ptr<JsonnetVm> vm, std::unique_ptr<Op> op)
    : Napi::AsyncWorker(env), vm(std::move(vm)), op(std::move(op)),
      deferred(Napi::Promise::Deferred::New(env)) {}

  void JsonnetWorker::Execute() {
    result = op->execute(*vm);
  }

  void JsonnetWorker::OnOK() {
    deferred.Resolve(op->toValue(Env(), std::move(result)));
  }

  void JsonnetWorker::OnError(Napi::Error const &error) {
    deferred.Reject(error.Value());
  }

  JsonnetWorker::EvaluateFileOp::EvaluateFileOp(std::string filename)
    : filename(std::move(filename)) {}

  JsonnetVm::Buffer JsonnetWorker::EvaluateFileOp::execute(JsonnetVm const &vm) {
    return vm.evaluateFile(filename);
  }

  Napi::Value JsonnetWorker::EvaluateFileOp::toValue(Napi::Env env, JsonnetVm::Buffer buffer) {
    return Napi::String::New(env, buffer.get());
  }

  JsonnetWorker::EvaluateSnippetOp::EvaluateSnippetOp(std::string snippet, std::string filename)
    : snippet(std::move(snippet)), filename(std::move(filename)) {}

  JsonnetVm::Buffer JsonnetWorker::EvaluateSnippetOp::execute(JsonnetVm const &vm) {
    return vm.evaluateSnippet(filename, snippet);
  }

  Napi::Value JsonnetWorker::EvaluateSnippetOp::toValue(Napi::Env env, JsonnetVm::Buffer buffer) {
    return Napi::String::New(env, buffer.get());
  }

  namespace {
    Napi::Value parseMultiValue(Napi::Env env, JsonnetVm::Buffer buffer) {
      auto result = Napi::Object::New(env);

      for(auto p = buffer.get(); *p; ) {
        std::string_view const name(p);
        p += name.size() + 1;
        std::string_view const json(p);
        p += json.size() + 1;

        result.Set(Napi::String::New(env, name.data(), name.size()),
                   Napi::String::New(env, json.data(), json.size()));
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

  Napi::Value JsonnetWorker::EvaluateSnippetMultiOp::toValue(Napi::Env env, JsonnetVm::Buffer buffer) {
    return parseMultiValue(env, std::move(buffer));
  }

  namespace {
    Napi::Value parseStreamValue(Napi::Env env, JsonnetVm::Buffer buffer) {
      std::vector<napi_value> jsons;
      for(auto p = buffer.get(); *p; ) {
        std::string_view const json(p);
        p += json.size() + 1;
        jsons.push_back(Napi::String::New(env, json.data(), json.size()));
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

  Napi::Value JsonnetWorker::EvaluateFileStreamOp::toValue(Napi::Env env, JsonnetVm::Buffer buffer) {
    return parseStreamValue(env, std::move(buffer));
  }

  JsonnetVm::Buffer JsonnetWorker::EvaluateSnippetStreamOp::execute(JsonnetVm const &vm) {
    return vm.evaluateSnippetStream(filename, snippet);
  }

  Napi::Value JsonnetWorker::EvaluateSnippetStreamOp::toValue(Napi::Env env, JsonnetVm::Buffer buffer) {
    return parseStreamValue(env, std::move(buffer));
  }

}
