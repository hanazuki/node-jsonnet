// SPDX-License-Identifier: MIT
#pragma once

#include <memory>
#include <string>

#include "Callback.hpp"

namespace nodejsonnet {

  namespace detail {

    struct ImportCallbackPayload : CallbackPayload<JsonnetVm::ImportResult> {
      static constexpr char resourceName[] = "Jsonnet Import Callback";

      ImportCallbackPayload(std::shared_ptr<JsonnetVm> vm, std::string base, std::string rel);

      std::vector<napi_value> makeArgs(Napi::Env env) const;
      void resolveResult(Napi::Value val);

    private:
      std::string base;
      std::string rel;
    };

  }

  class JsonnetImportCallback : public Callback<detail::ImportCallbackPayload> {
  public:
    using Callback::Callback;
  };

}
