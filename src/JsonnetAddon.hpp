// SPDX-License-Identifier: MIT
#include "Jsonnet.hpp"

namespace nodejsonnet {

  struct JsonnetAddon {
    Jsonnet::InstanceData jsonnet;

    static JsonnetAddon &getInstance(Napi::Env env);
  };

}
