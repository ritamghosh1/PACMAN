# Theory 06 — Pixelated Rendering

README: *"pixelated, should use grid (5px as 1 grid block)"*.

## Architecture: Low-Res Offscreen Framebuffer

To guarantee a 100% authentic retro arcade look without sub-pixel vector rendering:

1. **Offscreen Framebuffer (`QImage`)**:
   Render the entire scene into a low-resolution `140 × 171` integer-pixel `QImage` (`cfg::kW` × `cfg::kH + cfg::kHud`).
2. **Nearest-Neighbor Blit**:
   In `paintEvent`, the 140×171 image is blitted onto the window via `p.drawImage(rect(), buffer_)` with `SmoothPixmapTransform` disabled. Every logical pixel becomes a solid, razor-sharp 4×4 device pixel block.
3. **No Vector Arcs**:
   Every sprite (Pac-Man, Ghosts, Font, Fruits) is rendered using discrete pixel matrices rather than continuous floating-point `QPainter::drawEllipse` or trigonometric path arcs.

## Handcrafted Pixel Elements

| Element | Pixel Specification |
|---|---|
| Wall tile | 5×5 solid block: outer blue `#2121DE`, inner `#0A0A50`, center `#2121DE` |
| Dot | 2×2 square pixel `#FFB8AE` at tile center |
| Power Pellet | 4×4 pixel flashing diamond at tile center |
| Pac-Man | 7×7 discrete pixel matrix with 3 animation mouth frames + directional rotation |
| Dying Animation | 11-step progressive pixel slice dissolution + spark pop |
| Ghost Body | 7×7 pixel dome top with 2-frame walking wave skirt animation |
| Ghost Eyes | 2×3 pixel white sclera with direction-steered blue pupils |
| Frightened Ghost | Retro arcade blue body `#2121DE`, scared white eyes & wavy mouth; flashes white/blue during last 2 seconds |
| Eaten Eyes | Eyeballs-only returning home via BFS pathfinding |
| Bonus Fruit | Handcrafted 7×7 Cherry with green stems, red bodies, and white highlight pixels |
| HUD / Labels | 3×5 classic arcade bitmap font (1UP, HIGH SCORE, READY!, GAME OVER, PAUSED) |
