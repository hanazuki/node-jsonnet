#!/bin/bash
set -ex

if [[ $GITHUB_REF = refs/tags/* ]]; then
  dir=${GITHUB_REF#refs/tags/}
elif [[ $GITHUB_REF = refs/heads/* ]]; then
  dir=${GITHUB_REF#refs/heads/}
else
  exit 1
fi

git fetch origin docs

rm -rf docs
git worktree add -f docs -B docs -- origin/docs

(cd types; npx -p typedoc -p typescript typedoc --out "../docs/${dir}")

cd docs

git add -A
if git commit -m 'Generate docs'; then
  if !git push origin docs; then
    git pull origin docs --rebase
    git push origin docs
  fi
else
  echo "No changes"
fi
