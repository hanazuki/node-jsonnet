const assert = require('assert').strict;
const {Jsonnet} = require("../");

{
  const jsonnet = new Jsonnet();

  jsonnet.evaluateSnippet(`[1,"a",true,null,{a:{b:3}}]`).then(
    j => assert.deepEqual(JSON.parse(j), [1,"a",true,null,{a:{b:3}}])
  );
}

{
  const jsonnet = new Jsonnet().addJpath(`${__dirname}/fixtures`);
  jsonnet.extString("var1", "str");
  jsonnet.extCode("var2", `{a: [0]}`);

  jsonnet.evaluateSnippet(`std.extVar("var1")`).then(
    j => assert.equal(JSON.parse(j), "str")
  );
  jsonnet.evaluateSnippet(`std.extVar("var2")`).then(
    j => assert.deepEqual(JSON.parse(j), {a: [0]})
  );
}

{
  const jsonnet = new Jsonnet().addJpath(`${__dirname}/fixtures`);
  jsonnet.extString("var1", "str1");
  jsonnet.extCode("var1", `"str2"`);

  jsonnet.evaluateSnippet(`std.extVar("var1")`).then(
    j => assert.equal(JSON.parse(j), "str2")
  );
}

{
  const jsonnet = new Jsonnet().addJpath(`${__dirname}/fixtures`);

  jsonnet.evaluateFile(`${__dirname}/fixtures/fruits.jsonnet`).then(
    j => assert.deepEqual(JSON.parse(j), [{name: "Kiwi"}, {name: "Orange"}])
  );
}

{
  // jpath added later takes precedence
  const jsonnet = new Jsonnet();
  jsonnet.addJpath(`${__dirname}/fixtures/a`);
  jsonnet.addJpath(`${__dirname}/fixtures/b`);

  jsonnet.evaluateSnippet(`import "lib.jsonnet"`).then(
    j => assert.deepEqual(JSON.parse(j), "b")
  );
}

{
  const jsonnet = new Jsonnet().addJpath(`${__dirname}/fixtures`);

  jsonnet.evaluateFile(`${__dirname}/fixtures/utf8.jsonnet`).then(
    j => assert.deepEqual(JSON.parse(j), {"あ": "あいうえお", "🍔": "🐧"})
  );

  jsonnet.evaluateSnippet(`import "utf8.jsonnet"`).then(
    j => assert.deepEqual(JSON.parse(j), {"あ": "あいうえお", "🍔": "🐧"})
  );

  jsonnet.evaluateSnippet(`{"あ": "あいうえお", "🍔": "🐧"}`).then(
    j => assert.deepEqual(JSON.parse(j), {"あ": "あいうえお", "🍔": "🐧"})
  );

  jsonnet.evaluateFile(`${__dirname}/fixtures/🦔.jsonnet`).then(
    j => assert.deepEqual(JSON.parse(j), "🦔")
  );

  jsonnet.evaluateSnippet(`import "🦔.jsonnet"`).then(
    j => assert.deepEqual(JSON.parse(j), "🦔")
  );
}

{
  const jsonnet = new Jsonnet().tlaString("var1", "test").tlaCode("var2", "{x:1,y:2}");

  jsonnet.evaluateSnippet(`function(var1, var2) var1 + var2.y`).then(
    j => assert.equal(JSON.parse(j), "test2")
  );

  jsonnet.evaluateSnippet(`function(var2, var1) var1 + var2.y`).then(
    j => assert.equal(JSON.parse(j), "test2")
  );
}

{
  const jsonnet = new Jsonnet();
  jsonnet.nativeCallback("double", (x) => x * 2, "x");
  jsonnet.nativeCallback("negate", (b) => !b, "b");
  jsonnet.nativeCallback("concat", (s, t) => s + t, "s", "t");
  jsonnet.nativeCallback("isNull", (v) => v === null, "v");
  jsonnet.nativeCallback("null", () => null);
  jsonnet.nativeCallback("arrayOfObjects", () => [{name: "Kiwi"}, {name: "Orange"}]);

  jsonnet.evaluateSnippet(`std.native("double")(4)`).then(
    j => assert.equal(JSON.parse(j), 8)
  );

  jsonnet.evaluateSnippet(`std.native("negate")(true)`).then(
    j => assert.equal(JSON.parse(j), false)
  );

  jsonnet.evaluateSnippet(`std.native("concat")("a", "b")`).then(
    j => assert.equal(JSON.parse(j), "ab")
  );

  jsonnet.evaluateSnippet(`std.native("isNull")(null)`).then(
    j => assert.equal(JSON.parse(j), true)
  );

  jsonnet.evaluateSnippet(`std.native("null")()`).then(
    j => assert.equal(JSON.parse(j), null)
  );

  jsonnet.evaluateSnippet(`std.native("arrayOfObjects")()`).then(
    j => assert.deepEqual(JSON.parse(j), [{name: "Kiwi"}, {name: "Orange"}])
  );
}

{
  const jsonnet = new Jsonnet();

  jsonnet.nativeCallback("int8array", () => new Int8Array([1,2,3]));
  jsonnet.evaluateSnippet(`std.native("int8array")()`).then(
    j => assert.deepEqual(JSON.parse(j), {0: 1, 1: 2, 2: 3})
  );

  jsonnet.nativeCallback("function", () => function(){ return 1; });
  jsonnet.evaluateSnippet(`std.native("function")()`).then(
    j => assert.equal(JSON.parse(j), null)
  );

  jsonnet.nativeCallback("asyncFunction", () => async function(){ return 1; });
  jsonnet.evaluateSnippet(`std.native("asyncFunction")()`).then(
    j => assert.equal(JSON.parse(j), null)
  );

  jsonnet.nativeCallback("date", () => new Date(0));
  jsonnet.evaluateSnippet(`std.native("date")()`).then(
    j => assert.equal(JSON.parse(j), "1970-01-01T00:00:00.000Z")
  );

  jsonnet.nativeCallback("symbol", () => Symbol("foo"));
  jsonnet.evaluateSnippet(`std.native("symbol")()`).then(
    j => assert.equal(JSON.parse(j), null)
  );
}

{
  const jsonnet = new Jsonnet().tlaString("var1", "test").tlaCode("var2", "2");
  jsonnet.nativeCallback("func1", (var1, var2) => var1 + var2, "var1", "var2")

  jsonnet.evaluateSnippet(`std.native("func1")`).then(
    j => assert.equal(JSON.parse(j), "test2")
  );
}

{
  const jsonnet = new Jsonnet();
  jsonnet.nativeCallback("readFile", (name) => require("fs").promises.readFile(`${__dirname}/fixtures/${name}.jsonnet`, "utf8"), "name")

  jsonnet.evaluateSnippet(`std.native("readFile")("🦔")`).then(
    j => assert.equal(JSON.parse(j), `"🦔"\n`)
  );

  assert.rejects(
    jsonnet.evaluateSnippet(`std.native("readFile")("non-existent")`),
    {message: /^RUNTIME ERROR: .* ENOENT/}
  )
}

{
  const jsonnet = new Jsonnet();
  jsonnet.nativeCallback("func1", () => 1);

  jsonnet.evaluateSnippet(`std.native("func1")()`).then(
    j => assert.equal(JSON.parse(j), 1)
  )

  jsonnet.nativeCallback("func1", () => 2);

  jsonnet.evaluateSnippet(`std.native("func1")()`).then(
    j => assert.equal(JSON.parse(j), 2)
  )
}

{
  const jsonnet = new Jsonnet();

  assert.rejects(jsonnet.evaluateSnippet(`var1`), {message: /^STATIC ERROR: .* Unknown variable/});
  assert.rejects(jsonnet.evaluateSnippet(`1 / 0`), {message: /^RUNTIME ERROR: division by zero/});
}

{
  const jsonnet = new Jsonnet();

  // TODO: Exception propagation
  jsonnet.nativeCallback("fail", (msg) => { throw msg; }, "msg");
  // assert.rejects(jsonnet.evaluateSnippet(`std.native("fail")("kimagure")`), {message: /^RUNTIME ERROR: kimagure/});

  jsonnet.nativeCallback("failAsync", async (msg) => { throw msg; }, "msg");
  assert.rejects(jsonnet.evaluateSnippet(`std.native("failAsync")("kimagure")`), {message: /^RUNTIME ERROR: kimagure/});
}

{
  const jsonnet = new Jsonnet();

  assert.rejects(jsonnet.evaluateSnippet(`1 +`, "snippet-filename"), {message: /^STATIC ERROR: snippet-filename:1:4/});
}

{
  const jsonnet = new Jsonnet();

  jsonnet.evaluateSnippetMulti(`{"a.json": {a: 1}, "b.yaml": std.manifestYamlDoc({b: 2})}`).then(
    dict => {
      assert.equal(dict['a.json'], '{\n   "a": 1\n}\n');
      assert.equal(dict['b.yaml'], '"\\"b\\": 2"\n');
    },
  )

  jsonnet.evaluateFileMulti(`${__dirname}/fixtures/multi.jsonnet`).then(
    dict => {
      assert.equal(dict['a.json'], '{\n   "a": 1\n}\n');
      assert.equal(dict['b.yaml'], '"\\"b\\": 2"\n');
    },
  )
}

{
  const jsonnet = new Jsonnet();

  jsonnet.evaluateSnippetStream(`[{a: 1}, {b: 2}]`).then(
    list => {
      assert.equal(list[0], '{\n   "a": 1\n}\n');
      assert.equal(list[1], '{\n   "b": 2\n}\n');
    },
  )

  jsonnet.evaluateFileStream(`${__dirname}/fixtures/stream.jsonnet`).then(
    list => {
      assert.equal(list[0], '{\n   "a": 1\n}\n');
      assert.equal(list[1], '{\n   "b": 2\n}\n');
    },
  )
}
