# stetris - JavaScript

A port of the game logic in `cpp/stetris/`. Logic only: no UI, and nothing here
reads a keyboard, a clock or a file. Standard ES modules, no dependencies,
no build step.

```sh
make test
```

or

```sh
node test/logictest.js
```

## Use

```js
import { Play } from './stetris/index.js';

const play = new Play();
play.newGame(seed);

play.moveLeft();            // input
play.tick(elapsedMillis);   // the game decides when a block falls
render(play);               // read the state back
```

It runs unchanged in a browser, since nothing here touches Node APIs:

```html
<script type="module">
  import { Play } from './stetris/index.js';
</script>
```

## Differences from the C++

The rules are identical, the public API is the same name for name, and the
tests check that both stay that way. What changes is spelling and the
things the language does differently:

| C++ | JavaScript |
| --- | --- |
| `CStPlay`, `CStBoard`, … | `Play`, `Board`, … |
| `GetCell`, `MoveLeft`, `NewGame` | `getCell`, `moveLeft`, `newGame` |
| `GetScore()`, `IsGameOver()` | `score`, `isGameOver` getters |
| `CStRandom` | `Rng`, not `Math.random` |
| `ST_STATE::PLAYING` | frozen `State.PLAYING` |
| `protected` members | `#private` class fields |

`Rng` is a plain linear congruential generator rather than `Math.random`,
because a seed has to produce the same sequence here as it does in the
other ports. It uses `Math.imul`, since a plain `*` would lose precision
once the product passes 2^53, and `>>> 0` to match the C++ `unsigned int`.
