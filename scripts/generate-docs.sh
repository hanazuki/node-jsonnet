#!/bin/bash
set -ex

if [[ $GITHUB_REF = refs/tags/* ]]; then
  dir=${GITHUB_REF#refs/tags/}
elif [[ $GITHUB_REF = refs/heads/* ]]; then
  dir=${GITHUB_REF#refs/heads/}
else
  exit 1
fi

commit=$(git rev-parse HEAD)

git fetch origin docs

rm -rf docs
git worktree add -f docs -B docs -- origin/docs

(cd types; npm exec -- typedoc --out "../docs/${dir}")

cd docs

git add -A
if git commit -m 'Generate docs' -m "Source: $commit"; then
  if ! git push origin docs; then
    git pull origin docs --rebase
    git push origin docs
  fi
else
  echo "No changes"
fi
