// SPDX-License-Identifier: MIT
#pragma once

extern "C" {
#include <libjsonnet.h>
}
#include <forward_list>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
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

    JsonnetJsonValue *makeJsonString(std::string const &v) {
      return ::jsonnet_json_make_string(vm, v.c_str());
    }

    JsonnetJsonValue *makeJsonNumber(double v) {
      return ::jsonnet_json_make_number(vm, v);
    }

    JsonnetJsonValue *makeJsonBool(bool v) {
      return ::jsonnet_json_make_bool(vm, v);
    }

    JsonnetJsonValue *makeJsonNull() {
      return ::jsonnet_json_make_null(vm);
    }

    std::optional<std::string_view> extractJsonString(JsonnetJsonValue const *json) {
      if(auto const p = ::jsonnet_json_extract_string(vm, json); p) {
        return p;
      }
      return std::nullopt;
    }

    std::optional<double> extractJsonNumber(JsonnetJsonValue const *json) {
      double n;
      if(::jsonnet_json_extract_number(vm, json, &n)) {
        return n;
      }
      return std::nullopt;
    }

    std::optional<bool> extractJsonBool(JsonnetJsonValue const *json) {
      switch(::jsonnet_json_extract_bool(vm, json)) {
      case 0:
        return false;
      case 1:
        return true;
      default:
        return std::nullopt;
      }
    }

    bool extractJsonNull(JsonnetJsonValue const *json) {
      return ::jsonnet_json_extract_null(vm, json);
    }

  private:
    ::JsonnetVm *vm;

    Buffer buffer(char *buf) {
      auto self = shared_from_this();
      return {buf, [self](char *buf){ ::jsonnet_realloc(self->vm, buf, 0); }};
    }
  };

}
