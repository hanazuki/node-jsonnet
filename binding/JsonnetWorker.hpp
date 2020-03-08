// SPDX-License-Identifier: MIT
#pragma once

#include <napi.h>
#include <memory>
#include <string>
#include "JsonnetVm.hpp"

namespace nodejsonnet {

  class JsonnetWorker: public Napi::AsyncWorker {
  public:
    struct Op {
      virtual ~Op() = default;
      virtual JsonnetVm::Buffer execute(JsonnetVm const &vm) = 0;
    };

    struct EvaluateFileOp: public Op {
      explicit EvaluateFileOp(std::string filename);
      JsonnetVm::Buffer execute(JsonnetVm const &vm) override;

    private:
      std::string filename;
    };

    struct EvaluateSnippetOp: public Op {
      EvaluateSnippetOp(std::string snippet, std::string filename);
      JsonnetVm::Buffer execute(JsonnetVm const &vm) override;

    private:
      std::string snippet;
      std::string filename;
    };

    JsonnetWorker(Napi::Env env, std::shared_ptr<JsonnetVm> vm, std::unique_ptr<Op> op);

    void Execute() override;
    void OnOK() override;
    void OnError(Napi::Error const &error) override;

    Napi::Promise Promise() { return deferred.Promise(); }

  private:
    std::shared_ptr<JsonnetVm> vm;
    std::unique_ptr<Op> op;
    Napi::Promise::Deferred deferred;
    JsonnetVm::Buffer result;
  };

}
