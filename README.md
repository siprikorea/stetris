# stetris

Tetris, implemented four times over.

The point of the repository is the separation: the game rules are a plain
library that knows nothing about how it is shown. No UI, no terminal or
window code, no global random generator, no file or clock access. A front
end drives it and reads its state back.

There is one such library per language - C++, Java, Python and JavaScript -
and they all play the same game. C++ additionally has the one front end, a
terminal UI.

## Layout

Every language folder has the same shape:

```
<lang>/Makefile      make test
<lang>/README.md     how to use it, and how it differs from the C++
<lang>/stetris/      the logic
<lang>/test/         the tests, which link the logic alone
```

The test is `logictest` in every language, except in Java where the file
has to be `LogicTest.java` to match the public class.

```
cpp/                the original, and the only front end
cpp/console/          terminal UI (macOS / Linux / Windows)
java/
python/
javascript/
res/                icon and block images, kept for a graphical UI
```

## The logic

The public API is the same name for name in all four, with each port
spelling it the way its language would - `GetCell`, `getCell`, `get_cell`:

| | |
| --- | --- |
| `newGame(seed)` | start a game; the caller owns the seed |
| `moveLeft()` `moveRight()` `rotate()` | input |
| `softDrop()` `hardDrop()` | input |
| `tick(elapsedMillis)` | elapsed time in, the game decides when a block falls |
| `setPause()` `togglePause()` | |
| `isPlaying` `isPaused` `isGameOver` | state to read |
| `board` `currentBlock` `nextBlock` | state to draw |
| `score` `highScore` `level` `lines` | state to draw |

```js
const play = new Play();
play.newGame(seed);

play.moveLeft();            // input
play.tick(elapsedMillis);   // the game decides when a block falls
render(play);               // the front end pulls what it needs
```

The caller supplies the seed, so a given seed always replays the same game.
That is what makes the rules testable with no UI at all, and what lets the
four ports be compared against each other.

Nothing in the library reads a keyboard, a clock or a file. Deciding when to
call `tick`, where to get a seed and where to keep the high score are the
front end's problems. See each language's README for the exact spelling.

## Tests

Each test suite links its own logic alone, which is also the check that the
layers stay apart: if a test ever needs a header from a UI folder, the
separation has broken.

They cover determinism from a seed, scoring, line clears, pause, game over,
wall kicks and restart. On top of that, all four run the same two scripted
scenarios over the same seven seeds and assert the same table of score,
lines and level, so a change to the rules in one port shows up as a failure
in the others.

```sh
cd cpp        && make test
cd java       && make test
cd python     && make test
cd javascript && make test
```

## Play

Only C++ has a front end so far.

```sh
cd cpp
make console
console/stetris
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

The high score is kept in `~/.stetris_highscore`. Loading and saving it is
the front end's job; the game only holds the value.
