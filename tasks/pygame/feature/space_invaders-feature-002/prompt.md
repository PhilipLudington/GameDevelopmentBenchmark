# Progressive Difficulty Waves

## Problem Description

Currently, the game ends in victory when all aliens are destroyed. A proper Space Invaders game should have multiple waves of increasingly difficult aliens, challenging the player to survive as long as possible.

## Task

Implement a wave system where:
- Clearing all aliens starts the next wave instead of victory
- Each wave is progressively harder
- Wave number is displayed
- Victory only occurs after completing a set number of waves (or endless mode)

## Files

The main game file is `game/main.py`.

## Requirements

### Wave Management
- Track current wave number (starting at 1)
- After clearing all aliens, automatically start next wave
- Reset alien fleet positions but keep player position and score
- Preserve remaining lives between waves
- Brief pause/announcement between waves

### Progressive Difficulty
Each wave should increase difficulty through one or more of:
- Faster initial alien movement (decrease base `move_delay`)
- Aliens start lower on the screen (closer to player)
- Increased alien shooting frequency
- Faster alien bullets
- Fewer starting shields (or more damaged shields)

### Display
- Show current wave number in the HUD
- Optional: Show "Wave X" announcement at wave start

### Game Flow
- Option 1: Victory after completing 5 waves
- Option 2: Endless mode (play until death)
- Consider adding a wave transition state or brief pause

## Implementation Hints

Add to `Game` class:
```python
self.wave_number = 1
self.wave_transition_timer = 0
```

Modify `update()` to check if all aliens are dead:
```python
if self.fleet.all_dead:
    self.start_next_wave()
```

Create `start_next_wave()` method:
```python
def start_next_wave(self):
    self.wave_number += 1
    self.fleet.reset()
    # Apply difficulty increases
    self.fleet.move_delay = max(10, 30 - self.wave_number * 3)
    # etc.
```

## Expected Behavior

After implementation:
- Clearing all aliens spawns a new wave
- Wave 2+ is harder than Wave 1
- Wave number displayed on screen
- Player score and lives persist between waves
- Game ends after 5 waves (or player death)

## Testing

Run the automated tests:
```bash
pytest tests/ -v
```

Tests verify:
- Wave increments when aliens cleared
- Difficulty increases between waves
- Score persists across waves
- Wave number displays correctly
- Final victory condition works
