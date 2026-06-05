// SPDX-License-Identifier: MIT
#pragma once
#include <memory>
#include <vector>
#include <napi.h>
#include "JsonnetVm.hpp"

namespace nodejsonnet {

  struct JsonValueConverter {
    explicit JsonValueConverter(std::shared_ptr<JsonnetVm> vm);

    Napi::Value toNapiValue(Napi::Env const &env, JsonnetJsonValue const *json) const;
    JsonnetJsonValue *toJsonnetJson(Napi::Value v) const;

  private:
    class JsonnetJsonDeleter {
      JsonnetVm *vm;

    public:
      explicit JsonnetJsonDeleter(JsonnetVm *vm) noexcept;
      void operator()(JsonnetJsonValue *p) const noexcept;
    };

    std::shared_ptr<JsonnetVm> vm;
    JsonnetJsonValue *toJsonnetJsonImpl(Napi::Value v, std::vector<Napi::Object> &ancestors) const;
  };

}
