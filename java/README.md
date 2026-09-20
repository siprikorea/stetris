# stetris - Java

A port of the game logic in `src/`. Logic only: no UI, and nothing here
reads a keyboard, a clock or a file.

```sh
make test
```

That compiles into `out/` and runs the tests. Plain `javac` works too:

```sh
javac -d out stetris/*.java LogicTest.java
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

The rules are identical and the tests check that they stay identical. Only
the surface changes:

| C++ | Java |
| --- | --- |
| `CStPlay`, `CStBoard`, … | `Play`, `Board`, … in package `stetris` |
| `m_CurrentBlock = m_NextBlock` | `currentBlock.copyFrom(nextBlock)` |
| `CStRandom` | `Rng`, not `java.util.Random` |
| `ST_STATE` constants | `enum State` |
| `unsigned int` score | `long` score |
| `GetBlock(x, y)` | `getCell(x, y)` |

`Rng` is a plain linear congruential generator rather than
`java.util.Random`, because a seed has to produce the same sequence here as
it does in the other ports.
