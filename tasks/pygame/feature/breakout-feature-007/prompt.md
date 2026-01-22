# Add Pause Countdown

## Problem Description

When resuming from pause, the game instantly starts which can catch players off guard. Add a 3-2-1 countdown before resuming.

## Task

Implement countdown when unpausing:
1. Add COUNTDOWN state to GameState
2. Display 3, 2, 1 countdown
3. Each number shows for 1 second
4. Game resumes after countdown

## Files

The main game file is `game/main.py`. Modify `GameState` enum and `Game` class.

## Requirements

1. Add `COUNTDOWN` to GameState enum

2. Modify state transitions:
   - PAUSED -> COUNTDOWN (when pressing space/P)
   - COUNTDOWN -> PLAYING (after countdown completes)

3. Add countdown tracking:
   - `self.countdown_value` (3, 2, 1)
   - `self.countdown_timer` (time tracking)

4. Update countdown in `update()`:
   - Decrement countdown_value every second
   - When reaches 0, transition to PLAYING

5. Draw countdown in `draw()`:
   - Large centered number
   - Maybe with animation/scaling

## Expected Behavior

- Pressing space in pause shows "3"
- After 1 second, shows "2"
- After 1 second, shows "1"
- After 1 second, game resumes
- Ball and paddle frozen during countdown

## Testing

Run the automated tests:
```bash
pytest tests/ -v
```
