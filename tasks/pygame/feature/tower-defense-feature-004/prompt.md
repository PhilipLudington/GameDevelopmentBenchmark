# Implement Adaptive Wave Difficulty

## Feature Description
Create a dynamic difficulty system that adjusts waves based on player performance.

## Requirements
1. Track player metrics (lives lost, gold earned, time per wave)
2. Adjust enemy count and types based on performance
3. Scale between easy/medium/hard difficulty bands
4. Prevent death spirals (ease up if struggling)
5. Reward skilled play with bonus waves

## Testing

Run the automated tests:
```bash
pytest tests/ -v
```
