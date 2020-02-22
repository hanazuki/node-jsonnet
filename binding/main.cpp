#include <napi.h>
extern "C" {
#include <libjsonnet.h>
}

class Jsonnet: public Napi::ObjectWrap<Jsonnet> {
public:
  static Napi::Object Init(Napi::Env env, Napi::Object exports);
  Jsonnet(const Napi::CallbackInfo &info);

private:
  static Napi::FunctionReference constructor;

  static Napi::Value GetVersion(const Napi::CallbackInfo &info);
};

Napi::FunctionReference Jsonnet::constructor;

Napi::Object Jsonnet::Init(Napi::Env env, Napi::Object exports) {
  auto func = DefineClass(env, "Jsonnet", {
      StaticAccessor("version", &Jsonnet::GetVersion, nullptr),
    });

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();
  return func;
}

Napi::Value Jsonnet::GetVersion(const Napi::CallbackInfo& info) {
  auto env = info.Env();
  return Napi::String::New(env, ::jsonnet_version());
}

static Napi::Object init_node_jsonnet(Napi::Env env, Napi::Object exports) {
  return Jsonnet::Init(env, exports);
}

NODE_API_MODULE(jsonnet, init_node_jsonnet)
