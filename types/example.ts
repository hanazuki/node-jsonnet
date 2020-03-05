import { Jsonnet } from "@hanazuki/node-jsonnet";

const jsonnet = new Jsonnet();
jsonnet.nativeCallback("add", (a, b) => Number(a) + Number(b), "a", "b");
jsonnet.extCode("c", "4");

jsonnet.evaluateSnippet(`std.native("add")(1,2) * std.extVar("c")`)
       .then(json => JSON.parse(json)); // => 12
