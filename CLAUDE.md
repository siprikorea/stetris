# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

One Tetris game implemented four times: `cpp/` (the original, the reference the others follow), `java/`, `python/`, `javascript/`. Each port is a UI-free logic library (`<lang>/stetris/`) plus a terminal front end (`<lang>/app/console/`). JavaScript also has a browser front end (`javascript/app/web/`). No external dependencies in any language — standard library only, no package install, no build step for JS.

## Commands

Run from inside a language folder (`cpp`, `java`, `python`, `javascript`):

```sh
make test       # build (if needed) and run the logic tests
make console    # build/run the terminal app (C++: builds app/console/stetris, then run it)
make clean      # cpp, java, python
```

JavaScript only: `make web` serves the repo read-only on port 8080 (override with `PORT=`) since ES modules don't load over `file://`; `make dist` builds a self-contained deployable copy into `javascript/app/web/dist/`.

Each language has a single test program (`logictest` / `LogicTest.java`) with no per-test filter; to run one check, call it alone from the test's `main`. `res/make_icon.py` regenerates the icon files.

## Architecture rules that span files

- **Logic/UI separation is the point of the repo.** Nothing under `stetris/` may read a keyboard, clock, or file, use a global RNG, or touch a Node/terminal/window API. Front ends call `tick(elapsedMillis)`, supply the seed, and load/save the high score (`~/.stetris_highscore`, shared by all four console apps; the web app uses `localStorage`). The tests link `stetris/` alone — a test needing anything from `app/` means the separation broke. `javascript/app/web/` imports the exact same `stetris/*.js` files as the console app, so those must stay browser-safe.
- **The four ports must stay rule-identical.** The public API is the same name-for-name, spelled per language (`GetCell` / `getCell` / `get_cell`), and constants match (`Board.X_SIZE`, `Blocks.COUNT`, etc.). Each port's README has a "Differences from the C++" table listing the only allowed divergences.
- **Determinism is cross-port.** Every port uses its own copy of the same 32-bit LCG (`cpp/stetris/strandom.cpp`: `state * 1664525 + 1013904223`, return `state >> 16`, seed 0 → 1), never the language's built-in random. Ports must emulate C++ `unsigned int` wraparound (JS: `Math.imul` and `>>> 0`; Python: mask to 32 bits).
- **Shared reference table.** All four test suites run the same two scripted scenarios (A and B) over the same seven seeds and assert the same score/lines/level table (`g_Reference` in `cpp/test/logictest.cpp` and its counterparts). Any rule change must be made in all four ports and the table updated identically in all four; run `make test` in every language folder before considering a rules change done.
- Block type indices (1–7: J L O S T Z I, in `g_StBlocks` order in `cpp/stetris/stblocks.cpp`) also index the tiles in `res/stetris_block.bmp`.

## Conventions

- C++ uses Hungarian-style naming (`CStPlay`, `m_dwState`, `nBoardX`, `pszName`), `ST_`-prefixed enums, C++11, and `/*** @brief ... ***/` doc blocks on every function. Constants are grouped in classes/namespaces (`CStBoard::X_SIZE`, `StBlocks::COUNT`) rather than loose macros, to read like the ports.
- Every language folder keeps the same shape and the same Makefile targets (`test`, `console`, plus extras where needed).
