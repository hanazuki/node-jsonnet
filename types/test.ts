import { Jsonnet } from '@hanazuki/node-jsonnet';

const version = Jsonnet.version;

const jsonnet = new Jsonnet();

const snippet = `
local a = 3;
local b = 7;
function(c) {answer: a * b * c}
`;

jsonnet.nativeCallback("add", (a, b) => Number(a) + Number(b), "a", "b");

jsonnet.evaluateSnippet(snippet)
       .then(json => console.log(JSON.parse(json)))
       .catch(error => console.log(error));

jsonnet.evaluateSnippet(snippet, "snippet.jsonnet")
       .then(json => console.log(JSON.parse(json)))
       .catch(error => console.log(error));

jsonnet.evaluateFile("file.jsonnet")
       .then(json => console.log(JSON.parse(json)))
       .catch(error => console.log(error));

const multi_snippet = `
{
  "a.json": {a: 1},
  "b.yaml": std.manifestYamlDoc(
    {b: 1}
  )
}
`;

jsonnet.evaluateSnippetMulti(multi_snippet)
       .then(jsons => console.log(JSON.parse(jsons["a.json"])))
       .catch(error => console.log(error));

const stream_snippet = `
{
  {a: 1},
  {b: 1},
}
`;

jsonnet.evaluateSnippetStream(multi_snippet)
       .then(jsons => console.log(JSON.parse(jsons[0])))
       .catch(error => console.log(error));

// @ts-expect-error
jsonnet.nativeCallback("arityMismatch", (a) => a);

// @ts-expect-error
jsonnet.nativeCallback("arityMismatch", (a) => a, "a", "b");
