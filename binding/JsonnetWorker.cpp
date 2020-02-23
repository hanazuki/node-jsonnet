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

  std::shared_ptr<char> JsonnetWorker::EvaluateFileOp::execute(std::shared_ptr<JsonnetVm> vm) {
    int error;
    auto result = borrowBuffer(vm, jsonnet_evaluate_file(vm.get(), filename.c_str(), &error));
    if(error != 0) {
      throw std::runtime_error(std::string(result.get()));
    }
    return result;
  }

  std::shared_ptr<char> JsonnetWorker::EvaluateSnippetOp::execute(std::shared_ptr<JsonnetVm> vm) {
    int error;
    auto result = borrowBuffer(vm, jsonnet_evaluate_snippet(vm.get(), filename.c_str(), snippet.c_str(), &error));
    if(error != 0) {
      throw std::runtime_error(std::string(result.get()));
    }
    return result;
  }

}
