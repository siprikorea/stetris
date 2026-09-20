# stetris - Java

A port of the game logic in `cpp/stetris/`. Logic only: no UI, and nothing here
reads a keyboard, a clock or a file.

```sh
make test
```

That compiles into `out/` and runs the tests. Plain `javac` works too:

```sh
javac -d out stetris/*.java test/LogicTest.java
java -cp out LogicTest
```

## Use

```java
Play play = new Play();
play.newGame(seed);

while (!play.isGameOver()) {
    play.moveLeft();          // input
    play.tick(elapsedMillis); // the game decides when a block falls
    render(play);             // read the state back
}
```

## Differences from the C++

The rules are identical, the public API is the same name for name, and the
tests check that both stay that way. What changes is spelling and the
things the language does differently:

| C++ | Java |
| --- | --- |
| `CStPlay`, `CStBoard`, … | `Play`, `Board`, … in package `stetris` |
| `GetCell`, `MoveLeft`, `NewGame` | `getCell`, `moveLeft`, `newGame` |
| `CStRandom` | `Rng`, not `java.util.Random` |
| `ST_STATE::PLAYING` | `State.PLAYING` |
| `unsigned int` score | `long` score |
| raw pointers from `GetBoard()` | references |

`Rng` is a plain linear congruential generator rather than
`java.util.Random`, because a seed has to produce the same sequence here as
it does in the other ports.
