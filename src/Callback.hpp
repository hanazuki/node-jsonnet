// SPDX-License-Identifier: MIT
#pragma once

#include <exception>
#include <future>
#include <memory>
#include <stdexcept>
#include <napi.h>

#include "JsonnetVm.hpp"

namespace nodejsonnet {

  // Wrap Napi::Error in plain C++ error so it can safely pass throgh non-JS thread
  struct CallbackError: std::runtime_error {
    explicit CallbackError(std::string const &resourceName, Napi::Error e);
    std::shared_ptr<Napi::Error> jsError;
  };

  template <typename Result> struct CallbackPayload {
    explicit CallbackPayload(std::shared_ptr<JsonnetVm> vm): vm{std::move(vm)} {
    }

    void setResult(Result value) {
      result.set_value(std::move(value));
    }
    void setError(std::exception_ptr e) {
      result.set_exception(e);
    }
    std::future<Result> getFuture() {
      return result.get_future();
    }

  protected:
    std::shared_ptr<JsonnetVm> getVm() const {
      return vm;
    }

  private:
    std::shared_ptr<JsonnetVm> vm;
    std::promise<Result> result;
  };

  template <typename PayloadType> class Callback {
  public:
    Callback(Napi::Env env, Napi::Function fun)
      : tsfn{ThreadSafeFunction::New(env, fun, PayloadType::resourceName, 0, 1)} {
    }

    ~Callback() {
      tsfn.Release();
    }

    Callback(Callback const &) = delete;
    Callback &operator=(Callback const &) = delete;

    template <typename... Args> auto call(std::shared_ptr<JsonnetVm> vm, Args &&...args) {
      // This function runs in a worker thread and cannot access Node VM.
      PayloadType payload(std::move(vm), std::forward<Args>(args)...);
      auto const status = tsfn.BlockingCall(&payload);
      if(status != napi_ok) {
        throw std::runtime_error(
          std::string("ThreadSafeFunction::BlockingCall failed: napi_status=") +
          std::to_string(status));
      }
      return payload.getFuture().get();
    }

  private:
    static void callback(
      Napi::Env env, Napi::Function fun, std::nullptr_t *, PayloadType *payload) {
      // This function runs in the Node main thread.
      try {
        auto const result = fun.Call(payload->makeArgs(env));

        if(!result.IsPromise()) {
          payload->resolveResult(result);
          return;
        }

        auto const on_success = Napi::Function::New(
          env,
          [](Napi::CallbackInfo const &info) noexcept {
            auto const p = static_cast<PayloadType *>(info.Data());
            try {
              p->resolveResult(info[0]);
            } catch(Napi::Error const &e) {
              p->setError(std::make_exception_ptr(CallbackError{PayloadType::resourceName, e}));
            } catch(...) {
              p->setError(std::current_exception());
            }
          },
          "onSuccess", payload);

        auto const on_failure = Napi::Function::New(
          env,
          [](Napi::CallbackInfo const &info) noexcept {
            auto const p = static_cast<PayloadType *>(info.Data());
            try {
              p->setError(std::make_exception_ptr(
                CallbackError{PayloadType::resourceName, Napi::Error(info.Env(), info[0])}));
            } catch(...) {
              p->setError(std::current_exception());
            }
          },
          "onFailure", payload);

        result.template As<Napi::Promise>().Then(on_success, on_failure);
      } catch(Napi::Error const &e) {
        payload->setError(std::make_exception_ptr(CallbackError{PayloadType::resourceName, e}));
      } catch(...) {
        payload->setError(std::current_exception());
      }
    }

    using ThreadSafeFunction = Napi::TypedThreadSafeFunction<std::nullptr_t, PayloadType, callback>;
    ThreadSafeFunction tsfn;
  };

}
