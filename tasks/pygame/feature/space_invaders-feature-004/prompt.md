# Alien Formation Attack Patterns

## Problem Description

In Galaga and advanced Space Invaders variants, individual aliens occasionally break from formation to perform dive-bombing attack runs. This creates more dynamic and challenging gameplay.

## Task

Implement a dive-bombing attack system where:
- Individual aliens can break from formation
- Diving aliens follow curved paths toward the player
- Diving aliens shoot while attacking
- Successful dives return aliens to formation (or destroy them at screen bottom)

## Files

The main game file is `game/main.py`.

## Requirements

### Diving State for Aliens
Extend the Alien class to track diving state:
```python
@dataclass
class Alien:
    # ... existing fields ...
    diving: bool = False
    dive_path: List[tuple] = None  # Bezier curve control points
    dive_progress: float = 0.0  # 0.0 to 1.0
    formation_pos: tuple = None  # Remember where to return
```

### Attack Selection
- Periodically select a random alien from bottom rows to dive
- Only aliens not already diving can be selected
- Chance increases as alien count decreases
- Limit max concurrent diving aliens (2-3)

### Dive Path Calculation
Use quadratic or cubic Bezier curves for smooth paths:
```python
def calculate_dive_path(start_pos, target_x, screen_height):
    # Create control points for a curved dive
    control1 = (start_pos[0] + random.randint(-100, 100),
                start_pos[1] + 100)
    control2 = (target_x + random.randint(-50, 50),
                screen_height - 100)
    end = (target_x, screen_height + 50)  # Off screen
    return [start_pos, control1, control2, end]
```

### Dive Behavior
During a dive:
1. Alien follows the Bezier curve path
2. Alien rotates to face movement direction (optional)
3. Alien shoots more frequently than normal
4. Alien moves faster than formation movement

### Return to Formation (Optional Advanced)
After diving past the screen:
1. Alien reappears from top
2. Flies back to its original formation position
3. Resumes normal formation movement

### Collision During Dive
- Diving aliens can still be shot by player
- Diving aliens' bullets can still hit player
- Diving alien colliding with player damages player (optional)

## Implementation Hints

Bezier curve interpolation:
```python
def bezier_point(t, points):
    """Calculate point on Bezier curve at parameter t (0-1)"""
    n = len(points) - 1
    result = [0, 0]
    for i, p in enumerate(points):
        coef = math.comb(n, i) * (1-t)**(n-i) * t**i
        result[0] += coef * p[0]
        result[1] += coef * p[1]
    return tuple(result)
```

## Expected Behavior

After implementation:
- Some aliens periodically break formation
- Diving aliens follow curved paths
- Diving aliens shoot during their attack
- Diving aliens can be shot and destroyed
- Creates more dynamic, challenging gameplay

## Testing

Run the automated tests:
```bash
pytest tests/ -v
```

Tests verify:
- Aliens can enter diving state
- Diving aliens follow path correctly
- Diving aliens can be destroyed mid-dive
- Formation aliens unaffected by diving
- Dive attack rate scales with game state
