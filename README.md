# node-jsonnet

`@hanazuki/node-jsonnet` is a [libjsonnet](https://jsonnet.org) binding for [Node.js](https://nodejs.org) (native addon), which supports all the basic Jsonnet functionality including `import` and native callbacks (`std.native`). It also comes with TypeScript type definitions.

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

The complete library is documented in the TypeScript type definitions at [`types/index.d.ts`](types/index.d.ts).
