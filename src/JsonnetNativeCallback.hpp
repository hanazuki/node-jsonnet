// SPDX-License-Identifier: MIT
#pragma once

extern "C" {
#include <libjsonnet.h>
}
#include <memory>
#include <vector>

#include "Callback.hpp"

namespace nodejsonnet {

  namespace detail {

    struct NativeCallbackPayload: CallbackPayload<JsonnetJsonValue *> {
      static constexpr char resourceName[] = "Jsonnet Native Callback";

      NativeCallbackPayload(
        std::shared_ptr<JsonnetVm> vm, std::vector<JsonnetJsonValue const *> args);

      std::vector<napi_value> makeArgs(Napi::Env env) const;
      void resolveResult(Napi::Value val);

    private:
      std::vector<JsonnetJsonValue const *> args;
    };

  }

  class JsonnetNativeCallback: public Callback<detail::NativeCallbackPayload> {
  public:
    using Callback::Callback;
  };

}
