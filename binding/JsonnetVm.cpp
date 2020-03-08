// SPDX-License-Identifier: MIT
#include <algorithm>
#include "JsonnetVm.hpp"

namespace nodejsonnet {

  std::shared_ptr<JsonnetVm> JsonnetVm::make() {
    return std::shared_ptr<JsonnetVm>(new JsonnetVm());
  }

  JsonnetVm::JsonnetVm(): vm{jsonnet_make()} {
  }

  JsonnetVm::~JsonnetVm() {
    jsonnet_destroy(vm);
  }

  void JsonnetVm::maxStack(unsigned v) {
    ::jsonnet_max_stack(vm, v);
  }

  void JsonnetVm::maxTrace(unsigned v) {
    ::jsonnet_max_trace(vm, v);
  }

  void JsonnetVm::gcMinObjects(unsigned v) {
    ::jsonnet_gc_min_objects(vm, v);
  }

  void JsonnetVm::gcGrowthTrigger(double v) {
    ::jsonnet_gc_growth_trigger(vm, v);
  }

  void JsonnetVm::extVar(std::string const &key, std::string const &val) {
    ::jsonnet_ext_var(vm, key.c_str(), val.c_str());
  }

  void JsonnetVm::extCode(std::string const &key, std::string const &val) {
    ::jsonnet_ext_code(vm, key.c_str(), val.c_str());
  }

  void JsonnetVm::tlaVar(std::string const &key, std::string const &val) {
    ::jsonnet_tla_var(vm, key.c_str(), val.c_str());
  }

  void JsonnetVm::tlaCode(std::string const &key, std::string const &val) {
    ::jsonnet_tla_code(vm, key.c_str(), val.c_str());
  }

  void JsonnetVm::jpathAdd(std::string const &path) {
    ::jsonnet_jpath_add(vm, path.c_str());
  }

  void JsonnetVm::nativeCallback(std::string const &name, NativeCallback &&cb, std::vector<std::string> const &params) {
    // Construct NULL-terminated array
    std::vector<char const *> params_cstr(params.size() + 1);
    std::transform(cbegin(params), cend(params), begin(params_cstr),
                   std::function<char const *(std::string const &)>(&std::string::c_str));

    auto ptr = &callbacks.emplace_front(this, params.size(), std::move(cb));
    ::jsonnet_native_callback(vm, name.c_str(), &trampoline, ptr, params_cstr.data());
  }

  JsonnetVm::Buffer JsonnetVm::evaluateFile(std::string const &filename) {
    int error;
    auto result = buffer(::jsonnet_evaluate_file(vm, filename.c_str(), &error));
    if(error != 0) {
      throw std::runtime_error(result.get());
    }
    return result;
  }

  JsonnetVm::Buffer JsonnetVm::evaluateSnippet(std::string const &filename, std::string const &snippet) {
    int error;
    auto result = buffer(::jsonnet_evaluate_snippet(vm, filename.c_str(), snippet.c_str(), &error));
    if(error != 0) {
      throw std::runtime_error(result.get());
    }
    return result;
  }

  JsonnetJsonValue *JsonnetVm::makeJsonString(std::string const &v) {
    return ::jsonnet_json_make_string(vm, v.c_str());
  }

  JsonnetJsonValue *JsonnetVm::makeJsonNumber(double v) {
    return ::jsonnet_json_make_number(vm, v);
  }

  JsonnetJsonValue *JsonnetVm::makeJsonBool(bool v) {
    return ::jsonnet_json_make_bool(vm, v);
  }

  JsonnetJsonValue *JsonnetVm::makeJsonNull() {
    return ::jsonnet_json_make_null(vm);
  }

  JsonnetJsonValue *JsonnetVm::makeJsonArray() {
    return ::jsonnet_json_make_array(vm);
  }

  void JsonnetVm::appendJsonArray(JsonnetJsonValue *array, JsonnetJsonValue *value) {
    ::jsonnet_json_array_append(vm, array, value);
  }

  JsonnetJsonValue *JsonnetVm::makeJsonObject() {
    return ::jsonnet_json_make_object(vm);
  }

  void JsonnetVm::appendJsonObject(JsonnetJsonValue *array, std::string const &field, JsonnetJsonValue *value) {
    ::jsonnet_json_object_append(vm, array, field.c_str(), value);
  }

  std::optional<std::string_view> JsonnetVm::extractJsonString(JsonnetJsonValue const *json) {
    if(auto const p = ::jsonnet_json_extract_string(vm, json)) {
      return p;
    }
    return std::nullopt;
  }

  std::optional<double> JsonnetVm::extractJsonNumber(JsonnetJsonValue const *json) {
    if(double n; ::jsonnet_json_extract_number(vm, json, &n)) {
      return n;
    }
    return std::nullopt;
  }

  std::optional<bool> JsonnetVm::extractJsonBool(JsonnetJsonValue const *json) {
    switch(::jsonnet_json_extract_bool(vm, json)) {
    case 0:
      return false;
    case 1:
      return true;
    default:
      return std::nullopt;
    }
  }

  bool JsonnetVm::extractJsonNull(JsonnetJsonValue const *json) {
    return ::jsonnet_json_extract_null(vm, json);
  }

  JsonnetVm::Buffer JsonnetVm::buffer(char *buf) {
    return {buf, [self = shared_from_this()](char *buf){ ::jsonnet_realloc(self->vm, buf, 0); }};
  }

  JsonnetJsonValue *JsonnetVm::trampoline(void *ctx, JsonnetJsonValue const *const *argv, int *success) {
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

}
