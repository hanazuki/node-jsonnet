// @ts-check

const fs = require('node:fs/promises');
const path = require('node:path');

/**
 * @param {string} filename
 * @return {string}
 */
function contentType(filename) {
  const ext = path.extname(filename);
  if (ext == '.tgz') {
    return 'application/gzip';
  }
  if (ext == '.json') {
    return 'application/vnd.dev.sigstore.bundle+json';
  }
  return "application/octet-stream";
}

/**
 * @param {import('@actions/github-script').AsyncFunctionArguments["github"]} github
 * @param {{owner: string; repo: string}} repo
 * @param {string} tag
 * @param {boolean} prerelease
 */
async function getOrCreateRelease(github, repo, tag, prerelease) {
  try {
    return await github.rest.repos.getReleaseByTag({
      ...repo, tag,
    });
  } catch (/** @type {any} */ err) {
    if (err.status !== 404) throw err;
  }

  return await github.rest.repos.createRelease({
    ...repo,
    tag_name: tag,
    name: tag,
    prerelease,
    draft: true,
  });
}

/**
 * @param {import('@actions/github-script').AsyncFunctionArguments} AsyncFunctionArguments
 */
module.exports = async ({ github, context, core }) => {
  const ref = (
    await github.rest.git.getRef({
      ...context.repo,
      ref: context.ref.replace(/^refs\//, ""),
    })
  ).data;
  if (ref.object.type !== "tag") {
    throw new Error("ref is not an annotated tag");
  }

  const tag = (
    await github.rest.git.getTag({ ...context.repo, tag_sha: ref.object.sha })
  ).data;
  if (!tag.verification?.verified) {
    throw new Error(`tag is not verified: ${tag.verification?.reason}`);
  }

  const tagName = tag.tag;
  const prerelease = !!tagName.replace(/^v/, "").match(/[a-z]/i);

  const release = (
    await getOrCreateRelease(
      github,
      context.repo,
      tagName,
      prerelease,
    )
  ).data;

  if (release.draft) {
    const pkgdir = './pkg';
    await using dir = await fs.opendir(pkgdir, { encoding: 'utf-8' });
    for await (const dirent of dir) {
      if (!dirent.isFile()) continue;

      const name = dirent.name;
      if (release.assets.some((asset) => asset.name == name)) continue;

      await using file = await fs.open(path.join(pkgdir, dirent.name));

      /** @type {any} */
      const data = file.createReadStream();
      const size = (await file.stat()).size;

      await github.rest.repos.uploadReleaseAsset({
        ...context.repo,
        release_id: release.id,
        name: dirent.name,
        data,
        headers: {
          'content-type': contentType(name),
          'content-length': size,
        },
      })
    }

    await github.rest.repos.updateRelease({
      ...context.repo,
      release_id: release.id,
      draft: false,
    })
  }

  core.setOutput('prerelease', String(prerelease));
};
