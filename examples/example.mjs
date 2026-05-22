import { Jsonnet } from "@hanazuki/node-jsonnet";
import * as path from "node:path";

const jsonnet = new Jsonnet();

// Evaluates a simple Jsonnet program into a JSON value
const json1 = await jsonnet.evaluateSnippet(`{a: 1 + 2, b: self.a * 3}`);
console.log(JSON.parse(json1));  // => { a: 3, b: 9 }

// Jsonnet programs can use JavaScript values through external variables (std.extVar)
// and native callbacks (std.native).
const json2 = await jsonnet.extCode("x", "4")
  .nativeCallback("add", (a, b) => Number(a) + Number(b), "a", "b")
  .evaluateSnippet(`std.extVar("x") * std.native("add")(1, 2)`);
console.log(JSON.parse(json2));  // => 12

// importCallback lets you intercept import expressions with a custom resolver.
const files = {
  "lib/util.libsonnet": '{ greet(name): "Hello, " + name }',
};
const json3 = await new Jsonnet()
  .importCallback((base, rel) => {
    const key = path.join(base, rel);
    if (!(key in files)) throw new Error(`not found: ${rel}`);
    return { foundHere: key, content: files[key] };
  })
  .evaluateSnippet('local util = import "lib/util.libsonnet"; util.greet("world")', "main.jsonnet");
console.log(JSON.parse(json3));  // => "Hello, world"
