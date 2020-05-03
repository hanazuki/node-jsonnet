#include "Jsonnet.hpp"

struct InstanceData {
  nodejsonnet::Jsonnet::InstanceData jsonnet;
};

static Napi::Object init_node_jsonnet(Napi::Env env, Napi::Object) {
  auto instanceData = new InstanceData();
  env.SetInstanceData(instanceData);

  return nodejsonnet::Jsonnet::init(env, instanceData->jsonnet);
}
NODE_API_MODULE(jsonnet, init_node_jsonnet)
