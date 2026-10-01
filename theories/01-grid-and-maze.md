# Theory 01 — Grid and Maze

## Grid

- **1 grid block = 5px** (README requirement). All game logic is expressed in
  tiles; pixels only appear at render time.
- Maze is **28 × 31 tiles** = 140 × 155 logical px.
- The window is drawn at `kScale = 4` (560 × 684) so the 5px grid is
  actually playable. The logical grid never changes — only the zoom does.
- Walls are **square, exactly 1 grid block** (README requirement), rendered as
  a filled block with a darker inner fill (blue frame look).

## Tile symbols (`src/maze.cpp`)

| Char | Meaning | Pac-Man | Ghost |
|---|---|---|---|
| `#` | wall | blocked | blocked |
| `.` | dot (10 pts) | walkable, eaten | walkable |
| `o` | power pellet (50 pts) | walkable, eaten | walkable |
| ` ` | empty floor | walkable | walkable (outside) |
| `-` | ghost-house door | **blocked** | passable only when Exiting/Eyes |
| house interior (rows 13–15, cols 11–16) | ghost house | **blocked** | passable only when Exiting/Eyes/House |

## Maze layout (28 × 31)

- Border fully closed except **row 14 (tunnel row)**: cols 0–9 and 18–27 are
  open floor; walking off one edge wraps to the other.
- Ghost house: rows 13–15, cols 11–16; door at row 12, cols 13–14.
- Pac-Man spawn: **(13, 23)**, facing left.
- Ghost spawns (house row 14): Blinky 13, Pinky 14, Inky 12, Clyde 15.
- Power pellets: (1,3), (26,3), (1,23), (26,23) — classic corner positions.
- Two vertical side corridors (col 6 / col 21) + horizontal strips above and
  below the ghost house (rows 11 / 17) keep the maze fully connected.

## Open questions / future theories

- Multiple maze layouts per level? (complexity lever for later levels)
- Tunnel slowdown for ghosts (classic behavior, not yet implemented).
- Fruit bonus spawns (not implemented).
