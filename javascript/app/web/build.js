#!/usr/bin/env node
/**
 * Builds a self contained copy of the web app into app/web/dist/.
 *
 * The page normally imports the logic from ../../stetris/ and the icon
 * from res/ at the top of the repository. Neither path exists once the
 * folder is deployed on its own, so the two references are rewritten to
 * point inside dist/. Those rewrites are the only difference between
 * what runs locally and what ships.
 */
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

const HERE = path.dirname(fileURLToPath(import.meta.url));
const JS_ROOT = path.resolve(HERE, '../..');
const REPO = path.resolve(JS_ROOT, '..');
const DIST = path.join(HERE, 'dist');

fs.rmSync(DIST, { recursive: true, force: true });
fs.mkdirSync(path.join(DIST, 'lib'), { recursive: true });

// The logic, untouched. Only the modules: the folder can also hold
// stray files that editors and tooling leave behind.
const modules = fs.readdirSync(path.join(JS_ROOT, 'stetris'))
  .filter((name) => name.endsWith('.js'))
  .sort();

if (modules.length === 0) {
  throw new Error('no modules found to copy');
}

for (const name of modules) {
  fs.copyFileSync(path.join(JS_ROOT, 'stetris', name), path.join(DIST, 'lib', name));
}

// The page, with the icon path pointing at the copy beside it
const html = fs.readFileSync(path.join(HERE, 'index.html'), 'utf8')
  .replace('../../../res/stetris_icon.png', './icon.png');
fs.writeFileSync(path.join(DIST, 'index.html'), html);

// The app, with the import pointing at the copied logic
const app = fs.readFileSync(path.join(HERE, 'app.js'), 'utf8')
  .replace("from '../../stetris/index.js'", "from './lib/index.js'");

if (app.includes('../../stetris/')) {
  throw new Error('an import still points outside dist/');
}

fs.writeFileSync(path.join(DIST, 'app.js'), app);
fs.copyFileSync(path.join(REPO, 'res', 'stetris_icon.png'), path.join(DIST, 'icon.png'));

console.log(`built ${DIST}`);
console.log(`  ${modules.length} modules + index.html, app.js, icon.png`);
