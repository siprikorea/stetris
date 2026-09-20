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

## Play

Two front ends, both on the same logic.

```sh
make console    # terminal
make web        # browser, then open the printed URL
```

`make web` starts a small static server from the standard library, because
ES modules do not load over `file://`. It serves the repository read only
and takes `PORT` if 8080 is busy.

| key | action |
| --- | --- |
| Left / Right (or A / D) | move |
| Up (or W) | rotate |
| Down (or S) | soft drop |
| Space | hard drop |
| P | pause |
| R | restart |
| Q | quit |

The console app keeps the high score in `~/.stetris_highscore`, shared with
the apps in the other languages. The web app has no home directory to write
to, so it uses `localStorage` and keeps its own.

## Use

```js
import { Play } from './stetris/index.js';

const play = new Play();
play.newGame(seed);

play.moveLeft();            // input
play.tick(elapsedMillis);   // the game decides when a block falls
render(play);               // read the state back
```

Nothing in `stetris/` touches a Node API, which is why `app/web/` can
import the very same files:

```html
<script type="module">
  import { Play } from '../../stetris/index.js';
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
