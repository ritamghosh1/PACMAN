# Theory: Intermission Cutscenes & Giant Pac-Man

Pac-Man (1980) was the very first arcade game in history to feature animated cutscenes ("coffee breaks" or "intermissions"). Toru Iwatani designed them to give players a momentary rest, celebrate clearing stages, and add humorous personality to the characters.

## 1. Triggering & State Flow

```
[Level Cleared: S::Won]
         │ (1.2s maze flash)
         ▼
  Level 1 or 2 cleared?
   ├── Yes ──► [S::Intermission] (5.6s cutscene + ragtime chiptune)
   │                  │
   │                  ▼ (finished or skipped with Space)
   └── No  ──► [S::Ready] (Level N+1, intro fanfare)
```

## 2. Cutscene Choreography: "Act 1 — Giant Pac-Man"

The entire cutscene runs on a dedicated cinematic stage (Y = 95, floor line at Y = 106) over 5.6 seconds:

### Act 1A: The Chase (0.0s – 2.6s)
1. Pac-Man runs leftward across the screen from $X = 150$ to $X = -30$, chomping furiously.
2. Blinky (Red Ghost) pursues 24 pixels behind Pac-Man with his red skirt wiggling at 8 Hz.
3. Both exit off the left side of the screen.

### Act 1B: The Turnabout (2.6s – 5.6s)
1. A moment after exiting, a frightened, blue Blinky scrambles back across the screen from left ($X = -20$) to right ($X = 170$), eyes terrified.
2. Looming right behind him is **GIANT PAC-MAN** ($21 \times 21$ pixels, scaled $3\times$ from standard $7 \times 7$), chomping with massive jaws, chasing the terrified ghost!

## 3. Pixel Sprite Scaling

Rather than filtering or interpolating, Giant Pac-Man is rendered using exact integer nearest-neighbor block magnification ($pixelSize = 3$):
- Standard sprite: $7 \times 7$ pixels
- Giant sprite: each bit in the matrix occupies a $3 \times 3$ solid pixel square
- Total footprint: $21 \times 21$ logical pixels (84 device pixels at 4× window zoom)

```
......#########......
......#########......
...###############...
#########............
#########............
######...............
#########............
...###############...
......#########......
```

## 4. Audio & Controls

- Plays `pacman_intermission.wav`, an authentic 8-bit ragtime chiptune arpeggio track (5.2s).
- Player can press **SPACE** or **ENTER** to skip the cutscene immediately and proceed to the next stage.
