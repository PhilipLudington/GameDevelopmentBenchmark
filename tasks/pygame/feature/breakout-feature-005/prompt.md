# Add Sound Effects

## Problem Description

The game is silent. Add sound effects for key game events.

## Task

Add sound effects for:
1. Ball hitting paddle
2. Ball hitting brick
3. Ball hitting wall
4. Losing a life
5. Game over
6. Victory

## Files

The main game file is `game/main.py`. You'll need to add sound loading and playing.

## Requirements

1. Initialize pygame mixer
2. Create simple synthesized sounds or load from files
3. Play sounds at appropriate times
4. Sounds should work in non-headless mode only

## Implementation Approach

For simplicity, use pygame's built-in sound generation:
```python
import pygame.mixer

# Initialize mixer
pygame.mixer.init()

# Create simple beep sounds using numpy (if available) or silent fallback
def create_beep(frequency, duration_ms):
    try:
        import numpy as np
        sample_rate = 44100
        n_samples = int(sample_rate * duration_ms / 1000)
        t = np.linspace(0, duration_ms/1000, n_samples, False)
        wave = np.sin(2 * np.pi * frequency * t) * 0.3
        wave = (wave * 32767).astype(np.int16)
        stereo = np.column_stack([wave, wave])
        sound = pygame.sndarray.make_sound(stereo)
        return sound
    except ImportError:
        return None
```

Or just use placeholder sounds and document that sound files are needed.

## Expected Behavior

- Distinct sounds for each event type
- Sounds only play when not in headless mode
- Game works without errors if sounds fail to load

## Testing

Run the automated tests:
```bash
pytest tests/ -v
```
