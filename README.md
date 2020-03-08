# node-jsonnet

**@hanazuki/node-jsonnet** is a [libjsonnet](https://jsonnet.org) binding for [Node.js](https://nodejs.org) (native addon), which supports all the basic Jsonnet functionality including `import` and native callbacks (`std.native`). It also comes with TypeScript type definitions.

## Install Requirements

- Node.js v10 or later
- GCC or Clang C++ compiler that supports C++17
- CMake

## Usage Example

```typescript
import { Jsonnet } from "@hanazuki/node-jsonnet";

const jsonnet = new Jsonnet();
jsonnet.nativeCallback("add", (a, b) => Number(a) + Number(b), "a", "b");
jsonnet.extCode("c", "4");

jsonnet.evaluateSnippet(`std.native("add")(1,2) * std.extVar("c")`)
       .then(json => JSON.parse(json)); // => 12
```

The library is documented in the TypeScript type definitions at [`types/index.d.ts`](types/index.d.ts).

## License
@hanazuki/node-jsonnet is licensed under the MIT License (See [LICENSE](LICENSE) file for the full terms). libjsonnet, whose source code is included in the distributed NPM packages of @hanazuki/node-jsonnet, is developed by Google and licensed under the Apache License, Version 2.0 (See [jsonnet/LICENSE in this package](jsonnet/LICENSE) or [LICENSE in google/jsonnet](https://github.com/google/jsonnet/blob/master/LICENSE)).
