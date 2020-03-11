// SPDX-License-Identifier: MIT
#pragma once

#include <napi.h>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include "JsonnetVm.hpp"

namespace nodejsonnet {

  struct JsonnetVmParam {
    struct Variable {
      bool isCode;
      std::string value;
    };

    struct NativeCallback {
      std::shared_ptr<Napi::FunctionReference> fun;
      std::vector<std::string> params;
    };

    std::optional<unsigned> maxStack, maxTrace;
    std::optional<unsigned> gcMinObjects;
    std::optional<double> gcGrowthTrigger;

    std::map<std::string, Variable> ext, tla;
    std::vector<std::string> jpath;
    std::map<std::string, NativeCallback> nativeCallbacks;
  };

  class Jsonnet: public Napi::ObjectWrap<Jsonnet>, private JsonnetVmParam {
  public:
    static Napi::Object init(const Napi::Env env);

    explicit Jsonnet(const Napi::CallbackInfo &info);

  private:
    static Napi::FunctionReference constructor;
    static Napi::Value getVersion(const Napi::CallbackInfo &info);

    Napi::Value setMaxStack(const Napi::CallbackInfo &info);
    Napi::Value setMaxTrace(const Napi::CallbackInfo &info);
    Napi::Value setGcMinObjects(const Napi::CallbackInfo &info);
    Napi::Value setGcGrowthTrigger(const Napi::CallbackInfo &info);
    Napi::Value evaluateFile(const Napi::CallbackInfo &info);
    Napi::Value evaluateSnippet(const Napi::CallbackInfo &info);
    Napi::Value evaluateFileMulti(const Napi::CallbackInfo &info);
    Napi::Value evaluateSnippetMulti(const Napi::CallbackInfo &info);
    Napi::Value extString(const Napi::CallbackInfo &info);
    Napi::Value extCode(const Napi::CallbackInfo &info);
    Napi::Value tlaString(const Napi::CallbackInfo &info);
    Napi::Value tlaCode(const Napi::CallbackInfo &info);
    Napi::Value addJpath(const Napi::CallbackInfo &info);
    Napi::Value nativeCallback(const Napi::CallbackInfo &info);

    std::shared_ptr<JsonnetVm> createVm(Napi::Env const &env);
  };

}
