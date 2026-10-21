# Game Logic — Core Loop

## Tick

`QTimer` at **16 ms** (`cfg::kTickMs`) → `Game::tick()`:

```
dt = clock.restart()/1000   (clamped to 0.05s — tab-switch protection)
animT += dt
if paused → repaint only
switch (state):
  Ready    : stateT -= dt  → Playing at 0
  Playing  : updatePlaying(dt)
  Dying    : stateT -= dt  → death settled at 0
  Won      : stateT -= dt  → next level at 0
  Over     : idle (R restarts)
repaint
```

## updatePlaying(dt) — exact order

1. **Fright timer** — decrement; on expiry convert Frightened → Hunting,
   reset chain. Otherwise decrement **mode timer**; on expiry flip scatter/chase
   and reverse all Hunting ghosts.
2. **Pac-Man** `advance()` with pacDecide (buffered input).
3. **Eat** — `maze.eat(tr, tc)`; pellet (50) triggers `triggerFright()`.
4. **Ghosts** — House: count release timer (no movement). Others: set speed by
   state, `advance()` with ghostDecide.
5. **Collisions** — squared distance < 11 px²:
   - frightened ghost → eaten (score, become Eyes),
   - otherwise → `Dying` (1.5s), stop the update.
6. **Level clear** — `maze.empty()` → `Won` (1.5s flash).

## State machine

```
newGame → Ready(2s) → Playing ⇄ [Frightened overlay]
                       │
                       ├─ death → Dying(1.5s) ─ lives−1 ─┬─ >0 → Ready(2s)
                       │                                 └─ 0 → Over (R → newGame)
                       └─ maze empty → Won(1.5s) → level+1, retune, reset maze → Ready(2s)
```

## Input (keyPressEvent)

| Key | Action |
|---|---|
| Arrows / WASD | set `pac.want` (buffered turn) |
| P | pause toggle (Ready/Playing) |
| R | restart after GAME OVER |
| Esc | quit |

`dt`-based timers mean all of the above is frame-rate independent; the 16ms
tick only decides how often decisions get *asked*.
