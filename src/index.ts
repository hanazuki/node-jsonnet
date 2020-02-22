var addon = require('bindings')('node-jsonnet');

console.log(addon.hello()); // 'world'
