import { expect } from "tstyche";

import { Jsonnet } from '@hanazuki/node-jsonnet';

const version = Jsonnet.version;
expect(version)
  .type.toBe<string>();

const jsonnet = new Jsonnet();
expect(jsonnet)
  .type.toBe<Jsonnet>();

expect(jsonnet.nativeCallback("add", (a, b) => Number(a) + Number(b), "a", "b"))
  .type.toBe<Jsonnet>();

// @ts-expect-error!
expect(jsonnet.nativeCallback("typeMismatch", (a: Number, b: Number) => a + b, "a", "b"))
  .type.toBe<Jsonnet>();

// @ts-expect-error!
jsonnet.nativeCallback("arityMismatch", (a) => a);

// @ts-expect-error!
jsonnet.nativeCallback("arityMismatch", (a) => a, "a", "b");

const snippet = `
local a = 3;
local b = 7;
function(c) {answer: a * b * c}
`;

expect(jsonnet.evaluateSnippet(snippet))
  .type.toBe<Promise<string>>();

expect(jsonnet.evaluateSnippet(snippet, "snippet.jsonnet"))
  .type.toBe<Promise<string>>();

expect(jsonnet.evaluateFile("file.jsonnet"))
  .type.toBe<Promise<string>>();

const multi_snippet = `
{
  "a.json": {a: 1},
  "b.yaml": std.manifestYamlDoc(
    {b: 1}
  )
}
`;

expect(jsonnet.evaluateSnippetMulti(multi_snippet))
  .type.toBe<Promise<{ [name: string]: string; }>>();

const stream_snippet = `
{
  {a: 1},
  {b: 1},
}
`;

expect(jsonnet.evaluateSnippetStream(multi_snippet))
  .type.toBe<Promise<string[]>>();

expect(jsonnet.tlaString("tla", "0"))
  .type.toBe<Jsonnet>();

// @ts-expect-error!
jsonnet.tlaString("tla", 0);

expect(jsonnet.tlaCode("tla", "0"))
  .type.toBe<Jsonnet>();

// @ts-expect-error!
jsonnet.tlaCode("tla", 0);

expect(jsonnet.extString("ext", "0"))
  .type.toBe<Jsonnet>();

// @ts-expect-error!
jsonnet.extString("ext", 0);

expect(jsonnet.extCode("ext", "0"))
  .type.toBe<Jsonnet>();

// @ts-expect-error!
jsonnet.extCode("ext", 0);
