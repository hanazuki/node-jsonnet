const fs = require('fs').promises;
const path = require('path');

describe('package', () => {

  const licenseFiles = [
    'LICENSE',
    'third_party/jsonnet/LICENSE',
    'third_party/jsonnet/third_party/json/LICENSE',
    'third_party/jsonnet/third_party/md5/LICENSE',
  ];

  for (const licenseFile of licenseFiles) {
    it(`contains ${licenseFile}`, async () => {
      return expectAsync(fs.stat(path.join(__dirname, '..', licenseFile))).toBeResolved();
    });
  }

});
