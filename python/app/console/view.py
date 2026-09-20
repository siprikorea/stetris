"""Renders a Play to the terminal.

The logic knows nothing about this module - the app pulls the state it
needs and draws it.
"""

import sys

# Screen control
ESC_HIDE_CURSOR = "\033[?25l"
ESC_SHOW_CURSOR = "\033[?25h"
ESC_CLEAR_SCREEN = "\033[2J"
ESC_HOME = "\033[H"
ESC_CLEAR_LINE = "\033[K"
ESC_RESET = "\033[0m"

CELL_FILLED = "[]"
CELL_EMPTY = " ."

# Side panel column, one based
SIDE_COL = 28

# Colour per block type, index 0 is the empty cell
COLORS = (
    "\033[0;90m",   # empty - gray
    "\033[1;34m",   # J - blue
    "\033[1;37m",   # L - white
    "\033[1;33m",   # O - yellow
    "\033[1;32m",   # S - green
    "\033[1;35m",   # T - magenta
    "\033[1;31m",   # Z - red
    "\033[1;36m",   # I - cyan
)


class ConsoleView:
    def __init__(self) -> None:
        self._out = []

    def enter_screen(self) -> None:
        sys.stdout.write(ESC_HIDE_CURSOR + ESC_CLEAR_SCREEN + ESC_HOME)
        sys.stdout.flush()

    def leave_screen(self) -> None:
        sys.stdout.write(ESC_RESET + ESC_SHOW_CURSOR + "\n")
        sys.stdout.flush()

    def render(self, play) -> None:
        """Draw one frame."""
        self._out = [ESC_HOME]
        self._draw_board(play)
        self._draw_help()

        # Flush the whole frame at once
        sys.stdout.write("".join(self._out))
        sys.stdout.flush()

    def _put(self, text: str) -> None:
        self._out.append(text)

    def _put_cell(self, cell_type: int) -> None:
        colour = COLORS[cell_type] if 0 <= cell_type < len(COLORS) else COLORS[0]
        self._put(colour + (CELL_FILLED if cell_type else CELL_EMPTY) + ESC_RESET)

    def _draw_board(self, play) -> None:
        board = play.board
        block = play.current_block
        x_size = board.x_size
        y_size = board.y_size

        # Merge the board and the falling block into one buffer
        screen = [[board.get_value(x, y) for x in range(x_size)]
                  for y in range(y_size)]

        # A locked block is drawn from the board, so only overlay the
        # falling one while the game is running
        if not play.is_game_over:
            for by in range(block.y_size):
                for bx in range(block.x_size):
                    if not block.get_cell(bx, by):
                        continue
                    x = block.x_pos + bx
                    y = block.y_pos + by
                    if 0 <= x < x_size and 0 <= y < y_size:
                        screen[y][x] = block.type

        self._put("\033[1;37m       S T E T R I S\033[0m" + ESC_CLEAR_LINE + "\n")
        self._put("  +" + "--" * x_size + "+" + ESC_CLEAR_LINE + "\n")

        for y in range(y_size):
            self._put("  |")
            for x in range(x_size):
                self._put_cell(screen[y][x])
            self._put("|")

            # Side panel on the same line
            self._put(f"\033[{SIDE_COL}G")
            self._draw_side(play, y)

            self._put(ESC_CLEAR_LINE + "\n")

        self._put("  +" + "--" * x_size + "+" + ESC_CLEAR_LINE + "\n")

    def _draw_side(self, play, line: int) -> None:
        if line == 0:
            self._put("\033[1;37mNEXT\033[0m")
        elif 1 <= line <= 4:
            # The preview is drawn at the origin of the panel, the spawn
            # position of the next block is irrelevant here
            nxt = play.next_block
            by = line - 1
            for bx in range(4):
                self._put_cell(nxt.type if nxt.get_cell(bx, by) else 0)
        elif line == 6:
            self._put("\033[1;37mSCORE\033[0m")
        elif line == 7:
            self._put(str(play.score.get()))
        elif line == 9:
            self._put("\033[1;37mHIGH SCORE\033[0m")
        elif line == 10:
            self._put(str(play.high_score.get()))
        elif line == 12:
            self._put("\033[1;37mLEVEL\033[0m")
        elif line == 13:
            self._put(str(play.level))
        elif line == 15:
            self._put("\033[1;37mLINES\033[0m")
        elif line == 16:
            self._put(str(play.lines))
        elif line == 18:
            if play.is_game_over:
                self._put("\033[1;31mGAME OVER\033[0m")
            elif play.is_paused:
                self._put("\033[1;33mPAUSED\033[0m")
        elif line == 19:
            if play.is_game_over:
                self._put("\033[1;31mpress R to restart\033[0m")

    def _draw_help(self) -> None:
        self._put(ESC_CLEAR_LINE + "\n")
        self._put("  \033[0;90mLeft/Right: move   Up: rotate   Down: soft drop\033[0m"
                  + ESC_CLEAR_LINE + "\n")
        self._put("  \033[0;90mSpace: hard drop   P: pause     R: restart   Q: quit\033[0m"
                  + ESC_CLEAR_LINE + "\n")
