// SPDX-License-Identifier: MIT
#pragma once

extern "C" {
#include <libjsonnet.h>
}
#include <forward_list>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

namespace nodejsonnet {

  class JsonnetVm: public std::enable_shared_from_this<JsonnetVm> {
  public:
    using NativeCallback = std::function<JsonnetJsonValue *(std::shared_ptr<JsonnetVm> vm, std::vector<JsonnetJsonValue const *> &&args)>;
    using Buffer = std::unique_ptr<char, std::function<void(char *)>>;

  private:
    JsonnetVm();
    JsonnetVm(JsonnetVm const &) = delete;
    JsonnetVm &operator=(JsonnetVm const &) = delete;

  public:

    static std::shared_ptr<JsonnetVm> make();

    ~JsonnetVm();

    void maxStack(unsigned v);
    void maxTrace(unsigned v);
    void gcMinObjects(unsigned v);
    void gcGrowthTrigger(double v);

    void extVar(std::string const &key, std::string const &val);
    void extCode(std::string const &key, std::string const &val);
    void tlaVar(std::string const &key, std::string const &val);
    void tlaCode(std::string const &key, std::string const &val);
    void jpathAdd(std::string const &path);
    void nativeCallback(std::string const &name, NativeCallback &&cb, std::vector<std::string> const &params);

    Buffer evaluateFile(std::string const &filename);
    Buffer evaluateSnippet(std::string const &filename, std::string const &snippet);

    JsonnetJsonValue *makeJsonString(std::string const &v);
    JsonnetJsonValue *makeJsonNumber(double v);
    JsonnetJsonValue *makeJsonBool(bool v);
    JsonnetJsonValue *makeJsonNull();
    JsonnetJsonValue *makeJsonArray();
    void appendJsonArray(JsonnetJsonValue *array, JsonnetJsonValue *value);
    JsonnetJsonValue *makeJsonObject();
    void appendJsonObject(JsonnetJsonValue *array, std::string const &field, JsonnetJsonValue *value);
    std::optional<std::string_view> extractJsonString(JsonnetJsonValue const *json);
    std::optional<double> extractJsonNumber(JsonnetJsonValue const *json);
    std::optional<bool> extractJsonBool(JsonnetJsonValue const *json);
    bool extractJsonNull(JsonnetJsonValue const *json);

  private:
    using CallbackEntry = std::tuple<JsonnetVm *, size_t, NativeCallback>;

    ::JsonnetVm *vm;
    std::forward_list<CallbackEntry> callbacks;  // [(this, arity, fun)]

    Buffer buffer(char *buf);
    static JsonnetJsonValue *trampoline(void *ctx, JsonnetJsonValue const *const *argv, int *success);
  };

}
