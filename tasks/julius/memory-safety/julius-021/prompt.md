# Bug Report: Off-by-one error in array bounds check

## Summary

A loop iterating over a formation's soldiers uses `<=` instead of `<` in the loop condition, causing the loop to access one element past the end of the array.

## Environment

- Project: Julius (Caesar III reimplementation)
- File: `src/figure/formation.c`
- Severity: Medium (memory corruption, potential crash)

## Bug Description

When processing soldiers in a military formation, the code loops from 0 to the number of soldiers. However, using `<=` instead of `<` causes the loop to iterate one time too many, accessing soldiers[num_soldiers] which is out of bounds.

```c
#define MAX_SOLDIERS 16

typedef struct {
    int id;
    int health;
    int x, y;
} soldier;

typedef struct {
    soldier soldiers[MAX_SOLDIERS];
    int num_soldiers;
} formation;

void formation_update_soldiers(formation *f)
{
    // BUG: <= should be <
    for (int i = 0; i <= f->num_soldiers; i++) {
        // When i == num_soldiers, this accesses past the valid elements
        update_soldier(&f->soldiers[i]);
    }
}
```

If `num_soldiers` is 5, the loop runs for i = 0, 1, 2, 3, 4, 5, but soldiers[5] is not a valid soldier (only 0-4 are populated).

## Steps to Reproduce

1. Create a formation with soldiers
2. Call `formation_update_soldiers()`
3. Loop accesses soldiers[num_soldiers] which is uninitialized or out of bounds

## Expected Behavior

Loop should use `i < num_soldiers` to iterate exactly over valid soldiers.

## Current Behavior

The extra iteration accesses uninitialized memory or memory past the array bounds, causing:
- Reading garbage data
- Potential crash if num_soldiers == MAX_SOLDIERS

AddressSanitizer output:
```
==12345==ERROR: AddressSanitizer: heap-buffer-overflow on address 0x602000000050
READ of size 16 at 0x602000000050
    #0 update_soldier formation.c:89
    #1 formation_update_soldiers formation.c:76
```

## Relevant Code

Look at `src/figure/formation.c`:
- `formation_update_soldiers()` function
- Any other loops over soldiers array
- Similar patterns in other formation functions

## Suggested Fix Approach

Change the loop condition from `<=` to `<`:

```c
void formation_update_soldiers(formation *f)
{
    // FIXED: Use < instead of <=
    for (int i = 0; i < f->num_soldiers; i++) {
        update_soldier(&f->soldiers[i]);
    }
}
```

## Your Task

Fix the off-by-one error by correcting the loop condition. Your fix should:

1. Change `<=` to `<` in the loop condition
2. Ensure all valid soldiers (0 to num_soldiers-1) are still processed
3. Prevent access to soldiers[num_soldiers] which is out of bounds
4. Not change any other behavior

Provide your fix as a unified diff (patch).
