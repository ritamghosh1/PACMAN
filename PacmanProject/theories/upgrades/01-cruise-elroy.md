# Theory: Ghost Speed Scaling & "Cruise Elroy"

In standard retro arcade Pac-Man, the most legendary difficulty mechanic is **"Cruise Elroy"** (named by Midway distributors). It prevents the endgame of each level from becoming a tedious mop-up phase and instead transforms it into a heart-pounding survival climax.

## 1. Problem with Flat Ghost Speeds

Without Cruise Elroy:
- When only 5–10 isolated dots remain in opposite corners of the maze, the player can easily loop corridors and finish without threat.
- Scatter mode intervals allow Pac-Man free escapes, removing tension.

## 2. Cruise Elroy Mechanics

As the number of remaining dots (`dots + power pellets`) drops below defined thresholds, Blinky (Ghost 0, Red) triggers two distinct escalation states:

| State | Dot Threshold | Speed Multiplier | Effective Speed (L1) | Scatter Behavior |
|---|---|---|---|---|
| **Normal** | > 20 dots | 1.00× | 6.4 tiles/s | Respects scatter timer |
| **Elroy 1** | ≤ 20 dots | 1.10× | ~7.04 tiles/s | Ignores scatter (Continuous chase) |
| **Elroy 2** | ≤ 10 dots | 1.22× | ~7.80 tiles/s | Ignores scatter (Outruns Pac-Man) |

*(Note: Pac-Man's base speed on Level 1 is 7.0 tiles/sec. In Elroy 2, Blinky is strictly faster than Pac-Man on straightaways, requiring the player to utilize corners and tunnel wrap-arounds to survive).*

## 3. General Level Scaling

In addition to Elroy, overall ghost speed scales progressively with each level:
$$\text{ghostSpeed}(n) = \min(9.5,\, 6.4 + 0.45 \cdot (n - 1))$$

- **Level 1**: Ghost base speed = 6.4 tiles/s (Pac-Man = 7.0 tiles/s)
- **Level 2**: Ghost base speed = 6.85 tiles/s
- **Level 3**: Ghost base speed = 7.30 tiles/s (ghosts naturally match/outpace Pac-Man)
- **Frightened duration** decreases by 0.6s per level down to a floor of 1.5s.
- **House release delay** decreases exponentially ($1.6 \times 0.75^{n-1}$).

## 4. Implementation in Code

In `Game::updatePlaying`:
```cpp
int dotsLeft = static_cast<int>(maze_.dots().size() + maze_.pellets().size());
if (g.id == 0 && g.st == Ghost::St::Hunting) {
    if (dotsLeft <= 10) g.speed = ghostSpeed_ * 1.22;       // Elroy 2
    else if (dotsLeft <= 20) g.speed = ghostSpeed_ * 1.10;  // Elroy 1
    else g.speed = ghostSpeed_;
}
```

In `Game::ghostTarget`:
```cpp
// Blinky in Cruise Elroy ignores scatter and hunts Pac-Man directly
if (!chase_ && !(g.id == 0 && dotsLeft <= 20)) {
    return g.corner;
}
```
