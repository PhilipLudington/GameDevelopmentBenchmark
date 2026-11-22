# Two-Player Cooperative Mode

## Problem Description

Create a local two-player cooperative mode where both players work together to defeat the alien invasion. This requires significant changes to support two player ships, separate controls, and cooperative mechanics.

## Task

Implement a two-player cooperative mode with:
- Two player ships on screen
- Separate controls for each player
- Shared or individual lives system
- Cooperative scoring

## Files

The main game file is `game/main.py`.

## Requirements

### Two Player Ships
- Player 1: Left side of screen, controls with A/D and W to shoot
- Player 2: Right side of screen, controls with Arrow keys and Space to shoot
- Both ships operate independently
- Ships cannot pass through each other (optional: can overlap)

### Control Scheme
**Player 1:**
- A: Move left
- D: Move right
- W: Shoot

**Player 2:**
- Left Arrow: Move left
- Right Arrow: Move right
- Space: Shoot (or Up Arrow)

### Lives System (choose one approach)

**Option A - Shared Lives:**
- Team starts with 5 shared lives
- Either player hit = lose one team life
- Game over when team lives = 0

**Option B - Individual Lives:**
- Each player starts with 3 lives
- Player with 0 lives is "out" but game continues
- Game over when both players are out

### Scoring
- Shared score (both contribute to same total)
- Optionally track per-player stats
- High score system still works

### Visual Changes
- Two ships visible on screen
- Ships have different colors (Player 1: Green, Player 2: Blue/Cyan)
- HUD shows both players' status if using individual lives
- Clear visual distinction between players

### Collision Handling
- Both players can shoot aliens
- Alien bullets can hit either player
- Player bullets from both players work identically
- Consider: Can players shoot each other? (recommend: no)

### Mode Selection
Add menu option:
- "1 Player" - standard game
- "2 Players" - cooperative mode

## Implementation Hints

Modify `Game.__init__`:
```python
self.num_players = 1  # or 2
self.players = [Player(1)]  # or [Player(1), Player(2)]
```

Modify Player class:
```python
class Player:
    def __init__(self, player_num: int):
        self.player_num = player_num
        self.color = GREEN if player_num == 1 else CYAN
        # Set starting position based on player_num
```

Handle input separately:
```python
def handle_continuous_input(self):
    keys = pygame.key.get_pressed()

    # Player 1 controls
    if len(self.players) >= 1:
        if keys[pygame.K_a]:
            self.players[0].move_left()
        # ...

    # Player 2 controls
    if len(self.players) >= 2:
        if keys[pygame.K_LEFT]:
            self.players[1].move_left()
        # ...
```

## Expected Behavior

After implementation:
- Menu allows choosing 1 or 2 player mode
- Two player mode shows two ships
- Each player has independent controls
- Both players can shoot and score
- Game handles both players being hit
- Works with existing alien AI and shields

## Testing

Run the automated tests:
```bash
pytest tests/ -v
```

Tests verify:
- Two players can be created and controlled independently
- Both players can shoot and destroy aliens
- Alien bullets can hit either player
- Lives system works correctly
- Score accumulates from both players
- Game over conditions work for both modes
