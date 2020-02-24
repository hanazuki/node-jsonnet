// SPDX-License-Identifier: MIT
#pragma once

extern "C" {
#include <libjsonnet.h>
}
#include <memory>
#include <string>

namespace nodejsonnet {

  class JsonnetVm {
  public:

    JsonnetVm(): vm{jsonnet_make(), jsonnet_destroy} {
    }

    void extVar(std::string const &key, std::string const &val) {
      ::jsonnet_ext_var(vm.get(), key.c_str(), val.c_str());
    }

    void extCode(std::string const &key, std::string const &val) {
      ::jsonnet_ext_code(vm.get(), key.c_str(), val.c_str());
    }

    void tlaVar(std::string const &key, std::string const &val) {
      ::jsonnet_tla_var(vm.get(), key.c_str(), val.c_str());
    }

    void tlaCode(std::string const &key, std::string const &val) {
      ::jsonnet_tla_code(vm.get(), key.c_str(), val.c_str());
    }

    void jpathAdd(std::string const &path) {
      ::jsonnet_jpath_add(vm.get(), path.c_str());
    }

    using Buffer = std::unique_ptr<char, std::function<void(char *)>>;

    Buffer evaluateFile(std::string const &filename) {
      int error;
      auto result = buffer(::jsonnet_evaluate_file(vm.get(), filename.c_str(), &error));
      if(error != 0) {
        throw std::runtime_error(std::string(result.get()));
      }
      return result;
    }

    Buffer evaluateSnippet(std::string const &filename, std::string const &snippet) {
      int error;
      auto result = buffer(::jsonnet_evaluate_snippet(vm.get(), filename.c_str(), snippet.c_str(), &error));
      if(error != 0) {
        throw std::runtime_error(std::string(result.get()));
      }
      return result;
    }

  private:
    std::shared_ptr<::JsonnetVm> vm;

    Buffer buffer(char *buf) {
      return {buf, [vm = vm](char *buf){ ::jsonnet_realloc(vm.get(), buf, 0); }};
    }
  };

}
