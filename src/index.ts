type Jsonnet = {
  version: string;

  /* new(): {
   *   extVar(key: string, value: string): void;
   *   extCode(key: string, value: string): void;
   *   tlaVar(key: string, value: string): void;
   *   tlaCode(key: string, value: string): void;

   *   evaluateFile(filename: string): string;
   *   evaluateSnippet(snippet: string, filename: string | null): string;
   * }; */
}

const Jsonnet: Jsonnet = require('bindings')('node-jsonnet');

console.log(Jsonnet.version);
