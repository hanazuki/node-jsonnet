import {describe, expect, it} from 'vitest';

import fs from 'node:fs';
import path from 'node:path';
import cp from 'node:child_process';
import {promisify} from 'node:util';

const glob = require('glob').globSync;
const execFile = promisify(cp.execFile);

const jsExamples = path.join(__dirname, '../examples/*.{mjs,cjs}');
const tsExamples = path.join(__dirname, '../examples/*.ts');

const timeout = 10000;

describe('JavaScript examples', () => {
  for(const file of glob(jsExamples)) {
    it(`${file}`, async () => {
      await execFile('node', [file], {
        cwd: path.join(__dirname, '..'),
        timeout,
      });
    });
  }
});

describe('TypeScript examples', () => {
  for(const file of glob(tsExamples)) {
    it(`${file}`, async () => {
      await execFile('ts-node', [file], {
        cwd: path.join(__dirname, '../examples'),
        timeout,
      });
    });
  }
});
