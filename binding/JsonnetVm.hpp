// SPDX-License-Identifier: MIT
#pragma once

extern "C" {
#include <libjsonnet.h>
}
#include <forward_list>
#include <memory>
#include <string>
#include <vector>

namespace nodejsonnet {

  class JsonnetVm: public std::enable_shared_from_this<JsonnetVm> {
  public:
    using Buffer = std::unique_ptr<char, std::function<void(char *)>>;

    static std::shared_ptr<JsonnetVm> make() {
      return std::shared_ptr<JsonnetVm>(new JsonnetVm());
    }

  private:
    JsonnetVm(): vm{jsonnet_make()} {
    }

  public:
    JsonnetVm(const JsonnetVm&) = delete;
    JsonnetVm& operator=(const JsonnetVm&) = delete;

    ~JsonnetVm() {
      jsonnet_destroy(vm);
    }

    void extVar(std::string const &key, std::string const &val) {
      ::jsonnet_ext_var(vm, key.c_str(), val.c_str());
    }

    void extCode(std::string const &key, std::string const &val) {
      ::jsonnet_ext_code(vm, key.c_str(), val.c_str());
    }

    void tlaVar(std::string const &key, std::string const &val) {
      ::jsonnet_tla_var(vm, key.c_str(), val.c_str());
    }

    void tlaCode(std::string const &key, std::string const &val) {
      ::jsonnet_tla_code(vm, key.c_str(), val.c_str());
    }

    void jpathAdd(std::string const &path) {
      ::jsonnet_jpath_add(vm, path.c_str());
    }

    Buffer evaluateFile(std::string const &filename) {
      int error;
      auto result = buffer(::jsonnet_evaluate_file(vm, filename.c_str(), &error));
      if(error != 0) {
        throw std::runtime_error(std::string(result.get()));
      }
      return result;
    }

    Buffer evaluateSnippet(std::string const &filename, std::string const &snippet) {
      int error;
      auto result = buffer(::jsonnet_evaluate_snippet(vm, filename.c_str(), snippet.c_str(), &error));
      if(error != 0) {
        throw std::runtime_error(std::string(result.get()));
      }
      return result;
    }

  private:
    ::JsonnetVm *vm;

    Buffer buffer(char *buf) {
      auto self = shared_from_this();
      return {buf, [self](char *buf){ ::jsonnet_realloc(self->vm, buf, 0); }};
    }
  };

}
