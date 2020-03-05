const assert = require('assert').strict;
const Jsonnet = require("../lib/index.js");

{
  const jsonnet = new Jsonnet();

  jsonnet.evaluateSnippet(`[1,"a",true,null,{a:{b:3}}]`).then(
    j => assert.deepEqual(JSON.parse(j), [1,"a",true,null,{a:{b:3}}])
  );
}

{
  const jsonnet = new Jsonnet().addJpath(`${__dirname}/fixtures`);

  jsonnet.evaluateFile(`${__dirname}/fixtures/fruits.jsonnet`).then(
    j => assert.deepEqual(JSON.parse(j), [{name: "Kiwi"}, {name: "Orange"}])
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
  jsonnet.nativeCallback("double", (x) => x * 2, "x")
  jsonnet.nativeCallback("negate", (b) => !b, "b")
  jsonnet.nativeCallback("concat", (s, t) => s + t, "s", "t")
  jsonnet.nativeCallback("isNull", (v) => v === null, "v")
  jsonnet.nativeCallback("null", () => null)

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
}

{
  const jsonnet = new Jsonnet().tlaString("var1", "test").tlaCode("var2", "2");
  jsonnet.nativeCallback("func1", (var1, var2) => var1 + var2, "var1", "var2")

  jsonnet.evaluateSnippet(`std.native("func1")`).then(
    j => assert.equal(JSON.parse(j), "test2")
  );
}
