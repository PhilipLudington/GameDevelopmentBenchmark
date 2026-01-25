# Add Combo and Back-to-Back Scoring

## Feature Description
Implement the guideline combo and back-to-back bonus systems.

## Requirements
1. Track combo counter (consecutive line clears)
2. Combo bonus: 50 * combo * level points
3. Track back-to-back (consecutive Tetrises or T-spins)
4. B2B bonus: 1.5x multiplier
5. Display combo and B2B status in HUD

## Testing

Run the automated tests:
```bash
pytest tests/ -v
```
