#include <memory>
#include <napi.h>
extern "C" {
#include <libjsonnet.h>
}

class Jsonnet: public Napi::ObjectWrap<Jsonnet> {
public:
  static Napi::Object init(Napi::Env env);

  explicit Jsonnet(const Napi::CallbackInfo &info);

private:
  static Napi::FunctionReference constructor;
  static Napi::Value getVersion(const Napi::CallbackInfo &info);

  std::shared_ptr<JsonnetVm> vm;
  Napi::Value evaluateFile(const Napi::CallbackInfo &info);
  Napi::Value evaluateSnippet(const Napi::CallbackInfo &info);

  std::shared_ptr<char> vmAcquire(char *buf) {
    return {buf, [vm](char *buf){ jsonnet_realloc(vm.get(), buf, 0); }};
  }
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
  Napi::ObjectWrap<Jsonnet>(info), vm(jsonnet_make(), jsonnet_destroy) {
}

Napi::Value Jsonnet::getVersion(const Napi::CallbackInfo& info) {
  auto env = info.Env();
  return Napi::String::New(env, ::jsonnet_version());
}

Napi::Value Jsonnet::evaluateFile(const Napi::CallbackInfo& info) {
  auto env = info.Env();

  std::string const filename = info[0].As<Napi::String>();
  int error;
  auto result = jsonnet_evaluate_file(vm.get(), filename.c_str(), &error);
  if(error != 0) {
    
  }

  return Napi::String::New(env, result);
}

static Napi::Object init_node_jsonnet(Napi::Env env, Napi::Object _exports) {
  return Jsonnet::init(env);
}

NODE_API_MODULE(jsonnet, init_node_jsonnet)
