#!/usr/bin/env node
/**
 * A static server for the web app.
 *
 * ES modules are blocked over file://, so the page needs to be served.
 * The root is the repository, so the page can reach both the logic next
 * to it and res/ above it. Standard library only, like everything else.
 */
import fs from 'node:fs';
import http from 'node:http';
import path from 'node:path';
import process from 'node:process';
import { fileURLToPath } from 'node:url';

const REPO = path.resolve(fileURLToPath(new URL('../../..', import.meta.url)));
const PORT = Number.parseInt(process.env.PORT ?? '8080', 10);
const HOME = '/javascript/app/web/index.html';

const TYPES = {
  '.html': 'text/html; charset=utf-8',
  '.js': 'text/javascript; charset=utf-8',
  '.css': 'text/css; charset=utf-8',
  '.json': 'application/json; charset=utf-8',
  '.png': 'image/png',
  '.ico': 'image/x-icon',
};

const server = http.createServer((req, res) => {
  const url = new URL(req.url, 'http://localhost');
  const file = path.resolve(REPO, '.' + (url.pathname === '/' ? HOME : url.pathname));

  // Never serve anything outside the repository
  if (file !== REPO && !file.startsWith(REPO + path.sep)) {
    res.writeHead(403).end('forbidden');
    return;
  }

  fs.readFile(file, (err, body) => {
    if (err) {
      res.writeHead(404, { 'content-type': 'text/plain' }).end('not found');
      return;
    }
    res.writeHead(200, {
      'content-type': TYPES[path.extname(file)] ?? 'application/octet-stream',
      'cache-control': 'no-store',
    }).end(body);
  });
});

server.listen(PORT, () => {
  console.log(`stetris web  ->  http://localhost:${PORT}${HOME}`);
  console.log('ctrl-c to stop');
});
