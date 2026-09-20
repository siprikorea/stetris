#!/usr/bin/env node
/**
 * Terminal front end.
 *
 * Everything the game does not do lives here: reading keys, measuring
 * elapsed time to hand to tick(), seeding a new game and storing the high
 * score in a file.
 */
import fs from 'node:fs';
import os from 'node:os';
import path from 'node:path';
import process from 'node:process';

import { Play } from '../../stetris/index.js';
import { ConsoleView } from './view.js';

// Keys
const KEY = {
  NONE: 0, LEFT: 1, RIGHT: 2, DOWN: 3, UP: 4,
  DROP: 5, PAUSE: 6, RESTART: 7, QUIT: 8,
};

/** Input poll interval in milliseconds. */
const POLL_INTERVAL = 20;

const CHAR_KEYS = {
  ' ': KEY.DROP,
  p: KEY.PAUSE, P: KEY.PAUSE,
  r: KEY.RESTART, R: KEY.RESTART,
  q: KEY.QUIT, Q: KEY.QUIT,
  // WASD as an alternative to the arrow keys
  a: KEY.LEFT, A: KEY.LEFT,
  d: KEY.RIGHT, D: KEY.RIGHT,
  s: KEY.DOWN, S: KEY.DOWN,
  w: KEY.UP, W: KEY.UP,
};

const ARROW_KEYS = { A: KEY.UP, B: KEY.DOWN, C: KEY.RIGHT, D: KEY.LEFT };

function highScorePath() {
  return path.join(os.homedir() || '.', '.stetris_highscore');
}

function loadHighScore() {
  try {
    const value = Number.parseInt(fs.readFileSync(highScorePath(), 'utf8').trim(), 10);
    return Number.isFinite(value) ? value : 0;
  } catch {
    return 0;
  }
}

function saveHighScore(value) {
  try {
    fs.writeFileSync(highScorePath(), `${value}\n`);
  } catch {
    // Not being able to keep the high score is not worth failing over
  }
}

/** Turns a chunk of input into key codes, arrow escapes included. */
function decodeKeys(text) {
  const keys = [];

  for (let i = 0; i < text.length; i++) {
    const ch = text[i];

    // Arrow keys arrive as "ESC [ A" ~ "ESC [ D"
    if (ch === '\x1b' && text[i + 1] === '[') {
      const key = ARROW_KEYS[text[i + 2]];
      if (key !== undefined) {
        keys.push(key);
      }
      i += 2;
      continue;
    }

    // Ctrl-C, which raw mode would otherwise swallow
    if (ch === '\x03') {
      keys.push(KEY.QUIT);
      continue;
    }

    const key = CHAR_KEYS[ch];
    if (key !== undefined) {
      keys.push(key);
    }
  }

  return keys;
}

function main() {
  const play = new Play();
  play.newGame(Math.floor(Date.now() / 1000));

  // The high score is stored by the platform, the game only holds it
  play.highScore.set(loadHighScore());

  const view = new ConsoleView();

  // A piped stdin has no raw mode to set, which is what makes the app
  // runnable from a script
  const isTty = process.stdin.isTTY;
  if (isTty) {
    process.stdin.setRawMode(true);
  }
  process.stdin.resume();
  process.stdin.setEncoding('utf8');

  const pending = [];
  let ended = false;
  process.stdin.on('data', (chunk) => pending.push(...decodeKeys(chunk)));
  process.stdin.on('end', () => { ended = true; });

  view.enterScreen();

  let savedScore = false;
  let lastTick = Date.now();
  view.render(play);

  const timer = setInterval(() => {
    const key = pending.shift() ?? KEY.NONE;

    switch (key) {
      case KEY.QUIT:
        return finish();
      case KEY.RESTART:
        play.newGame(Math.floor(Date.now() / 1000));
        savedScore = false;
        lastTick = Date.now();
        break;
      case KEY.PAUSE: play.togglePause(); break;
      case KEY.LEFT: play.moveLeft(); break;
      case KEY.RIGHT: play.moveRight(); break;
      case KEY.UP: play.rotate(); break;
      case KEY.DOWN: play.softDrop(); break;
      case KEY.DROP: play.hardDrop(); break;
      default: break;
    }

    // Hand the elapsed time to the game, it decides when to fall
    const now = Date.now();
    play.tick(now - lastTick);
    lastTick = now;

    // Persist the high score as soon as the game ends
    if (play.isGameOver && !savedScore) {
      saveHighScore(play.highScore.get());
      savedScore = true;
    }

    view.render(play);

    // Nothing more can arrive on a pipe that has closed
    if (ended && pending.length === 0) {
      finish();
    }
  }, POLL_INTERVAL);

  function finish() {
    clearInterval(timer);
    saveHighScore(play.highScore.get());

    view.leaveScreen();
    if (isTty) {
      process.stdin.setRawMode(false);
    }
    process.stdin.pause();

    process.stdout.write(`SCORE ${play.score.get()}   `
      + `HIGH SCORE ${play.highScore.get()}   `
      + `LEVEL ${play.level}   LINES ${play.lines}\n`);
    process.exit(0);
  }
}

main();
