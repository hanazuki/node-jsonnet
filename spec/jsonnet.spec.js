import {describe, it} from 'vitest';

import fs from 'node:fs';
import path from 'node:path';

import {Jsonnet} from '../';

const quoteRegExp = str => str.replace(/[.*+\-?^${}()|[\]\\]/g, '\\$&');

const stackTraceLike = expected => ({
  asymmetricMatch(actual) {
    const stripped = actual
      .replace(/\tstd\.jsonnet:[0-9]*:[0-9-]*/g, "\tstd.jsonnet:<stdlib_position_redacted>");

    return stripped === expected;
  },
  toString() {
    return expected;
  },
});

const suiteDir = path.join(__dirname, '../third_party/jsonnet/test_suite');
const examples = fs.readdirSync(suiteDir, {withFileTypes: true})
                   .filter(de => de.isFile() && /\.jsonnet$/.test(de.name));

describe('libjsonnet', ({beforeEach}) => {
  beforeEach(() => {
    process.chdir(suiteDir);
  });

  for (const example of examples) {
    const fname = path.basename(example.name);

    it(`process ${fname}`, {timeout: 60_000}, async ({expect, skip}) => {

      if(/^trace\./.test(fname)) {
        skip('TODO: Test tracing');
      }

      const expectsError = /^error\./.test(fname);

      const jsonnet = new Jsonnet();
      if(/^tla\./.test(fname)) {
        jsonnet.tlaString('var1', 'test').tlaCode('var2', `{x: 1, y: 2}`);
      } else {
        jsonnet.extString('var1', 'test').extCode('var2', `{x: 1, y: 2}`);
      }

      const golden = await fs.promises.readFile(`${fname}.golden`, {encoding: 'utf8'}).catch(() => "true\n");

      const result = jsonnet.evaluateFile(fname)

      if(expectsError) {
        await expect(result).rejects.toThrowError(
          expect.objectContaining({message: stackTraceLike(golden)})
        )
      } else {
        await expect(result).resolves.toEqual(golden);
      }
    });
  }
});
