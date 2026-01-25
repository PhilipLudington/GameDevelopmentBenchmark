# Add Replay System with Deterministic Playback

## Feature Description
Implement a replay system that records inputs and random seed for perfect playback.

## Requirements
1. Record all inputs with frame timestamps
2. Save random seed for deterministic piece sequence
3. Save/load replays to file
4. Playback at variable speed (0.5x, 1x, 2x, 4x)
5. Seek to specific frame

## Testing

Run the automated tests:
```bash
pytest tests/ -v
```
