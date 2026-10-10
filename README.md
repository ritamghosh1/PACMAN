# PACMAN GAME

Pixelated retro arcade game built with the Qt framework (5px grid blocks, square walls, multi-level scaling).

## Features

- **True 8-Bit Pixel-Art Rendering**: Rendered onto an off-screen low-resolution framebuffer (140×171 logical pixels) and scaled using nearest-neighbor (4× zoom) so every pixel is a crisp, chunky device block without sub-pixel blurring.
- **Handcrafted Pixel Sprites**:
  - Animated Pac-Man (closed, half-open, wide-open mouth cycles, and classic death dissolve animation).
  - 4 Ghosts with animated 2-frame walking wave skirts, direction-aware eyeballs & pupils, frightened blue/white flashing, and eaten eyes.
  - Classic 3×5 retro arcade bitmap font for HUD (1UP, HIGH SCORE, READY!, GAME OVER, PAUSED).
  - Bonus Cherries and floating score point popups (+100, +200, +400, +800, +1600).
- **Classic Arcade Audio**:
  - Iconic Pac-Man opening theme fanfare during READY! countdown (Toshio Kai 1980).
  - Rhythmic "waka-waka" chomp sound effects when eating dots.
  - Frightened ghost eat chime, death fall & pop sound, and bonus fruit eat fanfare.
  - Mute/Unmute audio toggle with `M` key.
- **Core Game Logic**:
  - 28×31 maze on a 5px logical grid.
  - Square 1-block walls, dots, power pellets, side tunnel wrap-around, ghost house with door.
  - Tile-centered movement with buffered turns (Arrows / WASD).
  - 4 Ghost AI personalities (Blinky, Pinky, Inky, Clyde) with scatter/chase modes, frightened mode, and eaten-eyes BFS pathfinding.
  - Difficulty scaling: speed, frightened duration, house release time, and predictive targeting scale per level.
  - High Score persistence saved across games using `QSettings`.
- **Level Variety**:
  - A new maze colour scheme every level (blue, pink, orange, cyan, green, purple).
  - Three maze layouts (Classic, Grid, Pillars), switching every two levels. All share the same ghost house, tunnel and spawn points.
- **Two-Player Modes**: local Co-op and Versus (see Game Modes below), chosen from a title menu.

## Game Modes

Pick a mode on the title screen (Up/Down or 1/2/3, then Enter):

| Mode | How it plays |
|---|---|
| **1 Player** | Classic Pac-Man. Arrows or WASD. |
| **2P Co-op** | Two Pac-Men share one maze. P1 (yellow) uses Arrows, P2 (green) uses WASD. Separate scores and lives; ghosts chase whoever is closest. The game ends when both players are out of lives. |
| **2P Versus** | P1 is Pac-Man (Arrows). P2 controls the red ghost Blinky (WASD); the other three ghosts stay AI teammates. Pac-Man wins by clearing the maze, the Ghost wins by taking all 3 lives. The ghost player scores 1000 per personal catch and 300 per AI-assisted catch. |

## Controls

| Key | Action |
|---|---|
| **Arrow Keys** | Steer Player 1's Pac-Man (buffered turns at tile centers) |
| **WASD** | Single: Player 1 · Co-op: Player 2's Pac-Man · Versus: the red ghost |
| **P** | Pause / Resume |
| **M** | Mute / Unmute audio |
| **R** | Play again in the same mode (on Game Over) |
| **ESC** | Back to the mode menu (quits from the menu) |

## Build & Run

Open `pacman.pro` in Qt Creator and press Run, or from a terminal:

```sh
qmake pacman.pro && make
./pacman
```

Sound files in `sounds/` are embedded into the executable through `resources.qrc`.
