# Theory 02 — Movement Model

## Tile-centered movement (`src/entity.h`)

Entities do **not** move in free pixels. Each entity owns:

- `tr, tc` — the tile it currently occupies
- `p` — progress toward the *next* tile center, `0 ≤ p < 1`
- `dir` — direction of travel
- `want` — buffered input direction (Pac-Man only)

Rendered position:

```
x = (tc + 0.5 + dx(dir) * p) * 5
y = (tr + 0.5 + dy(dir) * p) * 5
```

## Rules

1. **Turns only at tile centers.** `advance()` walks the entity to the next
   center, then asks `decide()` for a new direction. Mid-corridor direction
   changes are impossible — this is what makes the grid feel crisp.
2. **Walls stop you at the center of the last open tile** (never halfway into
   a wall tile). `pass()` is checked before every segment.
3. **Input buffering.** Pac-Man's `want` persists until it becomes legal at a
   center; you can hold a turn early and it fires at the next opportunity.
4. **Tunnel wrap.** On `kTunnelRow` (14), columns past the edge wrap
   `0 ⇄ 27`. Any other row treats off-grid as a wall.
5. **Reversal** (`reverseMid`) is legal mid-corridor and is used only for
   scripted flips: entering frightened mode and scatter⇄chase switches.

## Decision points

`advance(e, dt, pass, decide)` calls `decide(e)`:

- on start (when `dir == None`),
- every time the entity **arrives** at a tile center.

Pac-Man's decide = "buffered input if legal, else keep going".
Ghosts' decide = AI (see theory 03).

## Speed

Speed is in **tiles/second**, consumed as `rem = speed * dt` each tick;
whatever remains after reaching a center carries over inside the same tick
(guard: 16 segments/tick max), so motion is frame-rate independent.
