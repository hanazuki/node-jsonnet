#!/usr/bin/env nodejs

const fs = require('fs');
const path = require('path');
const { execFileSync } = require('child_process');

const PACKAGE_JSON = path.join(__dirname, '..', 'package.json');
const CHANGELOG_MD = path.join(__dirname, '..', 'CHANGELOG.md');

const { version } = JSON.parse(fs.readFileSync(PACKAGE_JSON, 'utf8'));
const today = new Date().toISOString().split('T')[0];

let changelog = fs.readFileSync(CHANGELOG_MD, 'utf8');

changelog = changelog.replace(
  '## UNRELEASED',
  `## UNRELEASED\n\n## v${version} (${today})`
);

fs.writeFileSync(CHANGELOG_MD, changelog);

execFileSync('git', ['add', CHANGELOG_MD]);
