# Theory 03 — Ghost AI

## State machine (`Ghost::St`)

```
House ──houseT expires──▶ Exiting ──reaches row 11──▶ Hunting ⇄ Frightened
  ▲                                                             │ eaten
  └──────────1.0s regroup──────────── Eyes ◀────────────────────┘
              (BFS home to 13,14)
```

| State | Movement | Door/interior | Speed |
|---|---|---|---|
| House | frozen (release timer) | — | — |
| Exiting | scripted: align col 13/14 → go Up | passable | normal |
| Hunting | greedy Euclidean toward target | blocked | `ghostSpeed` |
| Frightened | random choice at centers | blocked | `0.55 ×` |
| Eyes | BFS shortest path to (13,14) | passable | `2.2 ×` |

## Decision rule (Hunting)

At each tile center, among the 4 neighbours:

1. drop **reverse** direction unless it is the only legal move (classic
   "ghosts never backtrack" rule),
2. keep only tiles `ghostPass()` accepts,
3. pick the tile minimizing **squared Euclidean distance to the target tile**,
4. tie-break in fixed order **Up → Left → Down → Right** (deterministic, the
   same ordering the arcade uses).

Targets (`ghostTarget`, Manhattan-style aiming):

| Ghost | Chase target | Scatter target |
|---|---|---|
| Blinky (0) | Pac-Man tile + predictive | (25, 0) |
| Pinky (1) | +4 tiles ahead of Pac-Man | (2, 0) |
| Inky (2) | 2×(Pac+2 ahead) − Blinky | (27, 30) |
| Clyde (3) | Pac-Man if >8 tiles away, else scatter corner | (0, 30) |

`predictive` = tiles of aim-ahead granted by level (theory 04).

## Modes

- Schedule (`kModes`): scatter 7s → chase 20s → scatter 7s → chase 20s →
  scatter 5s → chase forever.
- The mode timer **pauses while frightened mode is active**.
- On every switch, all Hunting ghosts do a mid-corridor 180° flip.

## Frightened mode

- Triggered by eating a power pellet (`triggerFright()`), duration from level
  tuning (6s → 2s across levels).
- Hunting ghosts flip and turn blue; random direction at each center,
  still never reversing.
- Flashes white in the last 2 seconds.
- House/Exiting ghosts are untouched: Exiting ones simply come out
  already-frightened (decided when they reach row 11).

## Eyes

Eaten ghost runs a **BFS distance field** from (13,14) over door+interior
tiles and steps to the strictly closer neighbour (reverse only as fallback).
On arrival it waits 1.0s in the House, then exits again.

## Known simplifications (future theories)

- No tunnel slowdown, no "cruise-elroy" Blinky speed-up, no target tile
  clamping, random frightened choice is per-center uniform.
