# Theory: Potential Game Logic Upgrades

This document outlines high-impact game logic enhancements that can be implemented to bring the game to authentic tournament-grade arcade fidelity and deepen strategic gameplay.

---

## 1. Ghost Tunnel Slowdown ("Tunnel Trap")

### Arcade Mechanic
In the original 1980 arcade game, the side warp tunnel (Row 14) contains a speed governor specifically for ghosts:
- **Pac-Man in Tunnel**: Moves at **100% normal speed**.
- **Hunting/Frightened Ghosts in Tunnel**: Speed is cut to **40% – 50%**.
- **Eaten Eyes in Tunnel**: Move at full speed (no penalty).

### Strategic Value
This gives the player an active tactical escape mechanism. If ghosts are hot on Pac-Man's tail, ducking into the side tunnel forces the ghosts into a crawl, allowing Pac-Man to gain valuable separation.

```cpp
// Logic sketch in Game::updatePlaying
bool inTunnel = (g.tr == cfg::kTunnelRow && (g.tc <= 5 || g.tc >= cfg::kCols - 6));
if (inTunnel && g.st != Ghost::St::Eyes) {
    g.speed *= 0.50; // Tunnel penalty
}
```

---

## 2. Cornering Speed Advantage ("Pre-turn Cutting")

### Arcade Mechanic
In the arcade hardware:
- **Ghosts** can only decide and execute a 90° turn when their coordinate exactly aligns with the center pixel of a tile intersection.
- **Pac-Man** can buffer a turn up to 3–4 pixels *before* reaching the tile center, allowing him to cut the inner corner obliquely.

### Strategic Value
Because Pac-Man travels a slightly shorter distance on every turn, skilled players who chain consecutive turns can outrun ghosts even when a ghost (such as Cruise Elroy Blinky) has a higher raw straightaway speed.

---

## 3. Dot-Eating Hesitation (1-Frame Chomp Delay)

### Arcade Mechanic
- When Pac-Man traverses an **empty corridor**, he travels at full speed without pause.
- When Pac-Man **eats a regular dot**, his movement is frozen for **1 frame (16.6 ms)**.
- When Pac-Man **eats an Energizer (Power Pellet)**, his movement is frozen for **3 frames (50 ms)**.

### Strategic Value
This introduces crucial risk/reward routing: fleeing through an untouched corridor full of dots slows Pac-Man down by ~10–15%, allowing pursuing ghosts to close the distance. Clean, cleared corridors become high-speed escape lanes.

---

## 4. Forbidden Ghost Turns ("One-Way Intersections")

### Arcade Mechanic
In the original arcade maze, there are 4 specific intersections where ghosts are **prohibited from turning UP**:
1. The two tiles directly above the ghost house (Row 11, Cols 12 and 15).
2. The two T-junctions near the top corners.

At these tiles, ghosts can only travel Left, Right, or Down—never Up.

### Strategic Value
This guarantees that the corridor directly above the ghost house is safe from vertical ambushes, giving the player predictable routing when navigating past the central house.

---

## 5. Dot Counter House Release System

### Current Logic
Ghosts exit the house purely on fixed timers (`kRelease = {0, 1.0, 3.0, 5.0}`).

### Arcade Dot Counter Logic
Ghosts release based on Pac-Man's eating progress:
- **Blinky**: Spawns outside the house immediately.
- **Pinky**: Exits immediately when the game starts.
- **Inky**: Trapped in the house until Pac-Man eats **30 dots**.
- **Clyde**: Trapped in the house until Pac-Man eats **60 dots** (1/3 of the maze).

### Life-Loss Fallback (Global Counter)
If Pac-Man dies, the game switches to a "Global Dot Counter" (7, 17, 32 dots) to release ghosts one at a time, preventing an unfair 4-ghost ambush right after respawning.

---

## 6. Ms. Pac-Man Wandering Fruit Logic

### Arcade Evolution
- **Pac-Man (1980)**: Fruit sits motionless below the ghost house for 9 seconds.
- **Ms. Pac-Man (1981)**: Fruit enters the maze from one side warp tunnel, wanders along a pseudo-random path through the corridors, and exits through the opposite tunnel if not intercepted in time.

### Strategic Value
Wandering fruit creates dynamic interception challenges instead of predictable camping near the center of the maze.

---

## 7. Fruit Value Progression Table

Instead of only Cherries, award authentic progressive bonus fruits based on the current level:

| Level | Fruit | Sprite | Points |
|---|---|---|---|
| **1** | Cherry | 🍒 | 100 pts |
| **2** | Strawberry | 🍓 | 300 pts |
| **3 – 4** | Peach / Orange | 🍊 | 500 pts |
| **5 – 6** | Apple | 🍎 | 700 pts |
| **7 – 8** | Melon | 🍈 | 1000 pts |
| **9 – 10** | Galaxian Starship | 🚀 | 2000 pts |
| **11 – 12** | Bell | 🔔 | 3000 pts |
| **13+** | Key | 🗝️ | 5000 pts |

---

## 8. Ghost House Idle Bobbing & Regeneration Loop

### Arcade Mechanic
- Ghosts inside the house bob vertically up and down in place with their eyes tracking Pac-Man.
- When an eaten ghost's eyes reach the house gate (`(13, 14)`), they do not instantly switch to hunting:
  1. The eyes descend into the house.
  2. The ghost body reforms on the house floor.
  3. The ghost transitions to `Exiting` and passes back through the pink door.
