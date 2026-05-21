// SPDX-License-Identifier: MIT
#include "JsonnetImportCallback.hpp"
#include <cstring>
#include <stdexcept>

namespace nodejsonnet {

  JsonnetImportCallback::JsonnetImportCallback(Napi::Env env, Napi::Function fun)
    : tsfn{ThreadSafeFunction::New(env, fun, "Jsonnet Import Callback", 0, 1)} {
  }

  JsonnetImportCallback::~JsonnetImportCallback() {
    this->tsfn.Release();
  }

  JsonnetVm::ImportResult JsonnetImportCallback::call(
    std::shared_ptr<JsonnetVm> vm, std::string const &base, std::string const &rel) {
    // This function runs in a worker thread and cannot access Node VM.

    Payload payload(std::move(vm), base, rel);
    tsfn.BlockingCall(&payload);
    return payload.getFuture().get();
  }

  void JsonnetImportCallback::resolveResult(Payload *payload, Napi::Value val) {
    auto const obj = val.As<Napi::Object>();
    auto const vm = payload->getVm();

    auto const foundHereStr = obj.Get("foundHere").As<Napi::String>().Utf8Value();
    auto foundHereBuf = vm->allocBuffer(foundHereStr.size() + 1);
    std::memcpy(foundHereBuf.get(), foundHereStr.c_str(), foundHereStr.size() + 1);

    auto const contentVal = obj.Get("content");
    JsonnetVm::Buffer contentBuf;
    size_t contentLen;

    if(contentVal.IsString()) {
      auto const str = contentVal.As<Napi::String>().Utf8Value();
      contentBuf = vm->allocBuffer(str.size());
      std::memcpy(contentBuf.get(), str.data(), str.size());
      contentLen = str.size();
    } else {
      auto const ta = contentVal.As<Napi::TypedArray>();
      contentBuf = vm->allocBuffer(ta.ByteLength());
      std::memcpy(contentBuf.get(), static_cast<char *>(ta.ArrayBuffer().Data()) + ta.ByteOffset(),
        ta.ByteLength());
      contentLen = ta.ByteLength();
    }

    payload->setResult(JsonnetVm::ImportResult{
      std::move(foundHereBuf),
      std::move(contentBuf),
      contentLen,
    });
  }

  void JsonnetImportCallback::callback(
    Napi::Env env, Napi::Function fun, std::nullptr_t *, Payload *payload) {
    // This function runs in the Node main thread.

    auto const base = Napi::String::New(env, payload->getBase());
    auto const rel = Napi::String::New(env, payload->getRel());

    auto const result = fun.Call({base, rel});

    if(!result.IsPromise()) {
      resolveResult(payload, result);
      return;
    }

    auto const on_success = Napi::Function::New(
      env,
      [](Napi::CallbackInfo const &info) {
        resolveResult(static_cast<Payload *>(info.Data()), info[0]);
      },
      "onSuccess", payload);

    auto const on_failure = Napi::Function::New(
      env,
      [](Napi::CallbackInfo const &info) {
        auto const payload = static_cast<Payload *>(info.Data());
        auto const error = info[0].ToString();
        payload->setError(std::make_exception_ptr(std::runtime_error(error)));
      },
      "onFailure", payload);

    result.As<Napi::Promise>().Then(on_success, on_failure);
  }

}
