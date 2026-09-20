# stetris

Tetris, implemented four times over.

The point of the repository is the separation: the game rules are a plain
library that knows nothing about how it is shown. No UI, no terminal or
window code, no global random generator, no file or clock access. A front
end drives it and reads its state back.

There is one such library per language - C++, Java, Python and JavaScript -
and they all play the same game. Each one comes with a terminal app built
on top of it, and the four apps look and behave the same, down to sharing
one high score file.

## Layout

Every language folder has the same shape:

```
<lang>/Makefile      make test
<lang>/README.md     how to use it, and how it differs from the C++
<lang>/stetris/      the logic
<lang>/test/         the tests, which link the logic alone
<lang>/app/console/  the terminal front end
```

The test is `logictest` in every language, except in Java where the file
has to be `LogicTest.java` to match the public class.

```
cpp/                the original
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

The constants match too: `Board.X_SIZE` and `Board.Y_SIZE` for the well,
`Blocks.COUNT`, `Blocks.ROTATIONS` and `Blocks.SIZE` for the shapes.

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

Any of the four, they play the same:

```sh
cd cpp        && make console
cd java       && make console
cd python     && make console
cd javascript && make console
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

The high score is kept in `~/.stetris_highscore`, shared by all four apps.
Loading and saving it is the app's job; the game only holds the value.

`app/` is where a second front end would go - a Swing window, a browser
page, a curses UI - without the logic changing.
