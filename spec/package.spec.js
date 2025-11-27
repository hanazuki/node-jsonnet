import {describe, it} from 'vitest';

import fs from 'node:fs/promises';
import path from 'node:path';

describe('package', () => {

  const licenseFiles = [
    'LICENSE',
    'third_party/jsonnet/LICENSE',
    'third_party/jsonnet/third_party/json/LICENSE',
    'third_party/jsonnet/third_party/md5/LICENSE',
  ];

  for (const licenseFile of licenseFiles) {
    it(`contains ${licenseFile}`, async ({expect}) => {
      return await expect(fs.stat(path.join(__dirname, '..', licenseFile))).resolves;
    });
  }

});
