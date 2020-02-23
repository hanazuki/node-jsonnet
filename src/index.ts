/*! SPDX-License-Identifier: MIT */

interface Jsonnet {
  readonly version: string;

  new(): {
    extString(key: string, value: string): void;
    extCode(key: string, value: string): void;
    tlaString(key: string, value: string): void;
    tlaCode(key: string, value: string): void;

    addJpath(path: string): void;

    evaluateFile(filename: string): Promise<string>;
    evaluateSnippet(snippet: string): Promise<string>;
    evaluateSnippet(snippet: string, filename: string): Promise<string>;
  };
}

const Jsonnet: Jsonnet = require('bindings')('node-jsonnet');

console.log(Jsonnet.version);

const jsonnet = new Jsonnet();
jsonnet.tlaCode("a", "{a:1+2}");

jsonnet.evaluateSnippet("function(a) {a: a}").then(console.log).catch(console.log);
