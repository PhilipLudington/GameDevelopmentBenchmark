# Bug Report: Tooltip Trailing Newline

## Summary

Tooltips in the game sometimes display with an unwanted trailing newline, causing extra whitespace at the bottom. This is caused by a buggy width calculation algorithm in the multiline text layout code.

## Symptoms

- Tooltips have inconsistent heights
- Extra blank line appears at the bottom of some tooltips
- Text wrapping behaves incorrectly near the boundary width

## Technical Details

The bug is in `src/graphics/text.c` in both `text_draw_multiline()` and `text_measure_multiline()` functions. The issue is in how the algorithm determines when to wrap to a new line.

The current logic:
1. Adds the word width to current_width first
2. Then checks if current_width >= box_width
3. This can cause the algorithm to calculate a width larger than necessary

The correct logic should:
1. Check if adding the next word would exceed box_width BEFORE adding it
2. Break the line at the right point
3. Only add word_width if the word fits

## Relevant Code Location

File: `src/graphics/text.c`
Functions: `text_draw_multiline()`, `text_measure_multiline()`

Look for the while loop that processes words and the order of operations between adding word_width and checking against box_width.

## Expected Fix

The fix should:
1. Check `current_width + word_width >= box_width` BEFORE adding word_width
2. Only add word_width to current_width if the word fits
3. Move the `break` statement to the right location
4. Apply the same fix to both functions

## Testing

To verify the fix:
1. Test with text strings that exactly fill the box width
2. Test with strings that are one character over the width
3. Verify no extra newlines appear in tooltips

## Constraints

- The fix should be applied consistently to both affected functions
- Do not change the visual appearance of correctly-rendered text
- Maintain the same line breaking behavior for normal cases
