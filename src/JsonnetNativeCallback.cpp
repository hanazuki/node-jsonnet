// SPDX-License-Identifier: MIT
#include "JsonnetNativeCallback.hpp"
#include "JsonValueConverter.hpp"

namespace nodejsonnet {

  namespace detail {

    NativeCallbackPayload::NativeCallbackPayload(
      std::shared_ptr<JsonnetVm> vm, std::vector<JsonnetJsonValue const *> args)
      : CallbackPayload{std::move(vm)}, args{std::move(args)} {
    }

    std::vector<napi_value> NativeCallbackPayload::makeArgs(Napi::Env env) const {
      JsonValueConverter const conv{getVm()};
      std::vector<napi_value> ret;
      ret.reserve(args.size());
      for(auto const arg: args) {
        ret.push_back(conv.toNapiValue(env, arg));
      }
      return ret;
    }

    void NativeCallbackPayload::resolveResult(Napi::Value val) {
      JsonValueConverter const conv{getVm()};
      setResult(conv.toJsonnetJson(val));
    }

  }

}
