#pragma once

#include <napi.h>
#include "JsonnetVm.hpp"

namespace nodejsonnet {

  class JsonnetWorker: public Napi::AsyncWorker {
  public:
    struct Op {
      virtual JsonnetVm::Buffer execute(JsonnetVm vm) = 0;
    };

    struct EvaluateFileOp: public Op {
      EvaluateFileOp(std::string const &filename): filename(filename) {
      }
      JsonnetVm::Buffer execute(JsonnetVm vm) override;

    private:
      std::string filename;
    };

    struct EvaluateSnippetOp: public Op {
      EvaluateSnippetOp(std::string const &snippet, std::string const &filename):
        snippet(snippet), filename(filename) {
      }
      JsonnetVm::Buffer execute(JsonnetVm vm) override;

    private:
      std::string snippet;
      std::string filename;
    };

    JsonnetWorker(Napi::Env env, JsonnetVm vm, std::unique_ptr<Op> &&op);

    void Execute() override;
    void OnOK() override;
    void OnError(Napi::Error const &error) override;

    Napi::Promise Promise() { return deferred.Promise(); }

  private:
    Napi::Promise::Deferred deferred;
    std::unique_ptr<Op> op;
    JsonnetVm vm;
    JsonnetVm::Buffer result;
  };

}