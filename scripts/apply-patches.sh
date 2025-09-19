#!/bin/sh
set -eux

git submodule update third_party/jsonnet
for patch in patches/jsonnet/*; do
  git -C third_party/jsonnet am <"$patch"
done
