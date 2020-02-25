// SPDX-License-Identifier: MIT
#include <memory>
#include <utility>
#include "JsonnetWorker.hpp"

namespace nodejsonnet {

  JsonnetWorker::JsonnetWorker(Napi::Env env, std::shared_ptr<JsonnetVm> vm, std::unique_ptr<Op> &&op):
    Napi::AsyncWorker(env), vm(vm), op(std::move(op)),
    deferred(Napi::Promise::Deferred::New(env)) {
  }

  void JsonnetWorker::Execute() {
    result = op->execute(vm);
  }

  void JsonnetWorker::OnOK() {
    deferred.Resolve(Napi::String::New(Env(), result.get()));
  }

  void JsonnetWorker::OnError(Napi::Error const &error) {
    deferred.Reject(error.Value());
  }

  JsonnetVm::Buffer JsonnetWorker::EvaluateFileOp::execute(std::shared_ptr<JsonnetVm> vm) {
    return vm->evaluateFile(filename);
  }

  JsonnetVm::Buffer JsonnetWorker::EvaluateSnippetOp::execute(std::shared_ptr<JsonnetVm> vm) {
    return vm->evaluateSnippet(filename, snippet);
  }

}
