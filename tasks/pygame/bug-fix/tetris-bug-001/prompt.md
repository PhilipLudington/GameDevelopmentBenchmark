# Fix T-Spin Detection Edge Cases

## Bug Description
The T-spin detection has edge cases where regular rotations are incorrectly identified as T-spins, and some valid T-spins are missed.

## Requirements
1. Implement proper 3-corner rule for T-spin detection
2. Detect mini T-spins vs full T-spins
3. Track last action (rotation vs movement)
4. Award correct bonus points for T-spins

## Testing

Run the automated tests:
```bash
pytest tests/ -v
```
