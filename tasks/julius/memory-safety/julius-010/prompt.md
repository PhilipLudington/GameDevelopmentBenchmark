# Bug Report: Use-After-Free in UI Window Callback

## Summary

The UI system has a use-after-free vulnerability. When a dialog window is closed, its memory is freed, but pending callbacks may still reference the freed window structure, causing memory corruption.

## Environment

- Project: Julius (Caesar III reimplementation)
- File: `src/window/window.c`
- Severity: Critical (memory corruption, potential code execution)

## Bug Description

The UI uses a callback system for window events. When a window is closed, this sequence occurs:

```c
void window_close(window *w)
{
    if (w->on_close) {
        w->on_close(w);  // Callback may do cleanup
    }
    free(w);  // Window freed
    // BUG: Callback might have scheduled more events that reference w!
}
```

The problem is that callbacks can schedule additional events or animations that still hold references to the window. After `free(w)`, these references become dangling pointers:

```c
// In on_close callback:
void dialog_on_close(window *w)
{
    // This schedules an animation that will try to access w later
    start_fade_animation(w, on_fade_complete);
}

// Later, when animation completes:
void on_fade_complete(window *w)
{
    w->alpha = 0;  // USE-AFTER-FREE! Window already freed
}
```

## Steps to Reproduce

1. Open a dialog with fade-out animation on close
2. Close the dialog (triggers on_close callback)
3. on_close schedules fade animation
4. Window is freed
5. When fade completes, callback accesses freed memory

## Expected Behavior

The window system should:
1. Defer freeing until all callbacks have completed, OR
2. Cancel pending callbacks before freeing, OR
3. Use reference counting to track usage

## Current Behavior

Window is freed while callbacks still hold references:
```
==12345==ERROR: AddressSanitizer: heap-use-after-free on address 0x...
READ of size 4 at 0x... thread T0
```

## Relevant Code

Look at `src/window/window.c`, specifically:
- `window_close()` function
- Callback scheduling system
- Any deferred/async event handling

## Suggested Fix Approach

Option 1: Mark window as "closing" and defer free:
```c
void window_close(window *w)
{
    w->state = WINDOW_STATE_CLOSING;  // Mark as closing
    if (w->on_close) {
        w->on_close(w);
    }
    // Don't free yet - add to cleanup list
    add_to_cleanup_list(w);
}

void window_cleanup_cycle(void)
{
    // Free windows that are safe to free (no pending callbacks)
    for (window *w = cleanup_list; w; w = next) {
        if (w->pending_callbacks == 0) {
            remove_from_cleanup_list(w);
            free(w);
        }
    }
}
```

Option 2: Reference counting:
```c
void window_close(window *w)
{
    w->state = WINDOW_STATE_CLOSING;
    if (w->on_close) {
        w->on_close(w);
    }
    window_release(w);  // Decrement ref count, free if zero
}

void window_retain(window *w) { w->ref_count++; }
void window_release(window *w) {
    if (--w->ref_count == 0) {
        free(w);
    }
}
```

## Your Task

Fix the use-after-free by implementing a safe cleanup mechanism. Your fix should:
1. Prevent accessing window memory after it's freed
2. Ensure callbacks can safely reference the window during their execution
3. Eventually free the window when truly no longer needed
4. Handle nested callbacks correctly

Provide your fix as a unified diff (patch).
