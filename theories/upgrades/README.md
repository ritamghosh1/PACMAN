# Upgrades & Advanced Mechanics

This folder documents the enhancements and arcade-faithful upgrades introduced to elevate the Pac-Man project beyond a basic grid clone into an authentic arcade experience.

## Upgrade Index

1. **[01-cruise-elroy.md](01-cruise-elroy.md)** — Dynamic Ghost Speedup & Cruise Elroy
   - The original 1980 Toru Iwatani arcade mechanic where Blinky accelerates as dots are eaten.
   - Stage 1 (<= 20 dots) and Stage 2 (<= 10 dots) acceleration thresholds.
   - Scatter-mode override and endgame tension curve.

2. **[02-intermission-cutscene.md](02-intermission-cutscene.md)** — Intermission Cinema & Giant Pac-Man
   - Arcade cutscene state machine (`S::Intermission`).
   - Two-act choreography: Leftward chase followed by Giant Pac-Man (3× scale / 21×21 px) chasing frightened Blinky.
   - Synchronization with the classic ragtime chiptune audio track and skip controls.

3. **[03-game-logic-upgrades.md](03-game-logic-upgrades.md)** — High-Impact Game Logic Enhancements
   - Ghost tunnel speed penalty ("Tunnel Trap").
   - Cornering speed differential ("Pre-turn cutting").
   - Dot-eating hesitation delay (1-frame chomp penalty).
   - Forbidden ghost turning intersections.
   - Dot-counter based house release rules.
   - Multi-tier level fruit progression table and Ms. Pac-Man wandering fruit.
