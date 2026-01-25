# Bug Report: Null pointer dereference in empty walker list

## Summary

The walker system iterates over a linked list of active walkers without checking if the list is empty (head is NULL), causing a null pointer dereference when processing an empty list.

## Environment

- Project: Julius (Caesar III reimplementation)
- File: `src/figure/walker.c`
- Severity: Medium (crash, denial of service)

## Bug Description

Walkers (citizens moving around the map) are tracked in a linked list. The `walker_update_all()` function iterates over all active walkers to update their positions. However, it doesn't check if the list head is NULL before accessing it.

```c
typedef struct walker {
    int id;
    int x, y;
    int destination_x, destination_y;
    struct walker *next;
} walker;

static walker *walker_list_head = NULL;

void walker_update_all(void)
{
    // BUG: No null check before dereferencing
    walker *current = walker_list_head;

    // Immediately accesses current->id without checking if current is NULL
    while (current->next != NULL) {
        walker_update_position(current);
        current = current->next;
    }
}
```

When the game first starts or after all walkers are removed, `walker_list_head` is NULL, and accessing `current->next` crashes.

## Steps to Reproduce

1. Start a new game (no walkers exist yet)
2. Game calls `walker_update_all()` in the main loop
3. Null pointer dereference on empty list

## Expected Behavior

The function should safely handle an empty list:
- Check if head is NULL before iteration
- Do nothing and return if list is empty

## Current Behavior

Attempting to access `current->next` when `current` is NULL causes:
- Segmentation fault
- Game crash

## Relevant Code

Look at `src/figure/walker.c`:
- `walker_update_all()` function
- Other functions that iterate over the walker list
- List head initialization

## Suggested Fix Approach

Add proper null checking:

```c
void walker_update_all(void)
{
    walker *current = walker_list_head;

    // Check for empty list
    while (current != NULL) {
        walker_update_position(current);
        current = current->next;
    }
}
```

Or with an early return:

```c
void walker_update_all(void)
{
    if (walker_list_head == NULL) {
        return;  // Empty list, nothing to do
    }

    walker *current = walker_list_head;
    while (current != NULL) {
        walker_update_position(current);
        current = current->next;
    }
}
```

## Your Task

Fix the null pointer dereference by adding proper empty list handling. Your fix should:

1. Check if the list head is NULL before iteration
2. Safely handle the empty list case
3. Continue to process all walkers when list is non-empty
4. Not change the function signature or semantics

Provide your fix as a unified diff (patch).
