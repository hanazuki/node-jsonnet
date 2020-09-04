// SPDX-License-Identifier: MIT
#include "JsonnetAddon.hpp"

static Napi::Object init_node_jsonnet(Napi::Env env, Napi::Object) {
  auto instance = new nodejsonnet::JsonnetAddon();
  env.SetInstanceData(instance);

  return nodejsonnet::Jsonnet::init(env, instance->jsonnet);
}
NODE_API_MODULE(jsonnet, init_node_jsonnet)
