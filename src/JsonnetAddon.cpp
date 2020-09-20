// SPDX-License-Identifier: MIT
#include "JsonnetAddon.hpp"

namespace nodejsonnet {

  JsonnetAddon::JsonnetAddon(Napi::Env env, Napi::Object exports) {
    DefineAddon(exports, {
                           InstanceValue("Jsonnet", nodejsonnet::Jsonnet::init(env, jsonnet)),
                         });
  }

  JsonnetAddon &JsonnetAddon::getInstance(Napi::Env env) {
    return *env.GetInstanceData<JsonnetAddon>();
  }

}
