#!/usr/bin/env node

const assert = require('assert').strict;
const fs = require('fs').promises;
const path = require('path');

const Jsonnet = require('../').Jsonnet;

const run = async () => {
  let tested = 0;

  for await (const dirent of await fs.opendir('.')) {
    if(!dirent.isFile() || !/\.jsonnet$/.test(dirent.name)) continue;

    if(/^trace\./.test(dirent.name)) continue;  // Skip

    const expectsError = /^error\./.test(dirent.name);

    const jsonnet = new Jsonnet();
    if(/^tla\./.test(dirent.name)) {
      jsonnet.tlaString('var1', 'test').tlaCode('var2', `{x: 1, y: 2}`);
    } else {
      jsonnet.extString('var1', 'test').extCode('var2', `{x: 1, y: 2}`);
    }

    const golden = fs.readFile(`${dirent.name}.golden`, {encoding: 'utf8'}).catch(() => "true\n");

    try {
      const json = await jsonnet.evaluateFile(dirent.name);
      assert.ok(!expectsError, `${dirent.name} is expected to fail, but succeeded.`);
      assert.equal(json, await golden);
    } catch(error) {
      assert.ok(expectsError, `${dirent.name} is expected to succeed, but failed with ${error}.`);
      const message = error.message.replace(/\tstd\.jsonnet:[0-9]*:[0-9-]*/g, "\tstd.jsonnet:<stdlib_position_redacted>");
      assert.equal(message, await golden);
    }

    ++tested;
  }

  assert.ok(tested > 0, "Nothing tested.");
};

process.chdir(path.join(__dirname, '../jsonnet/test_suite'));
run();
