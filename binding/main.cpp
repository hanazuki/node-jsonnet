#include <napi.h>
extern "C" {
#include <libjsonnet.h>
}

class Jsonnet: public Napi::ObjectWrap<Jsonnet> {
public:
  static Napi::Object init(Napi::Env env);

  explicit Jsonnet(const Napi::CallbackInfo &info);
  ~Jsonnet();

private:
  static Napi::FunctionReference constructor;
  static Napi::Value getVersion(const Napi::CallbackInfo &info);

  JsonnetVm *vm;
};

Napi::FunctionReference Jsonnet::constructor;

Napi::Object Jsonnet::init(Napi::Env env) {
  auto func = DefineClass(env, "Jsonnet", {
      StaticAccessor("version", &Jsonnet::getVersion, nullptr),
    });

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();
  return func;
}

Jsonnet::Jsonnet(const Napi::CallbackInfo &info):
  Napi::ObjectWrap<Jsonnet>(info), vm(jsonnet_make()) {
}

Jsonnet::~Jsonnet() {
  jsonnet_destroy(vm);
}

Napi::Value Jsonnet::getVersion(const Napi::CallbackInfo& info) {
  auto env = info.Env();
  return Napi::String::New(env, ::jsonnet_version());
}

static Napi::Object init_node_jsonnet(Napi::Env env, Napi::Object _exports) {
  return Jsonnet::init(env);
}

NODE_API_MODULE(jsonnet, init_node_jsonnet)
