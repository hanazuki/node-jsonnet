// SPDX-License-Identifier: MIT
#include "Callback.hpp"

namespace nodejsonnet {

  CallbackError::CallbackError(std::string const &resourceName, Napi::Error e)
    : std::runtime_error{std::string("JavaScript exception throw in ") + resourceName + ": " +
                         e.Message()},
      jsError{std::make_shared<Napi::Error>(e)} {
  }

}
