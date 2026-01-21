# Create 4-Player Variant

## Problem Description

Create an exciting 4-player variant of Pong where paddles are on all four sides of the screen.

## Task

Transform the standard 2-player Pong into a 4-player game.

## Files

The main game file is `game/main.py`.

## Expected Behavior

After your implementation:
- 4 paddles: top, bottom, left, and right sides of screen
- Player 1: Left paddle (W/S keys)
- Player 2: Right paddle (Up/Down arrows)
- Player 3: Top paddle (Q/E keys or similar)
- Player 4: Bottom paddle (Z/C keys or similar)
- Ball bounces off all paddles
- Players lose a life when ball passes their paddle
- Last player standing wins

## Hints

- You'll need to modify the Paddle class for horizontal paddles
- Update collision detection for all four sides
- Add life/score tracking for each player
- Consider the ball angle when hitting horizontal vs vertical paddles

## Testing

```bash
pytest tests/ -v
```
