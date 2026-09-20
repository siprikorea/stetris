#!/usr/bin/env node
/**
 * Tests for the logic, with no front end involved.
 *
 * The reference table at the bottom is shared with the C++, Java and Python
 * ports. If one of them drifts, they have stopped agreeing on the rules.
 */
import { Play } from '../stetris/index.js';

let failures = 0;

function check(name, passed, detail) {
  console.log(`  ${name.padEnd(14)} ${detail.padEnd(28)} ${passed ? 'ok' : 'FAIL'}`);
  if (!passed) {
    failures += 1;
  }
}

/** Plays a whole game out with hard drops only. */
function playOut(seed) {
  const play = new Play();
  play.newGame(seed);

  let guard = 0;
  while (!play.isGameOver && guard++ < 100000) {
    play.hardDrop();
  }
  return play.score.get();
}

function testDeterminism() {
  const first = playOut(12345);
  const second = playOut(12345);
  check('determinism', first === second, `seed 12345 -> ${first}, ${second}`);

  const other = playOut(999);
  check('variation', other !== first, `seed 999 -> ${other}`);
}

function testIdleScore() {
  const play = new Play();
  play.newGame(7);

  // Ten seconds of falling without touching a key
  for (let step = 0; step < 100; step++) {
    play.tick(100);
  }

  check('idle score', play.score.get() === 0, `10s idle -> ${play.score.get()}`);
}

function testPause() {
  const play = new Play();
  play.newGame(7);
  play.setPause(true);

  const yPos = play.currentBlock.yPos;
  for (let step = 0; step < 50; step++) {
    play.tick(100);
  }

  const frozen = play.currentBlock.yPos === yPos;
  const blocked = !play.moveLeft() && !play.rotate() && !play.hardDrop();
  check('pause', frozen && blocked, `frozen ${frozen}, input blocked ${blocked}`);

  play.setPause(false);
  check('resume', play.isPlaying && play.moveLeft(), 'moves again');
}

function testGameOver() {
  const play = new Play();
  play.newGame(42);

  let drops = 0;
  while (!play.isGameOver && drops < 100000) {
    play.hardDrop();
    drops += 1;
  }

  check('game over', play.isGameOver, `ended after ${drops} drops`);
  check('stays over', !play.hardDrop() && !play.rotate() && !play.moveLeft(),
    'input ignored');

  play.setPause(false);
  check('no revive', play.isGameOver, 'unpause does nothing');
}

function testLineClear() {
  const play = new Play();

  // Look for a seed that starts with the square block, so the shape that
  // lands in the gap left below is known
  let seed = 1;
  while (seed < 1000 && play.currentBlock.type !== 3) {
    play.newGame(seed);
    seed += 1;
  }

  if (play.currentBlock.type !== 3) {
    check('line clear', false, 'no square block seed found');
    return;
  }

  const board = play.board;
  const xSize = board.xSize;
  const ySize = board.ySize;

  // Fill the bottom two rows except the two leftmost columns
  for (let y = ySize - 2; y < ySize; y++) {
    for (let x = 2; x < xSize; x++) {
      board.set(x, y, 1);
    }
  }

  const before = play.score.get();

  while (play.moveLeft()) {
    // slide the square into the gap
  }
  play.hardDrop();

  const gained = play.score.get() - before;
  const detail = `${play.lines} lines, +${gained}`;

  check('line clear', play.lines === 2, detail);

  // Two lines at level 1 is 300, plus 2 per cell of the hard drop
  check('line score', gained >= 300, detail);

  let empty = true;
  for (let x = 0; x < xSize; x++) {
    if (board.get(x, ySize - 1)) {
      empty = false;
    }
  }
  check('rows removed', empty, 'bottom row cleared');
}

function testSpeed() {
  const play = new Play();
  play.newGame(7);
  check('fall speed', play.level === 1 && play.fallInterval === 800,
    `level ${play.level} -> ${play.fallInterval} ms`);
}

function testRestart() {
  const play = new Play();
  play.newGame(3);

  let guard = 0;
  while (!play.isGameOver && guard++ < 100000) {
    play.hardDrop();
  }

  const high = play.highScore.get();
  play.newGame(4);

  const reset = play.score.get() === 0 && play.lines === 0 && play.isPlaying;
  const kept = play.highScore.get() === high && high > 0;

  let empty = true;
  const board = play.board;
  for (let y = 0; y < board.ySize; y++) {
    for (let x = 0; x < board.xSize; x++) {
      if (board.get(x, y)) {
        empty = false;
      }
    }
  }

  check('restart', reset && empty, `high ${high} kept`);
  check('high score', kept, `high ${high} kept`);
}

function testWallKick() {
  const play = new Play();

  // The bar is the shape that needs the kick the most
  let seed = 1;
  while (seed < 1000 && play.currentBlock.type !== 7) {
    play.newGame(seed);
    seed += 1;
  }

  if (play.currentBlock.type !== 7) {
    check('wall kick', false, 'no bar block seed found');
    return;
  }

  // Stand the bar up, then push it against the right wall and turn it flat
  play.rotate();
  while (play.moveRight()) {
    // push to the wall
  }

  check('wall kick', play.rotate(), 'bar rotates at the wall');
}

/** Scenario A, a fixed move script from an empty board. */
function scenarioA(play) {
  for (let drop = 0; !play.isGameOver && drop < 100000; drop++) {
    for (let n = 0; n < drop % 4; n++) {
      play.rotate();
    }

    const shift = drop % 11;
    if (shift < 5) {
      for (let n = 0; n < 5 - shift; n++) {
        play.moveLeft();
      }
    } else {
      for (let n = 0; n < shift - 5; n++) {
        play.moveRight();
      }
    }

    if (drop % 7 === 0) {
      play.softDrop();
    }

    play.hardDrop();
  }
}

/** Scenario B, dropping into a gap so lines clear. */
function scenarioB(play) {
  // Leave a four wide gap at the right, so any shape pushed against the
  // wall drops into it and the rows fill up
  const board = play.board;
  for (let y = 14; y < board.ySize; y++) {
    for (let x = 0; x < board.xSize - 4; x++) {
      board.set(x, y, 1);
    }
  }

  for (let drop = 0; !play.isGameOver && drop < 100000; drop++) {
    while (play.moveRight()) {
      // push to the wall
    }
    play.hardDrop();
  }
}

// scenario, seed, score, lines, level
const REFERENCE = [
  ['A', 1, 176, 0, 1],
  ['A', 7, 263, 0, 1],
  ['A', 42, 267, 0, 1],
  ['A', 999, 174, 0, 1],
  ['A', 12345, 323, 0, 1],
  ['A', 2024, 353, 0, 1],
  ['A', 65535, 156, 0, 1],
  ['B', 1, 402, 1, 1],
  ['B', 7, 200, 0, 1],
  ['B', 42, 186, 0, 1],
  ['B', 999, 324, 1, 1],
  ['B', 12345, 362, 1, 1],
  ['B', 2024, 212, 0, 1],
  ['B', 65535, 198, 0, 1],
];

function testReference() {
  let mismatch = 0;

  for (const [scenario, seed, score, lines, level] of REFERENCE) {
    const play = new Play();
    play.newGame(seed);

    if (scenario === 'A') {
      scenarioA(play);
    } else {
      scenarioB(play);
    }

    if (play.score.get() !== score || play.lines !== lines || play.level !== level) {
      console.log(`    ${scenario} seed ${seed}: `
        + `got ${play.score.get()}/${play.lines}/${play.level}, `
        + `want ${score}/${lines}/${level}`);
      mismatch += 1;
    }
  }

  check('reference', mismatch === 0, `${REFERENCE.length} cases`);
}

console.log('stetris logic tests (javascript)\n');

testDeterminism();
testIdleScore();
testPause();
testGameOver();
testLineClear();
testSpeed();
testRestart();
testWallKick();
testReference();

console.log('\n' + (failures ? 'FAILED' : 'all passed'));
process.exit(failures ? 1 : 0);
