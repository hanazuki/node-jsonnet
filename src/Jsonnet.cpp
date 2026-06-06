// SPDX-License-Identifier: MIT
#include "Jsonnet.hpp"
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include "libjsonnet.h"

namespace nodejsonnet {

  Napi::Function Jsonnet::init(Napi::Env env) {
    return DefineClass(env, "Jsonnet",
      {
        StaticAccessor<&Jsonnet::getVersion>("version"),
        InstanceMethod<&Jsonnet::setMaxStack>("setMaxStack"),
        InstanceMethod<&Jsonnet::setMaxTrace>("setMaxTrace"),
        InstanceMethod<&Jsonnet::setGcMinObjects>("setGcMinObjects"),
        InstanceMethod<&Jsonnet::setGcGrowthTrigger>("setGcGrowthTrigger"),
        InstanceMethod<&Jsonnet::setStringOutput>("stringOutput"),
        InstanceMethod<&Jsonnet::setTrailingNewline>("trailingNewline"),
        InstanceMethod<&Jsonnet::evaluateFile>("evaluateFile"),
        InstanceMethod<&Jsonnet::evaluateSnippet>("evaluateSnippet"),
        InstanceMethod<&Jsonnet::evaluateFileMulti>("evaluateFileMulti"),
        InstanceMethod<&Jsonnet::evaluateSnippetMulti>("evaluateSnippetMulti"),
        InstanceMethod<&Jsonnet::evaluateFileStream>("evaluateFileStream"),
        InstanceMethod<&Jsonnet::evaluateSnippetStream>("evaluateSnippetStream"),
        InstanceMethod<&Jsonnet::extString>("_extString"),
        InstanceMethod<&Jsonnet::extCode>("_extCode"),
        InstanceMethod<&Jsonnet::tlaString>("_tlaString"),
        InstanceMethod<&Jsonnet::tlaCode>("_tlaCode"),
        InstanceMethod<&Jsonnet::addJpath>("addJpath"),
        InstanceMethod<&Jsonnet::nativeCallback>("_nativeCallback"),  // See also lib/index.js
        InstanceMethod<&Jsonnet::importCallback>("_importCallback"),  // See also lib/index.js
      });
  }

  Jsonnet::Jsonnet(const Napi::CallbackInfo &info): Napi::ObjectWrap<Jsonnet>(info) {
  }

  Napi::Value Jsonnet::getVersion(const Napi::CallbackInfo &info) {
    auto const env = info.Env();
    return Napi::String::New(env, ::jsonnet_version());
  }

  Napi::Value Jsonnet::setMaxStack(const Napi::CallbackInfo &info) {
    maxStack = info[0].As<Napi::Number>();
    return info.This();
  }

  Napi::Value Jsonnet::setMaxTrace(const Napi::CallbackInfo &info) {
    maxTrace = info[0].As<Napi::Number>();
    return info.This();
  }

  Napi::Value Jsonnet::setGcMinObjects(const Napi::CallbackInfo &info) {
    gcMinObjects = info[0].As<Napi::Number>();
    return info.This();
  }

  Napi::Value Jsonnet::setGcGrowthTrigger(const Napi::CallbackInfo &info) {
    gcGrowthTrigger = info[0].As<Napi::Number>();
    return info.This();
  }

  Napi::Value Jsonnet::setStringOutput(const Napi::CallbackInfo &info) {
    stringOutput = info[0].As<Napi::Boolean>();
    return info.This();
  }

  Napi::Value Jsonnet::setTrailingNewline(const Napi::CallbackInfo &info) {
    trailingNewline = info[0].As<Napi::Boolean>();
    return info.This();
  }

  Napi::Value Jsonnet::evaluate(Napi::Env const &env, std::unique_ptr<JsonnetWorker::Op> op) {
    auto const worker = new JsonnetWorker(env, *this, std::move(op));
    auto const promise = worker->Promise();
    worker->Queue();  // worker is deleted when it is done
    return promise;
  }

  Napi::Value Jsonnet::evaluateFile(const Napi::CallbackInfo &info) {
    auto filename = info[0].As<Napi::String>().Utf8Value();
    return evaluate(
      info.Env(), std::make_unique<JsonnetWorker::EvaluateFileOp>(std::move(filename)));
  }

  Napi::Value Jsonnet::evaluateSnippet(const Napi::CallbackInfo &info) {
    auto snippet = info[0].As<Napi::String>().Utf8Value();
    auto filename = info.Length() < 2 ? "(snippet)" : info[1].As<Napi::String>().Utf8Value();
    return evaluate(info.Env(),
      std::make_unique<JsonnetWorker::EvaluateSnippetOp>(std::move(snippet), std::move(filename)));
  }

  Napi::Value Jsonnet::evaluateFileMulti(const Napi::CallbackInfo &info) {
    auto filename = info[0].As<Napi::String>().Utf8Value();
    return evaluate(
      info.Env(), std::make_unique<JsonnetWorker::EvaluateFileMultiOp>(std::move(filename)));
  }

  Napi::Value Jsonnet::evaluateSnippetMulti(const Napi::CallbackInfo &info) {
    auto snippet = info[0].As<Napi::String>().Utf8Value();
    auto filename = info.Length() < 2 ? "(snippet)" : info[1].As<Napi::String>().Utf8Value();
    return evaluate(info.Env(), std::make_unique<JsonnetWorker::EvaluateSnippetMultiOp>(
                                  std::move(snippet), std::move(filename)));
  }

  Napi::Value Jsonnet::evaluateFileStream(const Napi::CallbackInfo &info) {
    auto filename = info[0].As<Napi::String>().Utf8Value();
    return evaluate(
      info.Env(), std::make_unique<JsonnetWorker::EvaluateFileStreamOp>(std::move(filename)));
  }

  Napi::Value Jsonnet::evaluateSnippetStream(const Napi::CallbackInfo &info) {
    auto snippet = info[0].As<Napi::String>().Utf8Value();
    auto filename = info.Length() < 2 ? "(snippet)" : info[1].As<Napi::String>().Utf8Value();
    return evaluate(info.Env(), std::make_unique<JsonnetWorker::EvaluateSnippetStreamOp>(
                                  std::move(snippet), std::move(filename)));
  }

  Napi::Value Jsonnet::extString(const Napi::CallbackInfo &info) {
    ext.insert_or_assign(info[0].As<Napi::String>().Utf8Value(),
      Variable{false, info[1].As<Napi::String>().Utf8Value()});

    return info.This();
  }

  Napi::Value Jsonnet::extCode(const Napi::CallbackInfo &info) {
    ext.insert_or_assign(info[0].As<Napi::String>().Utf8Value(),
      Variable{true, info[1].As<Napi::String>().Utf8Value()});

    return info.This();
  }

  Napi::Value Jsonnet::tlaString(const Napi::CallbackInfo &info) {
    tla.insert_or_assign(info[0].As<Napi::String>().Utf8Value(),
      Variable{false, info[1].As<Napi::String>().Utf8Value()});

    return info.This();
  }

  Napi::Value Jsonnet::tlaCode(const Napi::CallbackInfo &info) {
    tla.insert_or_assign(info[0].As<Napi::String>().Utf8Value(),
      Variable{true, info[1].As<Napi::String>().Utf8Value()});

    return info.This();
  }

  Napi::Value Jsonnet::addJpath(const Napi::CallbackInfo &info) {
    jpath.push_back(info[0].As<Napi::String>().Utf8Value());

    return info.This();
  }

  Napi::Value Jsonnet::nativeCallback(const Napi::CallbackInfo &info) {
    auto name = info[0].As<Napi::String>().Utf8Value();
    auto const fun = info[1].As<Napi::Function>();

    std::vector<std::string> params;
    for(size_t i = 2; i < info.Length(); ++i) {
      params.push_back(info[i].As<Napi::String>().Utf8Value());
    }

    nativeCallbacks.insert_or_assign(
      std::move(name), NativeCallbackParam{Napi::Persistent(fun), std::move(params)});

    return info.This();
  }

  Napi::Value Jsonnet::importCallback(const Napi::CallbackInfo &info) {
    auto const fun = info[0].As<Napi::Function>();
    importCallbackParam = ImportCallbackParam{Napi::Persistent(fun)};
    return info.This();
  }

}
