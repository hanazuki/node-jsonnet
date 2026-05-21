// SPDX-License-Identifier: MIT
#pragma once

#include <future>
#include <memory>
#include <string>
#include <napi.h>
#include "JsonnetVm.hpp"

namespace nodejsonnet {

  class JsonnetImportCallback {
  public:
    JsonnetImportCallback(Napi::Env env, Napi::Function fun);
    ~JsonnetImportCallback();

    JsonnetImportCallback(JsonnetImportCallback const &) = delete;
    JsonnetImportCallback &operator=(JsonnetImportCallback const &) = delete;

    JsonnetVm::ImportResult call(
      std::shared_ptr<JsonnetVm> vm, std::string const &base, std::string const &rel);

  private:
    struct Payload {
      Payload(std::shared_ptr<JsonnetVm> vm, std::string base, std::string rel)
        : vm{std::move(vm)}, base{std::move(base)}, rel{std::move(rel)} {
      }

      std::shared_ptr<JsonnetVm> getVm() const {
        return vm;
      }
      std::string const &getBase() const {
        return base;
      }
      std::string const &getRel() const {
        return rel;
      }

      void setResult(JsonnetVm::ImportResult value) {
        result.set_value(std::move(value));
      }

      void setError(std::exception_ptr e) {
        result.set_exception(e);
      }

      std::future<JsonnetVm::ImportResult> getFuture() {
        return result.get_future();
      }

    private:
      std::shared_ptr<JsonnetVm> vm;
      std::string base;
      std::string rel;
      std::promise<JsonnetVm::ImportResult> result;
    };

    static void resolveResult(Payload *payload, Napi::Value val);
    static void callback(Napi::Env env, Napi::Function fun, std::nullptr_t *, Payload *payload);

    using ThreadSafeFunction = Napi::TypedThreadSafeFunction<std::nullptr_t, Payload, callback>;

    ThreadSafeFunction tsfn;
  };

}
