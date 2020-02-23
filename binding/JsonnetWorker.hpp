#pragma once

#include <napi.h>
extern "C" {
#include <libjsonnet.h>
}

#include "Jsonnet.hpp"

namespace nodejsonnet {

  class JsonnetWorker: public Napi::AsyncWorker {
  public:
    struct Op {
      virtual std::shared_ptr<char> execute(std::shared_ptr<JsonnetVm> vm) = 0;

    protected:
      std::shared_ptr<char> borrowBuffer(std::shared_ptr<JsonnetVm> vm, char *buf) {
        return {buf, [vm](char *buf){ jsonnet_realloc(vm.get(), buf, 0); }};
      }
    };

    struct EvaluateFileOp: public Op {
      EvaluateFileOp(std::string const &filename): filename(filename) {
      }
      std::shared_ptr<char> execute(std::shared_ptr<JsonnetVm> vm) override;

    private:
      std::string filename;
    };

    struct EvaluateSnippetOp: public Op {
      EvaluateSnippetOp(std::string const &snippet, std::string const &filename):
        snippet(snippet), filename(filename) {
      }
      std::shared_ptr<char> execute(std::shared_ptr<JsonnetVm> vm) override;

    private:
      std::string snippet;
      std::string filename;
    };

    JsonnetWorker(Napi::Env env, std::shared_ptr<JsonnetVm> vm, std::unique_ptr<Op> &&op);

    void Execute() override;
    void OnOK() override;
    void OnError(Napi::Error const &error) override;

    Napi::Promise Promise() { return deferred.Promise(); }

  private:
    Napi::Promise::Deferred deferred;
    std::unique_ptr<Op> op;
    std::shared_ptr<JsonnetVm> vm;
    std::shared_ptr<char> result;
  };

}
