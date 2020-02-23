#include <napi.h>
extern "C" {
#include <libjsonnet.h>
}

namespace nodejsonnet {

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
    Napi::Value addJpath(const Napi::CallbackInfo &info);

    std::shared_ptr<char> borrowBuffer(char *buf) {
      return {buf, [vm = vm](char *buf){ jsonnet_realloc(vm.get(), buf, 0); }};
    }
  };

}
