# Theories

Theory notes and game-logic docs for the Pac-Man draft. Pollute this folder
with new `.md` files as ideas evolve — one theory per file, keep them short.

## Theory files

- [01-grid-and-maze.md](01-grid-and-maze.md) — 5px grid, wall/dot symbols, maze layout
- [02-movement.md](02-movement.md) — tile-centered movement model, turning, tunnel wrap
- [03-ghost-ai.md](03-ghost-ai.md) — ghost states, targeting, scatter/chase, frightened mode
- [04-levels-and-difficulty.md](04-levels-and-difficulty.md) — what changes per level (speed + complexity)
- [05-scoring-and-lives.md](05-scoring-and-lives.md) — points, lives, extra life
- [06-rendering.md](06-rendering.md) — pixelated rendering, scale factor, colors

## Game logic

- [game-logic/core-loop.md](game-logic/core-loop.md) — per-frame update order & state machine
- [game-logic/collision-and-rules.md](game-logic/collision-and-rules.md) — eating, death, level clear rules

## Code map

| Logic | File |
|---|---|
| Grid constants, level tuning | `src/config.h` |
| Movement model | `src/entity.h` |
| Maze layout, dots, walkability | `src/maze.cpp` |
| Ghost state + AI decision | `src/ghost.h`, `src/game.cpp` (`ghostDecide`) |
| Game loop, states, scoring | `src/game.cpp` |
| Rendering | `src/game.cpp` (`drawScene`) |
