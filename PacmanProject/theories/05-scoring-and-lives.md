# Theory 05 — Scoring and Lives

## Points table (`src/game.cpp`)

| Event | Points |
|---|---|
| Dot | 10 |
| Power pellet | 50 |
| Ghost 1st in chain | 200 |
| Ghost 2nd | 400 |
| Ghost 3rd | 800 |
| Ghost 4th (cap) | 1600 |
| Extra life | at 10 000 total, once per game |

Chain: `frightChain_` increments per eaten ghost, resets when frightened mode
ends or Pac-Man dies. Score uses `200 << min(chain, 3)`.

## Lives

- Start with **3 lives**.
- Death (collision with a non-frightened ghost) → 1.5s death animation →
  lives −1 → respawn everything (actors only; dots keep their state) →
  READY 2s.
- Lives ≤ 0 → GAME OVER screen, **R** restarts a fresh game.
- HUD draws one Pac-Man icon per life (including the one in play).

## What is *not* scored yet

- Level-clear bonus (classic awards 50/level, turbofruite etc. — see
  roadmap), fruit bonuses, 1UP flashing counter.
