// SPDX-License-Identifier: MIT
#include "JsonnetImportCallback.hpp"
#include <cstring>

namespace nodejsonnet {

  namespace detail {

    ImportCallbackPayload::ImportCallbackPayload(
      std::shared_ptr<JsonnetVm> vm, std::string base, std::string rel)
      : CallbackPayload{std::move(vm)}, base{std::move(base)}, rel{std::move(rel)} {
    }

    std::vector<napi_value> ImportCallbackPayload::makeArgs(Napi::Env env) const {
      return {
        Napi::String::New(env, base),
        Napi::String::New(env, rel),
      };
    }

    void ImportCallbackPayload::resolveResult(Napi::Value val) {
      auto const obj = val.As<Napi::Object>();
      auto const vm = getVm();

      auto const foundHereStr = obj.Get("foundHere").As<Napi::String>().Utf8Value();
      auto foundHereBuf = vm->allocBuffer(foundHereStr.size() + 1);
      std::memcpy(foundHereBuf.get(), foundHereStr.c_str(), foundHereStr.size() + 1);

      auto const contentVal = obj.Get("content");
      auto [contentBuf, contentLen] = [&]() -> std::tuple<JsonnetVm::Buffer, size_t> {
        if(contentVal.IsString()) {
          auto const str = contentVal.As<Napi::String>().Utf8Value();
          auto buf = vm->allocBuffer(str.size());
          std::memcpy(buf.get(), str.data(), str.size());
          return {std::move(buf), str.size()};
        } else {
          auto const ta = contentVal.As<Napi::TypedArray>();
          auto buf = vm->allocBuffer(ta.ByteLength());
          std::memcpy(buf.get(), static_cast<char *>(ta.ArrayBuffer().Data()) + ta.ByteOffset(),
            ta.ByteLength());
          return {std::move(buf), ta.ByteLength()};
        }
      }();

      setResult(JsonnetVm::ImportResult{
        std::move(foundHereBuf),
        std::move(contentBuf),
        contentLen,
      });
    }

  }

}
