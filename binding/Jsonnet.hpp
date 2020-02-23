// SPDX-License-Identifier: MIT
#pragma once

#include <napi.h>
#include "JsonnetVm.hpp"

namespace nodejsonnet {

  class Jsonnet: public Napi::ObjectWrap<Jsonnet> {
  public:
    static Napi::Object init(const Napi::Env env);

    explicit Jsonnet(const Napi::CallbackInfo &info);

  private:
    static Napi::FunctionReference constructor;
    static Napi::Value getVersion(const Napi::CallbackInfo &info);

    JsonnetVm vm;
    Napi::Value evaluateFile(const Napi::CallbackInfo &info);
    Napi::Value evaluateSnippet(const Napi::CallbackInfo &info);
    Napi::Value extString(const Napi::CallbackInfo &info);
    Napi::Value extCode(const Napi::CallbackInfo &info);
    Napi::Value tlaString(const Napi::CallbackInfo &info);
    Napi::Value tlaCode(const Napi::CallbackInfo &info);
    Napi::Value addJpath(const Napi::CallbackInfo &info);
  };

}
