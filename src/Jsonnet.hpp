// SPDX-License-Identifier: MIT
#pragma once

#include <napi.h>
#include "JsonnetVmParam.hpp"

namespace nodejsonnet {

  class Jsonnet: public Napi::ObjectWrap<Jsonnet>, private JsonnetVmParam {
  public:
    static Napi::Function init(const Napi::Env env);

    explicit Jsonnet(const Napi::CallbackInfo &info);

  private:
    static Napi::Value getVersion(const Napi::CallbackInfo &info);

    Napi::Value setMaxStack(const Napi::CallbackInfo &info);
    Napi::Value setMaxTrace(const Napi::CallbackInfo &info);
    Napi::Value setGcMinObjects(const Napi::CallbackInfo &info);
    Napi::Value setGcGrowthTrigger(const Napi::CallbackInfo &info);
    Napi::Value setStringOutput(const Napi::CallbackInfo &info);
    Napi::Value setTrailingNewline(const Napi::CallbackInfo &info);
    Napi::Value evaluateFile(const Napi::CallbackInfo &info);
    Napi::Value evaluateSnippet(const Napi::CallbackInfo &info);
    Napi::Value evaluateFileMulti(const Napi::CallbackInfo &info);
    Napi::Value evaluateSnippetMulti(const Napi::CallbackInfo &info);
    Napi::Value evaluateFileStream(const Napi::CallbackInfo &info);
    Napi::Value evaluateSnippetStream(const Napi::CallbackInfo &info);
    Napi::Value extString(const Napi::CallbackInfo &info);
    Napi::Value extCode(const Napi::CallbackInfo &info);
    Napi::Value tlaString(const Napi::CallbackInfo &info);
    Napi::Value tlaCode(const Napi::CallbackInfo &info);
    Napi::Value addJpath(const Napi::CallbackInfo &info);
    Napi::Value nativeCallback(const Napi::CallbackInfo &info);
    Napi::Value importCallback(const Napi::CallbackInfo &info);
  };

}
