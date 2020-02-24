import { Jsonnet } from '@hanazuki/node-jsonnet';

const version = Jsonnet.version;

const jsonnet = new Jsonnet();

const snippet = `
local a = 3;
local b = 7;
function(c) {answer: a * b * c}
`;

jsonnet.evaluateSnippet(snippet)
       .then(json => console.log(JSON.parse(json)))
       .catch(error => console.log(error));

jsonnet.evaluateSnippet(snippet, "snippet.jsonnet")
       .then(json => console.log(JSON.parse(json)))
       .catch(error => console.log(error));

jsonnet.evaluateFile("file.jsonnet")
       .then(json => console.log(JSON.parse(json)))
       .catch(error => console.log(error));
