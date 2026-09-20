#!/usr/bin/env python3
"""Terminal front end.

Everything the game does not do lives here: reading keys, measuring
elapsed time to hand to tick(), seeding a new game and storing the high
score in a file.
"""

import os
import select
import sys
import termios
import time
import tty
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[2]))

from stetris import Play                        # noqa: E402
from view import ConsoleView                    # noqa: E402

# Keys
KEY_NONE, KEY_LEFT, KEY_RIGHT, KEY_DOWN, KEY_UP = range(5)
KEY_DROP, KEY_PAUSE, KEY_RESTART, KEY_QUIT = range(5, 9)

# Input poll interval in seconds
POLL_INTERVAL = 0.02

_CHAR_KEYS = {
    " ": KEY_DROP,
    "p": KEY_PAUSE, "P": KEY_PAUSE,
    "r": KEY_RESTART, "R": KEY_RESTART,
    "q": KEY_QUIT, "Q": KEY_QUIT,
    # WASD as an alternative to the arrow keys
    "a": KEY_LEFT, "A": KEY_LEFT,
    "d": KEY_RIGHT, "D": KEY_RIGHT,
    "s": KEY_DOWN, "S": KEY_DOWN,
    "w": KEY_UP, "W": KEY_UP,
}

_ARROW_KEYS = {"A": KEY_UP, "B": KEY_DOWN, "C": KEY_RIGHT, "D": KEY_LEFT}


def high_score_path() -> Path:
    home = os.environ.get("HOME")
    return Path(home, ".stetris_highscore") if home else Path(".stetris_highscore")


def load_high_score() -> int:
    try:
        return int(high_score_path().read_text().strip())
    except (OSError, ValueError):
        return 0


def save_high_score(value: int) -> None:
    try:
        high_score_path().write_text(f"{value}\n")
    except OSError:
        pass


def read_key() -> int:
    """Return a pending key, or KEY_NONE when there is none."""
    if not select.select([sys.stdin], [], [], 0)[0]:
        return KEY_NONE

    ch = sys.stdin.read(1)
    if not ch:
        return KEY_NONE

    # Arrow keys arrive as "ESC [ A" ~ "ESC [ D"
    if ch == "\033":
        if not select.select([sys.stdin], [], [], 0)[0]:
            return KEY_NONE
        if sys.stdin.read(1) != "[":
            return KEY_NONE
        return _ARROW_KEYS.get(sys.stdin.read(1), KEY_NONE)

    return _CHAR_KEYS.get(ch, KEY_NONE)


def main() -> int:
    play = Play()
    play.new_game(int(time.time()))

    # The high score is stored by the platform, the game only holds it
    play.high_score.set(load_high_score())

    view = ConsoleView()

    # A piped stdin has no terminal settings to change, which is what
    # makes the app runnable from a script
    saved = None
    if sys.stdin.isatty():
        saved = termios.tcgetattr(sys.stdin)
        tty.setcbreak(sys.stdin.fileno())

    try:
        view.enter_screen()

        quit_game = False
        saved_score = False
        last_tick = time.monotonic()
        view.render(play)

        while not quit_game:
            key = read_key()

            if key == KEY_QUIT:
                quit_game = True
            elif key == KEY_RESTART:
                play.new_game(int(time.time()))
                saved_score = False
                last_tick = time.monotonic()
            elif key == KEY_PAUSE:
                play.toggle_pause()
            elif key == KEY_LEFT:
                play.move_left()
            elif key == KEY_RIGHT:
                play.move_right()
            elif key == KEY_UP:
                play.rotate()
            elif key == KEY_DOWN:
                play.soft_drop()
            elif key == KEY_DROP:
                play.hard_drop()

            # Hand the elapsed time to the game, it decides when to fall
            now = time.monotonic()
            play.tick(int((now - last_tick) * 1000))
            last_tick = now

            # Persist the high score as soon as the game ends
            if play.is_game_over and not saved_score:
                save_high_score(play.high_score.get())
                saved_score = True

            view.render(play)
            time.sleep(POLL_INTERVAL)

        save_high_score(play.high_score.get())
        view.leave_screen()
    finally:
        if saved is not None:
            termios.tcsetattr(sys.stdin, termios.TCSADRAIN, saved)

    print(f"SCORE {play.score.get()}   HIGH SCORE {play.high_score.get()}   "
          f"LEVEL {play.level}   LINES {play.lines}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
