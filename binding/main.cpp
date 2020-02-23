#include <memory>
#include <napi.h>
extern "C" {
#include <libjsonnet.h>
}

class Jsonnet: public Napi::ObjectWrap<Jsonnet> {
public:
  static Napi::Object init(const Napi::Env env);

  explicit Jsonnet(const Napi::CallbackInfo &info);

private:
  static Napi::FunctionReference constructor;
  static Napi::Value getVersion(const Napi::CallbackInfo &info);

  std::shared_ptr<JsonnetVm> vm;
  Napi::Value evaluateFile(const Napi::CallbackInfo &info);
  Napi::Value evaluateSnippet(const Napi::CallbackInfo &info);
  Napi::Value extString(const Napi::CallbackInfo &info);
  Napi::Value extCode(const Napi::CallbackInfo &info);
  Napi::Value tlaString(const Napi::CallbackInfo &info);
  Napi::Value tlaCode(const Napi::CallbackInfo &info);

  std::shared_ptr<char> borrowBuffer(char *buf) {
    return {buf, [vm = vm](char *buf){ jsonnet_realloc(vm.get(), buf, 0); }};
  }
};

Napi::FunctionReference Jsonnet::constructor;

Napi::Object Jsonnet::init(Napi::Env env) {
  auto const func = DefineClass(env, "Jsonnet", {
      StaticAccessor("version", &Jsonnet::getVersion, nullptr),
      InstanceMethod("evaluateFile", &Jsonnet::evaluateSnippet),
      InstanceMethod("evaluateSnippet", &Jsonnet::evaluateSnippet),
      InstanceMethod("extString", &Jsonnet::extString),
      InstanceMethod("extCode", &Jsonnet::extCode),
      InstanceMethod("tlaString", &Jsonnet::tlaString),
      InstanceMethod("tlaCode", &Jsonnet::tlaCode),
    });

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();
  return func;
}

Jsonnet::Jsonnet(const Napi::CallbackInfo &info):
  Napi::ObjectWrap<Jsonnet>(info), vm{jsonnet_make(), jsonnet_destroy} {
}

Napi::Value Jsonnet::getVersion(const Napi::CallbackInfo& info) {
  auto const env = info.Env();
  return Napi::String::New(env, ::jsonnet_version());
}

Napi::Value Jsonnet::evaluateFile(const Napi::CallbackInfo& info) {
  auto const env = info.Env();
  std::string const filename = info[0].As<Napi::String>();

  int error;
  auto result = borrowBuffer(jsonnet_evaluate_file(vm.get(), filename.c_str(), &error));
  if(error != 0) {
    throw Napi::Error::New(env, result.get());
  }

  return Napi::String::New(env, result.get());
}

Napi::Value Jsonnet::evaluateSnippet(const Napi::CallbackInfo& info) {
  auto const env = info.Env();
  std::string const snippet = info[0].As<Napi::String>();
  std::string const filename = info.Length() < 2 ? "(snippet)" : info[1].As<Napi::String>().Utf8Value();

  int error;
  auto const result = borrowBuffer(jsonnet_evaluate_snippet(vm.get(), filename.c_str(), snippet.c_str(), &error));
  if(error != 0) {
    throw Napi::Error::New(env, result.get());
  }

  return Napi::String::New(env, result.get());
}

Napi::Value Jsonnet::extString(const Napi::CallbackInfo& info) {
  auto const env = info.Env();
  std::string const key = info[0].As<Napi::String>();
  std::string const val = info[1].As<Napi::String>();

  jsonnet_ext_var(vm.get(), key.c_str(), val.c_str());

  return env.Undefined();
}

Napi::Value Jsonnet::extCode(const Napi::CallbackInfo& info) {
  auto const env = info.Env();
  std::string const key = info[0].As<Napi::String>();
  std::string const val = info[1].As<Napi::String>();

  jsonnet_ext_code(vm.get(), key.c_str(), val.c_str());

  return env.Undefined();
}

Napi::Value Jsonnet::tlaString(const Napi::CallbackInfo& info) {
  auto const env = info.Env();
  std::string const key = info[0].As<Napi::String>();
  std::string const val = info[1].As<Napi::String>();

  jsonnet_tla_var(vm.get(), key.c_str(), val.c_str());

  return env.Undefined();
}

Napi::Value Jsonnet::tlaCode(const Napi::CallbackInfo& info) {
  auto const env = info.Env();
  std::string const key = info[0].As<Napi::String>();
  std::string const val = info[1].As<Napi::String>();

  jsonnet_tla_code(vm.get(), key.c_str(), val.c_str());

  return env.Undefined();
}

static Napi::Object init_node_jsonnet(Napi::Env env, Napi::Object _exports) {
  return Jsonnet::init(env);
}
NODE_API_MODULE(jsonnet, init_node_jsonnet)
