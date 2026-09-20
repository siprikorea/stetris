/**
 * Renders a Play to the terminal.
 *
 * The logic knows nothing about this module - the app pulls the state it
 * needs and draws it.
 */

// Screen control
const ESC_HIDE_CURSOR = '\x1b[?25l';
const ESC_SHOW_CURSOR = '\x1b[?25h';
const ESC_CLEAR_SCREEN = '\x1b[2J';
const ESC_HOME = '\x1b[H';
const ESC_CLEAR_LINE = '\x1b[K';
const ESC_RESET = '\x1b[0m';

const CELL_FILLED = '[]';
const CELL_EMPTY = ' .';

/** Side panel column, one based. */
const SIDE_COL = 28;

/** Colour per block type, index 0 is the empty cell. */
const COLORS = [
  '\x1b[0;90m', // empty - gray
  '\x1b[1;34m', // J - blue
  '\x1b[1;37m', // L - white
  '\x1b[1;33m', // O - yellow
  '\x1b[1;32m', // S - green
  '\x1b[1;35m', // T - magenta
  '\x1b[1;31m', // Z - red
  '\x1b[1;36m', // I - cyan
];

export class ConsoleView {
  #out = [];

  enterScreen() {
    process.stdout.write(ESC_HIDE_CURSOR + ESC_CLEAR_SCREEN + ESC_HOME);
  }

  leaveScreen() {
    process.stdout.write(ESC_RESET + ESC_SHOW_CURSOR + '\n');
  }

  /** Draws one frame. */
  render(play) {
    this.#out = [ESC_HOME];
    this.#drawBoard(play);
    this.#drawHelp();

    // Flush the whole frame at once
    process.stdout.write(this.#out.join(''));
  }

  #put(text) {
    this.#out.push(text);
  }

  #putCell(type) {
    const colour = COLORS[type] ?? COLORS[0];
    this.#put(colour + (type ? CELL_FILLED : CELL_EMPTY) + ESC_RESET);
  }

  #drawBoard(play) {
    const board = play.board;
    const block = play.currentBlock;
    const xSize = board.xSize;
    const ySize = board.ySize;

    // Merge the board and the falling block into one buffer
    const screen = [];
    for (let y = 0; y < ySize; y++) {
      const row = [];
      for (let x = 0; x < xSize; x++) {
        row.push(board.getValue(x, y));
      }
      screen.push(row);
    }

    // A locked block is drawn from the board, so only overlay the falling
    // one while the game is running
    if (!play.isGameOver) {
      for (let by = 0; by < block.ySize; by++) {
        for (let bx = 0; bx < block.xSize; bx++) {
          if (!block.getCell(bx, by)) {
            continue;
          }
          const x = block.xPos + bx;
          const y = block.yPos + by;
          if (x >= 0 && x < xSize && y >= 0 && y < ySize) {
            screen[y][x] = block.type;
          }
        }
      }
    }

    this.#put(`\x1b[1;37m       S T E T R I S${ESC_RESET}${ESC_CLEAR_LINE}\n`);
    this.#put(`  +${'--'.repeat(xSize)}+${ESC_CLEAR_LINE}\n`);

    for (let y = 0; y < ySize; y++) {
      this.#put('  |');
      for (let x = 0; x < xSize; x++) {
        this.#putCell(screen[y][x]);
      }
      this.#put('|');

      // Side panel on the same line
      this.#put(`\x1b[${SIDE_COL}G`);
      this.#drawSide(play, y);

      this.#put(ESC_CLEAR_LINE + '\n');
    }

    this.#put(`  +${'--'.repeat(xSize)}+${ESC_CLEAR_LINE}\n`);
  }

  #drawSide(play, line) {
    switch (line) {
      case 0:
        this.#put(`\x1b[1;37mNEXT${ESC_RESET}`);
        break;
      case 1:
      case 2:
      case 3:
      case 4: {
        // The preview is drawn at the origin of the panel, the spawn
        // position of the next block is irrelevant here
        const next = play.nextBlock;
        const by = line - 1;
        for (let bx = 0; bx < 4; bx++) {
          this.#putCell(next.getCell(bx, by) ? next.type : 0);
        }
        break;
      }
      case 6:
        this.#put(`\x1b[1;37mSCORE${ESC_RESET}`);
        break;
      case 7:
        this.#put(String(play.score.get()));
        break;
      case 9:
        this.#put(`\x1b[1;37mHIGH SCORE${ESC_RESET}`);
        break;
      case 10:
        this.#put(String(play.highScore.get()));
        break;
      case 12:
        this.#put(`\x1b[1;37mLEVEL${ESC_RESET}`);
        break;
      case 13:
        this.#put(String(play.level));
        break;
      case 15:
        this.#put(`\x1b[1;37mLINES${ESC_RESET}`);
        break;
      case 16:
        this.#put(String(play.lines));
        break;
      case 18:
        if (play.isGameOver) {
          this.#put(`\x1b[1;31mGAME OVER${ESC_RESET}`);
        } else if (play.isPaused) {
          this.#put(`\x1b[1;33mPAUSED${ESC_RESET}`);
        }
        break;
      case 19:
        if (play.isGameOver) {
          this.#put(`\x1b[1;31mpress R to restart${ESC_RESET}`);
        }
        break;
      default:
        break;
    }
  }

  #drawHelp() {
    this.#put(ESC_CLEAR_LINE + '\n');
    this.#put(`  \x1b[0;90mLeft/Right: move   Up: rotate   Down: soft drop${ESC_RESET}${ESC_CLEAR_LINE}\n`);
    this.#put(`  \x1b[0;90mSpace: hard drop   P: pause     R: restart   Q: quit${ESC_RESET}${ESC_CLEAR_LINE}\n`);
  }
}
