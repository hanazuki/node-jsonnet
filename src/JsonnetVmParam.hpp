// SPDX-License-Identifier: MIT
#pragma once

#include <map>
#include <optional>
#include <string>
#include <vector>
#include <napi.h>

namespace nodejsonnet {

  struct JsonnetVmParam {
    struct Variable {
      bool isCode;
      std::string value;
    };

    struct NativeCallbackParam {
      Napi::FunctionReference fun;
      std::vector<std::string> params;
    };

    struct ImportCallbackParam {
      Napi::FunctionReference fun;
    };

    std::optional<unsigned> maxStack, maxTrace;
    std::optional<unsigned> gcMinObjects;
    std::optional<double> gcGrowthTrigger;
    bool stringOutput = false;
    bool trailingNewline = true;

    std::map<std::string, Variable> ext, tla;
    std::vector<std::string> jpath;
    std::map<std::string, NativeCallbackParam> nativeCallbacks;
    std::optional<ImportCallbackParam> importCallbackParam;
  };

}
