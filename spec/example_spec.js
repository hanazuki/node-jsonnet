const fs = require('fs');
const path = require('path');
const cp = require('child_process');
const { promisify } = require('util');

const glob = require('glob').sync;
const execFile = promisify(cp.execFile);

const jsExamples = path.join(__dirname, "../examples/*.{mjs,cjs}");
const tsExamples = path.join(__dirname, "../examples/*.ts");

const timeout = 3000;

describe('JavaScript examples', () => {
  for(const file of glob(jsExamples)) {
    it(`${file}`, async () => execFile('node', [file], {
      cwd: path.join(__dirname, '..'),
      timeout,
    }))
  }
});

describe('TypeScript examples', () => {
  for(const file of glob(tsExamples)) {
    it(`${file}`, async () => execFile('ts-node', [file], {
      cwd: path.join(__dirname, '../examples'),
      timeout,
    }))
  }
});
