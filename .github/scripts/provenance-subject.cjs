// @ts-check

const { execFile } = require('node:child_process');
const { createHash } = require('node:crypto');
const { readFile, readdir } = require('node:fs/promises');
const { promisify } = require('node:util');

const execFileAsync = promisify(execFile);

/**
 * @param {import('@actions/github-script').AsyncFunctionArguments} args
 */
module.exports = async ({ core }) => {
  const tarballs = (await readdir('pkg')).filter(name => name.endsWith('.tgz'));
  if (tarballs.length !== 1) throw new Error('Expected exactly one package tarball');
  const tarball = `pkg/${tarballs[0]}`;
  const { stdout } = await execFileAsync('tar', ['-xOf', tarball, 'package/package.json'], { encoding: 'utf8' });
  const { name, version } = JSON.parse(stdout);
  const subject = `pkg:npm/${name.replace(/^@/, '%40')}@${version}`;
  const digest = createHash('sha512').update(await readFile(tarball)).digest('hex');
  core.setOutput('name', subject);
  core.setOutput('digest', `sha512:${digest}`);
};
