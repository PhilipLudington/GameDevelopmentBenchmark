# Power-Up System

## Problem Description

Modern arcade shooters include power-ups to add variety and excitement. Implement a power-up system where destroying aliens has a chance to drop collectible items that grant temporary abilities.

## Task

Implement a complete power-up system with:
- At least 3 different power-up types
- Visual representation for each type
- Collection mechanics (player touches power-up)
- Timed effects that expire
- Power-up drops from destroyed aliens

## Files

The main game file is `game/main.py`.

## Requirements

### Power-Up Types (implement at least 3)

1. **Rapid Fire** (suggested)
   - Reduces shoot cooldown by 50-75%
   - Duration: 5-10 seconds
   - Visual: Yellow/orange color

2. **Shield Repair** (suggested)
   - Repairs 20-30% of shield damage
   - Instant effect (no duration)
   - Visual: Green color

3. **Wide Shot** (suggested)
   - Fires 3 bullets in a spread pattern
   - Duration: 5-10 seconds
   - Visual: Blue color

4. **Extra Life** (suggested)
   - Adds one life
   - Instant effect
   - Visual: Pink/heart color

5. **Slow Aliens** (suggested)
   - Slows alien movement for a duration
   - Duration: 5-10 seconds
   - Visual: Cyan color

### Power-Up Class
```python
@dataclass
class PowerUp:
    x: float
    y: float
    type: str  # 'rapid_fire', 'shield_repair', etc.
    fall_speed: float = 2
```

### Mechanics
- 5-15% chance for power-up drop when alien destroyed
- Power-ups fall downward slowly
- Collected when touching player ship
- Disappear if they reach bottom of screen
- Active power-ups shown in HUD (with timer if applicable)
- Only one of each type active at a time (refreshes duration if collected again)

### Player State Tracking
Track active power-ups on the Game or Player class:
```python
self.active_powerups = {
    'rapid_fire': 0,  # Timer in frames, 0 = inactive
    'wide_shot': 0,
}
```

### Integration
- Modify `Game.update()` to update falling power-ups and timers
- Modify shooting logic to check for rapid_fire and wide_shot
- Add power-up drawing in `_draw_game()`
- Show active power-up indicators in HUD

## Expected Behavior

After implementation:
- Destroying aliens occasionally drops power-ups
- Power-ups fall slowly and can be collected
- Collected power-ups activate their effects
- Timed effects expire and UI updates accordingly
- Multiple different power-up types function correctly

## Testing

Run the automated tests:
```bash
pytest tests/ -v
```

Tests verify:
- Power-ups spawn on alien destruction (probability)
- Power-ups fall and can be collected
- Each power-up type has correct effect
- Timed power-ups expire correctly
- Power-ups don't persist across game reset
