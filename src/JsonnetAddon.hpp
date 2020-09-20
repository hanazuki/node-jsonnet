// SPDX-License-Identifier: MIT
#pragma once

#include "Jsonnet.hpp"

namespace nodejsonnet {

  class JsonnetAddon: public Napi::Addon<JsonnetAddon> {
  public:
    JsonnetAddon(Napi::Env env, Napi::Object exports);

    static JsonnetAddon &getInstance(Napi::Env env);

    Jsonnet::InstanceData jsonnet;
  };

}
