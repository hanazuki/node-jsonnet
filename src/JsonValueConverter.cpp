// SPDX-License-Identifier: MIT
#include "JsonValueConverter.hpp"
#include <algorithm>
#include <memory>
#include <optional>
#include <utility>

namespace nodejsonnet {

  JsonValueConverter::JsonnetJsonDeleter::JsonnetJsonDeleter(JsonnetVm *vm) noexcept: vm{vm} {
  }

  void JsonValueConverter::JsonnetJsonDeleter::operator()(JsonnetJsonValue *p) const noexcept {
    vm->destroyJson(p);
  }

  JsonValueConverter::JsonValueConverter(std::shared_ptr<JsonnetVm> vm): vm{std::move(vm)} {
  }

  Napi::Value JsonValueConverter::toNapiValue(
    Napi::Env const &env, JsonnetJsonValue const *json) const {
    if(vm->extractJsonNull(json)) {
      return env.Null();
    }
    if(auto const b = vm->extractJsonBool(json)) {
      return Napi::Boolean::New(env, *b);
    }
    if(auto const n = vm->extractJsonNumber(json)) {
      return Napi::Number::New(env, *n);
    }
    if(auto const s = vm->extractJsonString(json)) {
      return Napi::String::New(env, *s);
    }

    // NOTREACHED
    // Native extensions can only take primitives
    return env.Undefined();
  }

  JsonnetJsonValue *JsonValueConverter::toJsonnetJson(Napi::Value v) const {
    std::vector<Napi::Object> ancestors;
    auto result = toJsonnetJsonImpl(v, ancestors);
    return result ? *result : vm->makeJsonNull();
  }

  std::optional<JsonnetJsonValue *> JsonValueConverter::toJsonnetJsonImpl(
    Napi::Value v, std::vector<Napi::Object> &ancestors, bool callToJSON) const {
    if(v.IsBoolean()) {
      return vm->makeJsonBool(v.As<Napi::Boolean>());
    }
    if(v.IsNumber()) {
      return vm->makeJsonNumber(v.As<Napi::Number>());
    }
    if(v.IsString()) {
      return vm->makeJsonString(v.As<Napi::String>());
    }
    if(callToJSON && v.IsObject()) {
      auto const toJSON = v.As<Napi::Object>().Get("toJSON");
      if(toJSON.IsFunction()) {
        return toJsonnetJsonImpl(toJSON.As<Napi::Function>().Call(v, {}), ancestors, false);
      }
    }
    if(v.IsFunction()) {
      return std::nullopt;
    }
    if(v.IsArray()) {
      auto const array = v.As<Napi::Array>();
      if(std::any_of(ancestors.begin(), ancestors.end(),
           [&](auto const &a) { return a.StrictEquals(array); })) {
        throw Napi::TypeError::New(array.Env(), "Converting circular structure to JSON");
      }
      ancestors.push_back(array);
      auto json = std::unique_ptr<JsonnetJsonValue, JsonnetJsonDeleter>(
        vm->makeJsonArray(), JsonnetJsonDeleter{vm.get()});
      for(size_t i = 0, len = array.Length(); i < len; ++i) {
        // SerializeJSONArray: undefined result from SerializeJSONProperty -> "null"
        auto elem = toJsonnetJsonImpl(array[i], ancestors);
        vm->appendJsonArray(json.get(), elem ? *elem : vm->makeJsonNull());
      }
      ancestors.pop_back();
      return json.release();
    }
    if(v.IsObject()) {
      auto const object = v.As<Napi::Object>();
      if(std::any_of(ancestors.begin(), ancestors.end(),
           [&](auto const &a) { return a.StrictEquals(object); })) {
        throw Napi::TypeError::New(object.Env(), "Converting circular structure to JSON");
      }
      ancestors.push_back(object);
      auto json = std::unique_ptr<JsonnetJsonValue, JsonnetJsonDeleter>(
        vm->makeJsonObject(), JsonnetJsonDeleter{vm.get()});
      for(auto const [key, value]: object) {
        if(object.HasOwnProperty(key)) {
          // SerializeJSONObject: undefined result from SerializeJSONProperty -> omit
          if(auto member = toJsonnetJsonImpl(value, ancestors)) {
            vm->appendJsonObject(json.get(), key.ToString(), *member);
          }
        }
      }
      ancestors.pop_back();
      return json.release();
    }
    return std::nullopt;
  }

}
