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

## Controls

| Key | Action |
|---|---|
| **Arrow Keys / WASD** | Steer Pac-Man (buffered turns at tile centers) |
| **P** | Pause / Resume |
| **M** | Mute / Unmute audio |
| **R** | Restart game (on Game Over) |
| **ESC** | Quit game |

## Build & Run

```sh
/opt/anaconda3/bin/qmake pacman.pro && make
./pacman
```
