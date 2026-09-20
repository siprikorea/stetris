#!/usr/bin/env python3
"""Tests for the logic, with no front end involved.

The reference table at the bottom is shared with the C++ and Java ports. If
one of them drifts, the three have stopped agreeing on the rules.
"""

import sys
from pathlib import Path

# The package sits next to this folder, not inside it
sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

from stetris import Play

_failures = 0


def check(name: str, passed: bool, detail: str) -> None:
    global _failures
    print(f"  {name:<14} {detail:<28} {'ok' if passed else 'FAIL'}")
    if not passed:
        _failures += 1


def play_out(seed: int) -> int:
    """Play a whole game out with hard drops only."""
    play = Play()
    play.new_game(seed)

    guard = 0
    while not play.is_game_over and guard < 100000:
        play.hard_drop()
        guard += 1
    return play.score.get()


def test_determinism() -> None:
    first = play_out(12345)
    second = play_out(12345)
    check("determinism", first == second, f"seed 12345 -> {first}, {second}")

    other = play_out(999)
    check("variation", other != first, f"seed 999 -> {other}")


def test_idle_score() -> None:
    play = Play()
    play.new_game(7)

    # Ten seconds of falling without touching a key
    for _ in range(100):
        play.tick(100)

    check("idle score", play.score.get() == 0, f"10s idle -> {play.score.get()}")


def test_pause() -> None:
    play = Play()
    play.new_game(7)
    play.set_pause(True)

    y_pos = play.current_block.y_pos
    for _ in range(50):
        play.tick(100)

    frozen = play.current_block.y_pos == y_pos
    blocked = not play.move_left() and not play.rotate() and not play.hard_drop()
    check("pause", frozen and blocked, f"frozen {frozen}, input blocked {blocked}")

    play.set_pause(False)
    check("resume", play.is_playing and play.move_left(), "moves again")


def test_game_over() -> None:
    play = Play()
    play.new_game(42)

    drops = 0
    while not play.is_game_over and drops < 100000:
        play.hard_drop()
        drops += 1

    check("game over", play.is_game_over, f"ended after {drops} drops")

    locked = not play.hard_drop() and not play.rotate() and not play.move_left()
    check("stays over", locked, "input ignored")

    play.set_pause(False)
    check("no revive", play.is_game_over, "unpause does nothing")


def test_line_clear() -> None:
    play = Play()

    # Look for a seed that starts with the square block, so the shape that
    # lands in the gap left below is known
    seed = 1
    while seed < 1000 and play.current_block.type != 3:
        play.new_game(seed)
        seed += 1

    if play.current_block.type != 3:
        check("line clear", False, "no square block seed found")
        return

    board = play.board
    x_size = board.x_size
    y_size = board.y_size

    # Fill the bottom two rows except the two leftmost columns
    for y in range(y_size - 2, y_size):
        for x in range(2, x_size):
            board.set(x, y, 1)

    before = play.score.get()

    while play.move_left():
        pass
    play.hard_drop()

    gained = play.score.get() - before
    detail = f"{play.lines} lines, +{gained}"

    check("line clear", play.lines == 2, detail)

    # Two lines at level 1 is 300, plus 2 per cell of the hard drop
    check("line score", gained >= 300, detail)

    empty = all(board.get(x, y_size - 1) == 0 for x in range(x_size))
    check("rows removed", empty, "bottom row cleared")


def test_speed() -> None:
    play = Play()
    play.new_game(7)
    check("fall speed", play.level == 1 and play.fall_interval == 800,
          f"level {play.level} -> {play.fall_interval} ms")


def test_restart() -> None:
    play = Play()
    play.new_game(3)

    guard = 0
    while not play.is_game_over and guard < 100000:
        play.hard_drop()
        guard += 1

    high = play.high_score.get()
    play.new_game(4)

    reset = play.score.get() == 0 and play.lines == 0 and play.is_playing
    kept = play.high_score.get() == high and high > 0

    board = play.board
    empty = all(board.get(x, y) == 0
                for y in range(board.y_size) for x in range(board.x_size))

    check("restart", reset and empty, f"high {high} kept")
    check("high score", kept, f"high {high} kept")


def test_wall_kick() -> None:
    play = Play()

    # The bar is the shape that needs the kick the most
    seed = 1
    while seed < 1000 and play.current_block.type != 7:
        play.new_game(seed)
        seed += 1

    if play.current_block.type != 7:
        check("wall kick", False, "no bar block seed found")
        return

    # Stand the bar up, then push it against the right wall and turn it flat
    play.rotate()
    while play.move_right():
        pass

    check("wall kick", play.rotate(), "bar rotates at the wall")


def scenario_a(play: Play) -> None:
    """A fixed move script from an empty board."""
    drop = 0
    while not play.is_game_over and drop < 100000:
        for _ in range(drop % 4):
            play.rotate()

        shift = drop % 11
        if shift < 5:
            for _ in range(5 - shift):
                play.move_left()
        else:
            for _ in range(shift - 5):
                play.move_right()

        if drop % 7 == 0:
            play.soft_drop()

        play.hard_drop()
        drop += 1


def scenario_b(play: Play) -> None:
    """Dropping into a gap so lines clear."""
    # Leave a four wide gap at the right, so any shape pushed against the
    # wall drops into it and the rows fill up
    board = play.board
    for y in range(14, board.y_size):
        for x in range(board.x_size - 4):
            board.set(x, y, 1)

    drop = 0
    while not play.is_game_over and drop < 100000:
        while play.move_right():
            pass
        play.hard_drop()
        drop += 1


# scenario, seed, score, lines, level
REFERENCE = (
    ("A",     1, 176, 0, 1),
    ("A",     7, 263, 0, 1),
    ("A",    42, 267, 0, 1),
    ("A",   999, 174, 0, 1),
    ("A", 12345, 323, 0, 1),
    ("A",  2024, 353, 0, 1),
    ("A", 65535, 156, 0, 1),
    ("B",     1, 402, 1, 1),
    ("B",     7, 200, 0, 1),
    ("B",    42, 186, 0, 1),
    ("B",   999, 324, 1, 1),
    ("B", 12345, 362, 1, 1),
    ("B",  2024, 212, 0, 1),
    ("B", 65535, 198, 0, 1),
)


def test_reference() -> None:
    mismatch = 0

    for scenario, seed, score, lines, level in REFERENCE:
        play = Play()
        play.new_game(seed)

        if scenario == "A":
            scenario_a(play)
        else:
            scenario_b(play)

        if play.score.get() != score or play.lines != lines or play.level != level:
            print(f"    {scenario} seed {seed}: "
                  f"got {play.score.get()}/{play.lines}/{play.level}, "
                  f"want {score}/{lines}/{level}")
            mismatch += 1

    check("reference", mismatch == 0, f"{len(REFERENCE)} cases")


def main() -> int:
    print("stetris logic tests (python)\n")

    test_determinism()
    test_idle_score()
    test_pause()
    test_game_over()
    test_line_clear()
    test_speed()
    test_restart()
    test_wall_kick()
    test_reference()

    print("\n" + ("FAILED" if _failures else "all passed"))
    return 1 if _failures else 0


if __name__ == "__main__":
    sys.exit(main())
