# Bug Report: Double Free in Smacker Video Decoder

## Summary

The smacker video decoder in Julius has a double-free vulnerability. When certain error conditions occur during video loading, frame information pointers are freed multiple times, leading to memory corruption.

## Environment

- Project: Julius (Caesar III reimplementation)
- File: `src/core/smacker.c`
- Severity: High (memory corruption, potential security issue)

## Bug Description

The smacker decoder allocates memory for frame information during video loading:
- `frame_sizes` - sizes of each video frame
- `frame_offsets` - file offsets for each frame
- `frame_types` - type flags for each frame

These pointers are freed in multiple code paths:
1. In error handling within `read_frame_info()` when allocation fails
2. In `smacker_close()` during normal cleanup

The problem is that if an error occurs after partial allocation, some pointers may be freed in the error handler, and then freed again when `smacker_close()` is called, causing a double-free.

## Steps to Reproduce

1. Attempt to load a malformed or truncated SMK video file
2. Trigger an allocation failure during frame info reading
3. The error path frees the partially allocated data
4. When cleanup runs, the same pointers are freed again

## Expected Behavior

Each pointer should be freed exactly once, regardless of error conditions.

## Current Behavior

Pointers can be freed multiple times, triggering AddressSanitizer:
```
==12345==ERROR: AddressSanitizer: attempting double-free on 0x...
```

## Relevant Code

Look at `src/core/smacker.c`, specifically:
- The `read_frame_info()` function and its error handling
- The `smacker_close()` cleanup function
- How `frame_sizes`, `frame_offsets`, and `frame_types` are managed

## Suggested Fix Approach

Consider:
1. Centralizing cleanup into a single function
2. Setting pointers to NULL after freeing
3. Checking for NULL before freeing (though this alone doesn't prevent the issue if the pointer isn't nullified)

## Your Task

Fix the double-free bug by ensuring each pointer is freed exactly once. Your fix should:
1. Prevent double-free in all error paths
2. Still properly clean up memory in all cases
3. Be minimal and targeted

Provide your fix as a unified diff (patch).
