// SPDX-License-Identifier: MIT
#pragma once

#include "Jsonnet.hpp"

namespace nodejsonnet {

  struct JsonnetAddon {
    Jsonnet::InstanceData jsonnet;

    static JsonnetAddon &getInstance(Napi::Env env);
  };

}
