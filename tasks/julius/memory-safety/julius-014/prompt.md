# Bug Report: Double free in sound system cleanup

## Summary

The sound system has a double-free vulnerability where sound buffers are freed in an error handling path, but the same buffers are freed again during normal cleanup because the pointers are not set to NULL after the first free.

## Environment

- Project: Julius (Caesar III reimplementation)
- File: `src/sound/sound.c`
- Severity: High (memory corruption, potential code execution)

## Bug Description

The sound initialization function allocates multiple buffers for sound data. If an error occurs during initialization (e.g., a sound file fails to load), the error handling code frees already-allocated buffers. However, when the cleanup function is later called, it attempts to free the same buffers again.

```c
typedef struct {
    unsigned char *data;
    int size;
    int loaded;
} sound_buffer;

static sound_buffer sounds[MAX_SOUNDS];

int sound_init(void)
{
    for (int i = 0; i < MAX_SOUNDS; i++) {
        sounds[i].data = malloc(SOUND_BUFFER_SIZE);
        if (!sounds[i].data) {
            // Error path - free already allocated buffers
            for (int j = 0; j < i; j++) {
                free(sounds[j].data);
                // BUG: Pointer not set to NULL after free
            }
            return 0;
        }
    }
    return 1;
}

void sound_cleanup(void)
{
    for (int i = 0; i < MAX_SOUNDS; i++) {
        // BUG: May free already-freed pointers from error path
        free(sounds[i].data);
    }
}
```

## Steps to Reproduce

1. Start sound initialization
2. Have allocation fail partway through (e.g., buffer 5 of 10)
3. Error handler frees buffers 0-4
4. Later, sound_cleanup() is called
5. Cleanup tries to free buffers 0-4 again (double free)

## Expected Behavior

After freeing a buffer, the pointer should be set to NULL. The cleanup function should check for NULL before freeing.

## Current Behavior

The first free succeeds, but the second free of the same pointer causes:
- Memory corruption
- Potential heap exploitation
- Crash

AddressSanitizer output:
```
==12345==ERROR: AddressSanitizer: attempting double-free on 0x602000000010
    #0 free
    #1 sound_cleanup sound.c:89
```

## Relevant Code

Look at `src/sound/sound.c`:
- `sound_init()` function and its error handling
- `sound_cleanup()` function
- Any other places where sound buffers are freed

## Suggested Fix Approach

Set pointers to NULL after freeing and check before freeing:

```c
int sound_init(void)
{
    for (int i = 0; i < MAX_SOUNDS; i++) {
        sounds[i].data = malloc(SOUND_BUFFER_SIZE);
        if (!sounds[i].data) {
            for (int j = 0; j < i; j++) {
                free(sounds[j].data);
                sounds[j].data = NULL;  // Nullify after free
            }
            return 0;
        }
    }
    return 1;
}

void sound_cleanup(void)
{
    for (int i = 0; i < MAX_SOUNDS; i++) {
        if (sounds[i].data) {  // Check before free
            free(sounds[i].data);
            sounds[i].data = NULL;
        }
    }
}
```

## Your Task

Fix the double-free vulnerability in the sound system. Your fix should:

1. Set pointers to NULL after freeing in the error path
2. Add NULL checks before freeing in the cleanup function
3. Also nullify pointers in the cleanup function for consistency
4. Not change the external behavior of successful initialization

Provide your fix as a unified diff (patch).
