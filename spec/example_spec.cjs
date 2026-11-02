const path = require('path');
const cp = require('child_process');
const { promisify } = require('util');

const glob = require('glob').globSync;
const execFile = promisify(cp.execFile);

const jsExamples = "../examples/*.{mjs,cjs}";
const tsExamples = "../examples/*.ts";
const globOptions = { cwd: __dirname, absolute: true };

const timeout = 10000;

describe('JavaScript examples', () => {
  for (const file of glob(jsExamples, globOptions)) {
    it(`${file}`, async () => {
      await execFile(process.execPath, [file], {
        cwd: path.join(__dirname, '..'),
        timeout,
      });
    });
  }
});

describe('TypeScript examples', () => {
  for (const file of glob(tsExamples, globOptions)) {
    it(`${file}`, async () => {
      await execFile(process.execPath, ['--import', 'tsx', file], {
        cwd: path.join(__dirname, '../examples'),
        timeout,
      });
    });
  }
});
