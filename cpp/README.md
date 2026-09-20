# stetris - C++

The original implementation, and the one the other ports follow.

`stetris/` is the game and nothing else. It has no reference to any UI, no
terminal or window code, no global `rand()`, and no file or clock access.

```sh
make test       # build and run the logic tests
make console    # build the terminal UI
console/stetris # play
```

## Use

```cpp
CStPlay play;
play.NewGame(dwSeed);

while (!play.IsGameOver())
{
    play.MoveLeft();                // input
    play.Tick(dwElapsedMilliSec);   // the game decides when a block falls
    Render(play);                   // read the state back
}
```

## Layout

```
stetris/    the logic
test/       the logic tests, which link stetris/ alone
console/    the terminal UI, the only front end
```

`console/` is the one part that talks to the outside world: it reads keys,
measures elapsed time to hand to `Tick()`, seeds a new game and stores the
high score in a file. None of that lives in `stetris/`.
