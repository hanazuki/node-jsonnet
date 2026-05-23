// SPDX-License-Identifier: MIT
#pragma once

#include <exception>
#include <future>
#include <memory>
#include <stdexcept>
#include <napi.h>

#include "JsonnetVm.hpp"

namespace nodejsonnet {

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
      tsfn.BlockingCall(&payload);
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
          [](Napi::CallbackInfo const &info) {
            auto const p = static_cast<PayloadType *>(info.Data());
            try {
              p->resolveResult(info[0]);
            } catch(Napi::Error const &e) {
              p->setError(std::make_exception_ptr(std::runtime_error(e.Message())));
            } catch(...) {
              p->setError(std::current_exception());
            }
          },
          "onSuccess", payload);

        auto const on_failure = Napi::Function::New(
          env,
          [](Napi::CallbackInfo const &info) {
            auto const p = static_cast<PayloadType *>(info.Data());
            try {
              auto const error = info[0].ToString();
              p->setError(std::make_exception_ptr(std::runtime_error(error)));
            } catch(Napi::Error const &e) {
              p->setError(std::make_exception_ptr(std::runtime_error(e.Message())));
            } catch(...) {
              p->setError(std::current_exception());
            }
          },
          "onFailure", payload);

        result.template As<Napi::Promise>().Then(on_success, on_failure);
      } catch(Napi::Error const &e) {
        payload->setError(std::make_exception_ptr(std::runtime_error(e.Message())));
      } catch(...) {
        payload->setError(std::current_exception());
      }
    }

    using ThreadSafeFunction = Napi::TypedThreadSafeFunction<std::nullptr_t, PayloadType, callback>;
    ThreadSafeFunction tsfn;
  };

}
