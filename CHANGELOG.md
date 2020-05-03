# ChangeLog

## v0.4.0 (unreleased)
- [breaking] NAPI_VERSION >= 6 is required
- Fixed to work in multi-context application (worker threads)

## v0.3.3
- Update libjsonnet to [v0.16.0](https://github.com/google/jsonnet/releases/tag/v0.16.0)

## v0.3.2
- Fixe bug where throwing in a native callback kills Node.js VM (such as `jsonnet.nativeCallback("fun", () => { throw "fail"; })`).

## v0.3.1
- Updated dependencies.

## v0.3.0
- Added `Jsonnet.prototype.evaluateFileMulti`, `Jsonnet.prototype.evaluateSnippetMulti`.
- Added `Jsonnet.prototype.evaluateFileStream`, `Jsonnet.prototype.evaluateSnippetStream`.
- Added `Jsonnet.prototype.stringOutput`.

## v0.2.0

## v0.1.1

## v0.1.0
