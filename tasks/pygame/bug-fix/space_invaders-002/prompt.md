# Alien Fleet Speed Calculation Overflow

## Problem Description

Players have discovered that when only a few aliens remain (typically 1-3), the alien fleet movement becomes erratic or freezes entirely. The aliens should speed up dramatically as their numbers decrease, but instead they sometimes stop moving or move at unexpected speeds.

This bug involves a subtle issue with how the `move_delay` is calculated based on the number of remaining aliens.

## Task

Find and fix the bug in the `AlienFleet.update()` method that causes incorrect movement timing when few aliens remain.

## Files

The main game file is `game/main.py`. Focus on the `AlienFleet` class and its `update` method.

## The Core Issue

The current calculation for `move_delay` uses:
```python
self.move_delay = max(5, int(30 * alive_count / (ALIEN_ROWS * ALIEN_COLS)))
```

This formula has multiple issues:
1. When `alive_count` is very small (1-5), the division can result in 0 or very small values
2. The `max(5, ...)` prevents the delay from going below 5, but this creates a speed plateau
3. The linear relationship doesn't create the classic "panic speedup" feel

The expected behavior is that the last few aliens should move much faster (almost frame-by-frame), creating tension.

## Expected Behavior

After your fix:
- With 55 aliens (full fleet): move_delay should be around 30 frames
- With 27 aliens (half): move_delay should be around 15 frames
- With 10 aliens: move_delay should be around 5-8 frames
- With 5 aliens: move_delay should be around 3-4 frames
- With 1 alien: move_delay should be 1-2 frames (very fast!)

The speed increase should feel exponential as aliens decrease, not linear.

## Implementation Hints

Consider using an exponential or logarithmic formula:
- `move_delay = max(1, int(base_delay * (alive_count / total_aliens) ** 0.5))`
- Or: `move_delay = max(1, int(math.log2(alive_count + 1) * some_factor))`

The key is that the last few aliens should move dramatically faster than a linear formula would predict.

## Testing

Run the automated tests:
```bash
pytest tests/ -v
```

The tests verify:
- Full fleet has reasonable movement speed
- Movement speeds up as aliens are destroyed
- Single remaining alien moves very fast (delay <= 2)
- No freeze or erratic behavior at any alien count
