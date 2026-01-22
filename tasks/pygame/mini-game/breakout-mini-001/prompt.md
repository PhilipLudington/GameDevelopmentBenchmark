# Create Two-Player Cooperative Mode

## Problem Description

Add a two-player mode where one player controls a paddle at the bottom and another controls a paddle at the top. Players cooperate to keep the ball in play.

## Task

Implement two-player cooperative Breakout:
1. Two paddles: bottom (player 1) and top (player 2)
2. Different controls for each player
3. Bricks in the middle
4. Ball bounces between both paddles

## Files

The main game file is `game/main.py`. Major modifications to `Game` class.

## Requirements

1. Two paddles:
   - Player 1 (bottom): Left/Right arrow keys
   - Player 2 (top): A/D keys
   - Top paddle at y = 50

2. Modified brick layout:
   - Bricks in middle of screen
   - Space above and below for paddles

3. Ball mechanics:
   - Bounces off both paddles
   - Life lost if ball passes either paddle

4. Shared resources:
   - Shared lives (both players lose together)
   - Combined score

5. Controls:
   ```python
   # Player 1 (bottom paddle)
   if keys[pygame.K_LEFT]:
       self.paddle1.move_left()
   if keys[pygame.K_RIGHT]:
       self.paddle1.move_right()

   # Player 2 (top paddle)
   if keys[pygame.K_a]:
       self.paddle2.move_left()
   if keys[pygame.K_d]:
       self.paddle2.move_right()
   ```

## Expected Behavior

- Two paddles visible at top and bottom
- Both players can move their paddles
- Ball bounces off both paddles
- Bricks centered between paddles
- Game over when lives exhausted

## Testing

Run the automated tests:
```bash
pytest tests/ -v
```
