#!/bin/bash
set -eux

npm build
rm -f *.tgz
npm pack
tmpdir=$(mktemp -d)
tar xf hanazuki-node-jsonnet-*.tgz -C "$tmpdir"
cd "$tmpdir/package"
npm install-test
