# Reduce CPU Usage in Main Loop

## Problem Description

The game uses excessive CPU resources even when nothing is happening. This drains laptop batteries and causes unnecessary heat.

## Task

Optimize the main game loop to reduce CPU usage while maintaining smooth gameplay.

## Files

The main game file is `game/main.py`. Focus on the game loop and rendering.

## Expected Behavior

After your optimization:
- CPU usage should be significantly reduced (target: under 25%)
- Game should still run at 60 FPS
- Gameplay should feel identical to before
- No visual or gameplay regressions

## Hints

- Check if pygame.time.Clock is being used correctly
- Consider if the display needs to update every frame
- Look for any busy-waiting or unnecessary calculations

## Testing

```bash
pytest tests/ -v
```
