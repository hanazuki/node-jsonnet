// SPDX-License-Identifier: MIT
#pragma once

extern "C" {
#include <libjsonnet.h>
}
#include <algorithm>
#include <forward_list>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

namespace nodejsonnet {

  class JsonnetVm: public std::enable_shared_from_this<JsonnetVm> {
  public:
    using NativeCallback = std::function<JsonnetJsonValue *(std::shared_ptr<JsonnetVm> vm, std::vector<JsonnetJsonValue const *> &&args)>;
    using Buffer = std::unique_ptr<char, std::function<void(char *)>>;

    static std::shared_ptr<JsonnetVm> make() {
      return std::shared_ptr<JsonnetVm>(new JsonnetVm());
    }

  private:
    JsonnetVm(): vm{jsonnet_make()} {
    }

  public:
    JsonnetVm(JsonnetVm const &) = delete;
    JsonnetVm &operator=(JsonnetVm const &) = delete;

    ~JsonnetVm() {
      jsonnet_destroy(vm);
    }

    void maxStack(unsigned v) {
      ::jsonnet_max_stack(vm, v);
    }

    void maxTrace(unsigned v) {
      ::jsonnet_max_trace(vm, v);
    }

    void gcMinObjects(unsigned v) {
      ::jsonnet_gc_min_objects(vm, v);
    }

    void gcGrowthTrigger(double v) {
      ::jsonnet_gc_growth_trigger(vm, v);
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

    void nativeCallback(std::string const &name, NativeCallback &&cb, std::vector<std::string> const &params) {
      // Construct NULL-terminated array
      std::vector<char const *> params_cstr(params.size() + 1);
      std::transform(cbegin(params), cend(params), begin(params_cstr),
                     std::function<char const *(std::string const &)>(&std::string::c_str));

      auto ptr = &callbacks.emplace_front(this, params.size(), std::move(cb));
      ::jsonnet_native_callback(vm, name.c_str(), &trampoline, ptr, params_cstr.data());
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
        throw std::runtime_error(result.get());
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

    JsonnetJsonValue *makeJsonArray() {
      return ::jsonnet_json_make_array(vm);
    }

    void appendJsonArray(JsonnetJsonValue *array, JsonnetJsonValue *value) {
      ::jsonnet_json_array_append(vm, array, value);
    }

    JsonnetJsonValue *makeJsonObject() {
      return ::jsonnet_json_make_object(vm);
    }

    void appendJsonObject(JsonnetJsonValue *array, std::string const &field, JsonnetJsonValue *value) {
      ::jsonnet_json_object_append(vm, array, field.c_str(), value);
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
    using CallbackEntry = std::tuple<JsonnetVm *, size_t, NativeCallback>;

    ::JsonnetVm *vm;
    std::forward_list<CallbackEntry> callbacks;  // [(this, arity, fun)]

    Buffer buffer(char *buf) {
      return {buf, [self = shared_from_this()](char *buf){ ::jsonnet_realloc(self->vm, buf, 0); }};
    }

    static JsonnetJsonValue *trampoline(void *ctx, JsonnetJsonValue const *const *argv, int *success) {
      auto [vm, arity, func] = *static_cast<CallbackEntry *>(ctx);

      try {
        auto result = func(vm->shared_from_this(), {argv, argv + arity});
        *success = 1;
        return result;
      } catch(std::exception const &e) {
        *success = 0;
        return vm->makeJsonString(e.what());
      }
    }
  };

}
