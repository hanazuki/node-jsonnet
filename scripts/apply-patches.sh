#!/bin/bash
set -euo pipefail

git submodule update third_party/jsonnet
find patches/jsonnet/ -maxdepth 1 -name '*.patch' -print0 | sort -z | while IFS= read -r -d '' patch; do
  git -C third_party/jsonnet am <"$patch"
done
