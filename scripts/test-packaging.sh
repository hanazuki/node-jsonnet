#!/bin/bash
set -eux

__dir="$(cd "$(dirname "$BASH_SOURCE")" && pwd)"

mkdir -p coverage
TRACEFILE="$PWD/coverage/lcov.info"

npm build
rm -f *.tgz
npm pack
tmpdir=$(mktemp -d)
tar xf hanazuki-node-jsonnet-*.tgz -C "$tmpdir"
pushd "$tmpdir/package"
npm install-test
if [[ ${NODE_JSONNET_ENABLE_COVERAGE-} ]]; then
  mkdir -p coverage
  lcov_opts=()
  if [[ $NODE_JSONNET_ENABLE_COVERAGE = clang ]]; then
    lcov_opts+=(--gcov-tool "${__dir}"/llvm-gcov.sh)
  fi
  lcov -c -d build/CMakeFiles -b "$(pwd -P)/src" --no-external -o coverage/lcov.info -t "$NODE_JSONNET_ENABLE_COVERAGE" ${lcov_opts[@]+"${lcov_opts[@]}"}
  sed -i.bak -e "s|^SF:$(pwd -P)/|SF:|" coverage/lcov.info
  lcov -a coverage/lcov.info -o "$TRACEFILE"
fi
popd
rm -rf "$tmpdir"
