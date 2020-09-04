// SPDX-License-Identifier: MIT
#include "JsonnetAddon.hpp"

namespace nodejsonnet {

  JsonnetAddon &JsonnetAddon::getInstance(Napi::Env env) {
    return *env.GetInstanceData<JsonnetAddon>();
  }

}
