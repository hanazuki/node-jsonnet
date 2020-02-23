type Jsonnet = {
  version: string;

  new(): {
    extString(key: string, value: string): void;
    extCode(key: string, value: string): void;
    tlaString(key: string, value: string): void;
    tlaCode(key: string, value: string): void;

    evaluateFile(filename: string): string;
    evaluateSnippet(snippet: string): string;
    evaluateSnippet(snippet: string, filename: string): string;
  };
}

const Jsonnet: Jsonnet = require('bindings')('node-jsonnet');

console.log(Jsonnet.version);

const jsonnet = new Jsonnet();
jsonnet.tlaCode("a", "{a:1+2}");
console.log(jsonnet.evaluateSnippet("function(a) {a: a}"));
