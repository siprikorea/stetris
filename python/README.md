# stetris - Python

A port of the game logic in `cpp/stetris/`. Logic only: no UI, and nothing here
reads a keyboard, a clock or a file. Standard library only, no packages to
install.

```sh
make test
```

or

```sh
python3 test/logictest.py
```

## Use

```python
from stetris import Play

play = Play()
play.new_game(seed)

while not play.is_game_over:
    play.move_left()            # input
    play.tick(elapsed_millis)   # the game decides when a block falls
    render(play)                # read the state back
```

## Differences from the C++

The rules are identical, the public API is the same name for name, and the
tests check that both stay that way. What changes is spelling and the
things the language does differently:

| C++ | Python |
| --- | --- |
| `CStPlay`, `CStBoard`, … | `Play`, `Board`, … in package `stetris` |
| `GetCell`, `MoveLeft`, `NewGame` | `get_cell`, `move_left`, `new_game` |
| `GetScore()`, `IsGameOver()` | `score`, `is_game_over` properties |
| `CStRandom` | `Rng`, not the `random` module |
| `ST_STATE::PLAYING` | `State.PLAYING` |
| `unsigned int` score | unbounded `int` |

`Rng` is a plain linear congruential generator rather than `random`, because
a seed has to produce the same sequence here as it does in the other ports.
Its state is masked to 32 bits so it matches the C++ `unsigned int`.
