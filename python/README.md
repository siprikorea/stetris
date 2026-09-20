# stetris - Python

A port of the game logic in `cpp/`. Logic only: no UI, and nothing here
reads a keyboard, a clock or a file. Standard library only, no packages to
install.

```sh
make test
```

or

```sh
python3 test_logic.py
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

The rules are identical and the tests check that they stay identical. Only
the surface changes:

| C++ | Python |
| --- | --- |
| `CStPlay`, `CStBoard`, … | `Play`, `Board`, … in package `stetris` |
| `GetScore()`, `IsGameOver()` | `score`, `is_game_over` properties |
| `m_CurrentBlock = m_NextBlock` | `current_block.copy_from(next_block)` |
| `CStRandom` | `Rng`, not the `random` module |
| `ST_STATE` constants | `enum State` |
| `GetValue` / `SetValue` | `board.get` / `board.set` |
| `GetBlock(x, y)` | `block.get(x, y)` |

`Rng` is a plain linear congruential generator rather than `random`, because
a seed has to produce the same sequence here as it does in the other ports.
Its state is masked to 32 bits so it matches the C++ `unsigned int`.
