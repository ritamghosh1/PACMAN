# Game Logic — Collision & Rules

## Eating

- Dot/pellet consumed the moment Pac-Man's **current tile** contains one
  (`maze.eat(pac.tr, pac.tc)` runs every tick after his move).
- Grid char is rewritten to `' '` and the point removed from the list;
  `maze.empty()` (no dots **and** no pellets) = level cleared.

## Pac-Man ↔ Ghost

```
ddx = g.fx - pac.fx ; ddy = g.fy - pac.fy
collide if ddx² + ddy² < 11   (≈ 3.3px = 0.66 tile)
```

| Ghost state on contact | Result |
|---|---|
| Frightened | ghost → Eyes, Pac +200·2^chain, chain++ |
| Hunting / Exiting | Pac-Man → Dying |
| House / Eyes | no interaction (Pac-Man can't reach them anyway) |

Collision is checked **after both sides moved**, so simultaneous frames resolve
in ghosts' favour only when actually overlapping — never on adjacent tiles.

## Death

1. `Dying`, 1.5s, ghosts hidden, mouth animates open.
2. `lives--`; if 0 → `Over`; else `resetActors()` (positions, house timers,
   mode schedule, fright all reset — **dots stay eaten**) → `Ready` 2s.

## Level clear

1. `Won`, 1.5s, maze flashes, actors frozen.
2. `level++` → `applyTuning()` (new speeds/fright/release/predictive) →
   `maze.reset()` (all dots back) → `resetActors()` → `Ready` 2s.

## Ghost release order (per spawn/level/death)

| Ghost | Spawn col | Delay |
|---|---|---|
| Blinky | 13 | 0 × releaseBase/1.6 |
| Pinky | 14 | 1 × |
| Inky | 12 | 3 × |
| Clyde | 15 | 5 × |

`releaseBase` shrinks per level (theory 04) → whole squad streams out faster.

## Door rules

- Pac-Man: door and house interior are **never** walkable
  (`maze.walkablePac`).
- Ghosts: door/interior walkable **only** in `Exiting`/`Eyes` (and `House`),
  so a Hunting ghost can never slip back into its own house.
