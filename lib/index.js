/*! SPDX-License-Identifier: MIT */
const binding = module.exports = require('bindings')('node-jsonnet')
const { Jsonnet } = binding;
const { isAsyncFunction } = require('node:util').types;

const proto = Jsonnet.prototype;

// Workaround: Callback's throwing in ThreadSafeFunction kills Node VM.
// Wrapping in async function can prevent it.
const _nativeCallback = proto._nativeCallback;
Object.defineProperty(proto, 'nativeCallback', {
  value: function (name, func, ...params) {
    const asyncFunc = isAsyncFunction(func)
      ? func
      : async function (...args) { return func.call(this, ...args); }

    return _nativeCallback.call(this, name, asyncFunc, ...params);
  },
});

const _importCallback = proto._importCallback;
Object.defineProperty(proto, 'importCallback', {
  value: function(func) {
    const asyncFunc = isAsyncFunction(func)
      ? func
      : async function(...args) { return func.call(this, ...args); };
    return _importCallback.call(this, asyncFunc);
  },
});

// Accept an object as the sole argument when defining ext/tla.
for (const func of ['extString', 'extCode', 'tlaString', 'tlaCode']) {
  const origFunc = proto[`_${func}`];
  Object.defineProperty(proto, func, {
    value: function (...params) {
      if (params.length == 1) {
        for (const [key, value] of Object.entries(params[0])) {
          origFunc.call(this, key, value);
        }
        return this;
      } else {
        return origFunc.call(this, ...params);
      }
    },
  });
}

class JsonnetError extends Error {
  constructor(...args) {
    super(...args);
    Error.captureStackTrace(this, this.constructor);
    this.name = this.constructor.name;
  }
}

Object.defineProperty(binding, 'JsonnetError', {
  value: JsonnetError,
  enumerable: true,
});
