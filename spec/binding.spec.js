import {describe, expect, it} from 'vitest';

import {Jsonnet, JsonnetError} from '../';

expect.extend({
  toBeJSON(actual, expected) {
    const result = {
      pass: this.equals(JSON.parse(actual), expected),
    }
    if(!result.pass) {
      result.message = () => `Expect '${actual}'${this.isNot ? ' not' : ''} to be JSON representing ${JSON.stringify(expected)}.`
    }
    return result;
  },
  withError(actual, expected_class, expected_message) {
    if(!actual instanceof expected_class) {
      return ({
        pass: false,
        message: () => `Expect to throw an error of class ${expected_class}.`,
      });
    }
    if(!this.equals(actual.message, expect.stringMatching(expected_message))) {
      return {
        pass: false,
        message: () => `Expect to throw an error with the message ${expected_message}.`,
      };
    }
    return {pass: true};
  },
});

describe('binding', () => {
  it('has version', async({expect}) => {
    expect(Jsonnet.version).toMatch(/^v/);
  });

  it('can evaluate complex JSON', async ({expect}) => {
    const jsonnet = new Jsonnet();

    await expect(jsonnet.evaluateSnippet(`[1,"a",true,null,{a:{b:3}}]`))
      .resolves.toBeJSON([1,"a",true,null,{a:{b:3}}]);
  });

  it('supports extVar', async ({expect}) => {
    const jsonnet = new Jsonnet();
    jsonnet.extString("var1", "str");
    jsonnet.extCode("var2", `{a: [0]}`);

    await expect(jsonnet.evaluateSnippet(`std.extVar("var1")`))
      .resolves.toBeJSON("str");

    await expect(jsonnet.evaluateSnippet(`std.extVar("var2")`))
      .resolves.toBeJSON({a: [0]});
  });

  it('uses the extVar added most recently for the same name', async ({expect}) => {
    const jsonnet = new Jsonnet();
    jsonnet.extString("var1", "str1");
    jsonnet.extCode("var1", `"str2"`);

    await expect(jsonnet.evaluateSnippet(`std.extVar("var1")`))
      .resolves.toBeJSON("str2");
  });

  it('can import files using Jpath', async ({expect}) => {
    const jsonnet = new Jsonnet().addJpath(`${__dirname}/fixtures`);

    await expect(jsonnet.evaluateFile(`${__dirname}/fixtures/fruits.jsonnet`))
      .resolves.toBeJSON([{name: "Kiwi"}, {name: "Orange"}]);
  });

  it('Jpath added later takes precedence', async ({expect}) => {
    const jsonnet = new Jsonnet();
    jsonnet.addJpath(`${__dirname}/fixtures/a`);
    jsonnet.addJpath(`${__dirname}/fixtures/b`);

    await expect(jsonnet.evaluateSnippet(`import "lib.jsonnet"`))
      .resolves.toBeJSON("b");
  });

  it('handles files in UTF-8', async ({expect}) => {
    const jsonnet = new Jsonnet().addJpath(`${__dirname}/fixtures`);

    await expect(jsonnet.evaluateFile(`${__dirname}/fixtures/utf8.jsonnet`))
      .resolves.toBeJSON({"あ": "あいうえお", "🍔": "🐧"});

    await expect(jsonnet.evaluateSnippet(`import "utf8.jsonnet"`))
      .resolves.toBeJSON({"あ": "あいうえお", "🍔": "🐧"});

    await expect(jsonnet.evaluateSnippet(`{"あ": "あいうえお", "🍔": "🐧"}`))
      .resolves.toBeJSON({"あ": "あいうえお", "🍔": "🐧"});
  });

  it('handles paths in UTF-8', async ({expect}) => {
    const jsonnet = new Jsonnet().addJpath(`${__dirname}/fixtures`);

    await expect(jsonnet.evaluateFile(`${__dirname}/fixtures/🦔.jsonnet`))
      .resolves.toBeJSON("🦔");

    await expect(jsonnet.evaluateSnippet(`import "🦔.jsonnet"`))
      .resolves.toBeJSON("🦔");
  });

  it('supports importstr', async ({expect}) => {
    const jsonnet = new Jsonnet().addJpath(`${__dirname}/fixtures`);

    await expect(jsonnet.evaluateSnippet(`importstr "🦔.jsonnet"`))
      .resolves.toBeJSON('"🦔"\n');
  });

  it('supports top-level arguments', async ({expect}) => {
    const jsonnet = new Jsonnet().tlaString("var1", "test").tlaCode("var2", "{x:1,y:2}");

    await expect(jsonnet.evaluateSnippet(`function(var1, var2) var1 + var2.y`))
      .resolves.toBeJSON("test2");

    await expect(jsonnet.evaluateSnippet(`function(var2, var1) var1 + var2.y`))
      .resolves.toBeJSON("test2");
  });

  it('support native callbacks', async ({expect}) => {
    const jsonnet = new Jsonnet();
    jsonnet.nativeCallback("double", (x) => x * 2, "x");
    jsonnet.nativeCallback("negate", (b) => !b, "b");
    jsonnet.nativeCallback("concat", (s, t) => s + t, "s", "t");
    jsonnet.nativeCallback("isNull", (v) => v === null, "v");
    jsonnet.nativeCallback("null", () => null);
    jsonnet.nativeCallback("arrayOfObjects", () => [{name: "Kiwi"}, {name: "Orange"}]);

    await expect(jsonnet.evaluateSnippet(`std.native("double")(4)`))
      .resolves.toBeJSON(8);

    await expect(jsonnet.evaluateSnippet(`std.native("negate")(true)`))
      .resolves.toBeJSON(false);

    await expect(jsonnet.evaluateSnippet(`std.native("negate")(false)`))
      .resolves.toBeJSON(true);

    await expect(jsonnet.evaluateSnippet(`std.native("concat")("a", "b")`))
      .resolves.toBeJSON("ab");

    await expect(jsonnet.evaluateSnippet(`std.native("isNull")(null)`))
      .resolves.toBeJSON(true);

    await expect(jsonnet.evaluateSnippet(`std.native("null")()`))
      .resolves.toBeJSON(null);

    await expect(jsonnet.evaluateSnippet(`std.native("arrayOfObjects")()`))
      .resolves.toBeJSON([{name: "Kiwi"}, {name: "Orange"}]);
  });

  it('serializes JavaScript objects', async ({expect}) => {
    const jsonnet = new Jsonnet();

    jsonnet.nativeCallback("int8array", () => new Int8Array([1,2,3]));
    await expect(jsonnet.evaluateSnippet(`std.native("int8array")()`)).resolves.toBeJSON({0: 1, 1: 2, 2: 3});

    jsonnet.nativeCallback("function", () => function(){ return 1; });
    await expect(jsonnet.evaluateSnippet(`std.native("function")()`)).resolves.toBeJSON(null);

    jsonnet.nativeCallback("asyncFunction", () => async function(){ return 1; });
    await expect(jsonnet.evaluateSnippet(`std.native("asyncFunction")()`)).resolves.toBeJSON(null);

    jsonnet.nativeCallback("date", () => new Date(0));
    await expect(jsonnet.evaluateSnippet(`std.native("date")()`)).resolves.toBeJSON("1970-01-01T00:00:00.000Z");

    jsonnet.nativeCallback("symbol", () => Symbol("foo"));
    await expect(jsonnet.evaluateSnippet(`std.native("symbol")()`)).resolves.toBeJSON(null);
  });

  it('supports top-level arguments for native callbacks', async ({expect}) => {
    const jsonnet = new Jsonnet().tlaString("var1", "test").tlaCode("var2", "2");
    jsonnet.nativeCallback("func1", (var1, var2) => var1 + var2, "var1", "var2")

    await expect(jsonnet.evaluateSnippet(`std.native("func1")`)).resolves.toBeJSON("test2");
  });

  it('supports native callback that returns a promise', async ({expect}) => {
    const jsonnet = new Jsonnet();
    jsonnet.nativeCallback("readFile", (name) => require("fs").promises.readFile(`${__dirname}/fixtures/${name}.jsonnet`, "utf8"), "name")

    await expect(jsonnet.evaluateSnippet(`std.native("readFile")("🦔")`)).resolves.toBeJSON(`"🦔"
`);

    await expect(jsonnet.evaluateSnippet(`std.native("readFile")("non-existent")`))
      .rejects.withError(JsonnetError, /^RUNTIME ERROR: .* ENOENT/);
  });

  it('uses the native callback added most recently for the same name', async ({expect}) => {
    const jsonnet = new Jsonnet();
    jsonnet.nativeCallback("func1", () => 1);

    await expect(jsonnet.evaluateSnippet(`std.native("func1")()`)).resolves.toBeJSON(1);

    jsonnet.nativeCallback("func1", () => 2);
    await expect(jsonnet.evaluateSnippet(`std.native("func1")()`)).resolves.toBeJSON(2);
  });

  it('reports Jsonnet errors', async ({expect}) => {
    const jsonnet = new Jsonnet();

    await expect(jsonnet.evaluateSnippet(`var1`))
      .rejects.withError(JsonnetError, /^STATIC ERROR: .* Unknown variable/);
    await expect(jsonnet.evaluateSnippet(`1 / 0`))
      .rejects.withError(JsonnetError, /^RUNTIME ERROR: division by zero/);
  });

  it('reports throwing native callback', async ({expect}) => {
    const jsonnet = new Jsonnet();

    jsonnet.nativeCallback("fail", (msg) => { throw msg; }, "msg");
    await expect(jsonnet.evaluateSnippet(`std.native("fail")("kimagure")`))
      .rejects.withError(JsonnetError, /^RUNTIME ERROR: kimagure/);
  });

  it('reports throwing async native callback', async ({expect}) => {
    const jsonnet = new Jsonnet();

    jsonnet.nativeCallback("failAsync", async (msg) => { throw msg; }, "msg");
    await expect(jsonnet.evaluateSnippet(`std.native("failAsync")("kimagure")`))
      .rejects.withError(JsonnetError, /^RUNTIME ERROR: kimagure/);
  });

  it('reports syntax error in snippet with filename', async ({expect}) => {
    const jsonnet = new Jsonnet();

    await expect(jsonnet.evaluateSnippet(`1 +`, "snippet-filename"))
      .rejects.withError(JsonnetError, /^STATIC ERROR: snippet-filename:1:4/);
  });

  it('evaluateSnippetMulti', async ({expect}) => {
    const jsonnet = new Jsonnet();

    await expect(jsonnet.evaluateSnippetMulti(`{"a.json": {a: 1}, "b.yaml": std.manifestYamlDoc({b: 2})}`))
      .resolves.toEqual({
        'a.json': '{\n   "a": 1\n}\n',
        'b.yaml': '"\\"b\\": 2"\n',
      });

    await expect(jsonnet.evaluateFileMulti(`${__dirname}/fixtures/multi.jsonnet`))
      .resolves.toEqual({
        'a.json': '{\n   "a": 1\n}\n',
        'b.yaml': '"\\"b\\": 2"\n',
      });
  });

  it('reports error for evaluateSnippetMulti', async ({expect}) => {
    const jsonnet = new Jsonnet();

    await expect(jsonnet.evaluateSnippetMulti(`1`))
      .rejects.withError(JsonnetError, /^RUNTIME ERROR:/)
  });

  it('reports error for evaluateFileMulti', async ({expect}) => {
    const jsonnet = new Jsonnet();

    await expect(jsonnet.evaluateFileMulti(`${__dirname}/fixtures/runtime_error.jsonnet`))
      .rejects.withError(JsonnetError, /^RUNTIME ERROR:/)
  });

  it('evaluateSnippetStream', async ({expect}) => {
    const jsonnet = new Jsonnet();

    await expect(jsonnet.evaluateSnippetStream(`[{a: 1}, {b: 2}]`))
      .resolves.toEqual([
        '{\n   "a": 1\n}\n',
        '{\n   "b": 2\n}\n',
      ]);

    await expect(jsonnet.evaluateFileStream(`${__dirname}/fixtures/stream.jsonnet`))
      .resolves.toEqual([
        '{\n   "a": 1\n}\n',
        '{\n   "b": 2\n}\n',
      ]);
  });

  it('reports error for evaluateSnippetStream', async ({expect}) => {
    const jsonnet = new Jsonnet();

    await expect(jsonnet.evaluateSnippetStream(`1`))
      .rejects.withError(JsonnetError, /^RUNTIME ERROR:/)
  });

  it('reports error for evaluateFileStream', async ({expect}) => {
    const jsonnet = new Jsonnet();

    await expect(jsonnet.evaluateFileStream(`${__dirname}/fixtures/runtime_error.jsonnet`))
      .rejects.withError(JsonnetError, /^RUNTIME ERROR:/)
  });

  it('supports stringOutput for snippet', async ({expect}) => {
    const jsonnet = new Jsonnet();

    jsonnet.stringOutput(true);
    await expect(jsonnet.evaluateSnippet(`"a"`))
      .resolves.toEqual("a\n");

    jsonnet.stringOutput(false);
    await expect(jsonnet.evaluateSnippet(`"a"`))
      .resolves.toEqual('"a"\n');
  });

  it('supports stringOutput for multi', async ({expect}) => {
    const jsonnet = new Jsonnet().stringOutput(true);

    await expect(jsonnet.evaluateSnippetMulti(`{"0":"a","1":"b"}`))
      .resolves.toEqual({'0': "a\n", '1': "b\n"});
  });

  it('supports stringOutput for stream', async ({expect}) => {
    const jsonnet = new Jsonnet().stringOutput(true);

    await expect(jsonnet.evaluateSnippetStream(`["a","b"]`))
      .resolves.toEqual(["a\n", "b\n"]);
  });
});
