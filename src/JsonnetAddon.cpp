// SPDX-License-Identifier: MIT
#include "JsonnetAddon.hpp"

namespace nodejsonnet {

  JsonnetAddon::JsonnetAddon(Napi::Env env, Napi::Object exports) {
    auto const jsonnet = nodejsonnet::Jsonnet::init(env);
    this->jsonnet = Napi::Persistent(jsonnet);

    DefineAddon(exports, {
                           InstanceValue("Jsonnet", jsonnet),
                         });
  }

  JsonnetAddon &JsonnetAddon::getInstance(Napi::Env env) {
    return *env.GetInstanceData<JsonnetAddon>();
  }

  Napi::Function JsonnetAddon::getJsonnet() {
    return jsonnet.Value().As<Napi::Function>();
  }
}
