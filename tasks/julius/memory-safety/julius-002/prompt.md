# Bug Report: Dangling Pointer on localized_filename

## Summary

A dangling pointer vulnerability exists in `src/game/file.c` in the `game_file_write_mission_saved_game()` function. The bug causes a stack-use-after-scope error when saving mission files with localization enabled.

## Symptoms

- Crash or undefined behavior when saving mission files
- AddressSanitizer reports `stack-use-after-scope` error
- Corrupted saved game files intermittently

## Technical Details

The bug is in the `game_file_write_mission_saved_game()` function around the handling of `localized_filename`. A local variable is declared inside a conditional block but is used after the block ends, causing the pointer to reference deallocated stack memory.

## Relevant Code Location

File: `src/game/file.c`
Function: `game_file_write_mission_saved_game()`

Look for the conditional block that handles locale translation and the `localized_filename` variable declaration.

## Expected Fix

The fix should ensure that any stack-allocated variables used outside their declaration scope are moved to an appropriate scope where they remain valid for the duration of their use.

## Testing

To verify the fix:
1. Build with AddressSanitizer enabled (`-fsanitize=address`)
2. Run the test that triggers the mission save with localization
3. Confirm no ASan errors occur

## Constraints

- The fix should be minimal and targeted
- Do not change the overall logic of the function
- Maintain backward compatibility with existing save files
