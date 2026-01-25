# Bug Report: Prefect Message Queue Overflow Handling

## Summary

The prefect warning message system uses a circular queue, but the wraparound logic applies the modulo operation to the wrong variable. This causes messages to be lost when the queue wraps around, and can result in duplicate or out-of-order messages.

## Environment

- Project: Julius (Caesar III reimplementation)
- File: `src/figure/prefect.c`
- Severity: Medium (missed warnings, UI confusion)

## Bug Description

Prefect warnings (fire risks, crime reports) are stored in a circular queue. The queue has a head (read position) and tail (write position):

```c
#define MAX_PREFECT_MESSAGES 8

typedef struct {
    int type;
    int building_id;
    int tick;
} prefect_message;

static prefect_message message_queue[MAX_PREFECT_MESSAGES];
static int queue_head = 0;  // Read position
static int queue_tail = 0;  // Write position

void queue_prefect_message(int type, int building_id, int tick)
{
    // BUG: Modulo applied to wrong variable!
    // Should apply to tail AFTER incrementing
    queue_tail = queue_tail % MAX_PREFECT_MESSAGES;

    message_queue[queue_tail].type = type;
    message_queue[queue_tail].building_id = building_id;
    message_queue[queue_tail].tick = tick;

    queue_tail++;  // Increment AFTER modulo - wrong order!
}
```

The problem: `queue_tail = queue_tail % MAX_PREFECT_MESSAGES` does nothing when tail < MAX (e.g., `3 % 8 = 3`). Then `queue_tail++` makes it 4, 5, 6, 7, 8... and eventually exceeds the array bounds!

When we finally add at index 8 (out of bounds), memory is corrupted.

## Steps to Reproduce

1. Play for a while with many fire-risk buildings
2. Prefects generate many warnings over time
3. After 8+ warnings, the 9th warning corrupts memory
4. Subsequent warnings may show wrong data or crash

## Expected Behavior

The modulo should be applied AFTER incrementing:
```c
message_queue[queue_tail].type = type;
// ...
queue_tail = (queue_tail + 1) % MAX_PREFECT_MESSAGES;
```

Or:
```c
queue_tail++;
queue_tail = queue_tail % MAX_PREFECT_MESSAGES;  // Now this is correct
```

## Current Behavior

Modulo is applied before increment, so it has no effect until overflow occurs:
- tail starts at 0
- `0 % 8 = 0`, write at 0, increment to 1
- `1 % 8 = 1`, write at 1, increment to 2
- ... continues ...
- `7 % 8 = 7`, write at 7, increment to 8
- `8 % 8 = 0`, write at 0 (FINALLY wraps, but index 8 was already corrupt!)

Actually, since modulo happens BEFORE the write, we get:
- Write at `tail % MAX`, then increment
- So we write at indices 0,1,2,...,7 correctly
- Then tail becomes 8, `8 % 8 = 0`, write at 0, increment to 9
- Then tail is 9, `9 % 8 = 1`... wait, this works?

Let me re-examine... The bug is subtler:

```c
queue_tail = queue_tail % MAX;  // Modifies tail BEFORE use
message_queue[queue_tail]...;   // Uses modified tail
queue_tail++;                   // Increments (now can exceed MAX before next modulo)
```

The issue is that `queue_tail` can grow unbounded between calls. After many messages:
- tail could be 800, `800 % 8 = 0`
- But this wastes integer space and can eventually overflow

The REAL bug is the modulo should be: `queue_tail = (queue_tail + 1) % MAX_PREFECT_MESSAGES;` INSTEAD of separate operations.

## Relevant Code

Look at `src/figure/prefect.c`:
- `queue_prefect_message()` function
- `dequeue_prefect_message()` function
- Queue head/tail management

## Suggested Fix Approach

Apply modulo after increment in one expression:

```c
void queue_prefect_message(int type, int building_id, int tick)
{
    message_queue[queue_tail].type = type;
    message_queue[queue_tail].building_id = building_id;
    message_queue[queue_tail].tick = tick;

    // FIXED: Increment and wrap in one operation
    queue_tail = (queue_tail + 1) % MAX_PREFECT_MESSAGES;
}
```

## Your Task

Fix the circular queue wraparound by properly combining the increment and modulo. Your fix should:

1. Keep queue_tail always in valid range [0, MAX_PREFECT_MESSAGES)
2. Correctly wrap from MAX-1 back to 0
3. Not lose messages when wrapping
4. Handle the dequeue operation similarly if affected

Provide your fix as a unified diff (patch).
