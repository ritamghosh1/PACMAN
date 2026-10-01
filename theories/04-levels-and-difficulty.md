# Theory 04 — Levels: Speed & Complexity

README: *"Different levels, Speed and Complexity will Change according to
levels."* All formulas live in `cfg::level(n)` (`src/config.h`).

## Formulas

| Parameter | Formula | L1 | L5 | L10 |
|---|---|---|---|---|
| Pac-Man speed (tiles/s) | `min(8.5, 7.0 + 0.15(n−1))` | 7.00 | 7.60 | 8.35 |
| Ghost speed (tiles/s) | `min(9.0, 5.5 + 0.40(n−1))` | 5.50 | 7.10 | 9.00 |
| Frightened duration (s) | `max(2.0, 6.0 − 0.6(n−1))` | 6.0 | 3.6 | 2.0 |
| House release base (s) | `max(0.6, 1.6 × 0.75^(n−1))` | 1.60 | 0.67 | 0.60 |
| Predictive aim (tiles) | `min(3, max(0, n−2))` | 0 | 1 | 3 |

## How the two levers work

**Speed** — both sides speed up, but ghosts gain **faster** (0.40 vs 0.15 per
level), so the margin shrinks from 1.5 tiles/s at L1 to a cap of 9.0 vs 8.5.
Pac-Man keeps a small edge because he only needs to outrun *one* ghost.

**Complexity** (what makes later levels *smarter*, not just faster):

1. `predictive` aim-ahead — ghosts start leading their shots (Pinky targets
   4+`predictive` tiles ahead, Inky's whole vector shifts, Blinky aims at the
   future tile).
2. Shorter frightened windows — fewer free ghost kills per pellet.
3. Faster house releases — pressure appears earlier after every death/level.

## Open questions

- Per-level maze variants?
- Scatter/chase schedule shrinking with level (currently fixed, theory 03)?
- Ghosts start pre-released (all 4 out) at high levels?
