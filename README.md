# stetris c++

Tetris written in C++.

`cpp/` is the game and nothing else. It has no reference to any UI, no
terminal or window code, no global `rand()`, and no file or clock access.
A UI drives it and reads its state back:

```cpp
CStPlay play;
play.NewGame(dwSeed);

while (!play.IsGameOver())
{
    play.MoveLeft();            // input
    play.Tick(dwElapsedMilliSec);   // the game decides when a block falls
    Render(play);               // the UI pulls what it needs
}
```

Because the seed is supplied by the caller, a given seed always replays the
same game, so the logic can be tested with no UI at all.

```
cpp/                game logic in C++ (board, block, play, score, random)
build/console/      terminal UI (macOS / Linux / Windows)
test/               logic tests, no UI linked
java/               the same logic in Java
python/             the same logic in Python
res/                icon and block images, kept for a graphical UI
```

## Ports

`java/` and `python/` are the logic only, no UI. All three implementations
follow the same rules, and each test suite checks the same table of seeds
against the same expected scores, so a change to the rules in one place
shows up as a failure in the others.

```sh
cd test   && make test   # C++
cd java   && make test   # Java
cd python && make test   # Python
```

## Build and run

```sh
cd build/console
make
./stetris
```

| key | action |
| --- | --- |
| Left / Right (or A / D) | move |
| Up (or W) | rotate |
| Down (or S) | soft drop |
| Space | hard drop |
| P | pause |
| R | restart |
| Q | quit |

The high score is kept in `~/.stetris_highscore`. Loading and saving it is the
UI's job - the game only holds the value.

## Tests

The tests link the logic layer only, with no UI, and check the game rules:
determinism from a seed, scoring, line clears, pause, game over, wall kicks
and restart.

```sh
cd test
make test
```
