#include "Jsonnet.hpp"

static Napi::Object init_node_jsonnet(Napi::Env env, Napi::Object _exports) {
  return nodejsonnet::Jsonnet::init(env);
}
NODE_API_MODULE(jsonnet, init_node_jsonnet)
