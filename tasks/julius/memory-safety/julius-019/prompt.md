# Bug Report: Race condition in message queue access

## Summary

The game message queue (for advisor alerts, events, etc.) can be accessed concurrently from the main game loop and from event handlers, causing data corruption when messages are added during queue processing.

## Environment

- Project: Julius (Caesar III reimplementation)
- File: `src/city/message.c`
- Severity: High (data corruption, unpredictable behavior)

## Bug Description

The message queue stores pending notifications for the player. The main loop processes messages while event handlers can add new messages. Without synchronization, concurrent access corrupts the queue.

```c
#define MAX_MESSAGES 100

typedef struct {
    int type;
    int param1;
    int param2;
    int processed;
} message;

static message queue[MAX_MESSAGES];
static int queue_head = 0;
static int queue_tail = 0;

void message_add(int type, int param1, int param2)
{
    // BUG: No synchronization
    queue[queue_tail].type = type;
    queue[queue_tail].param1 = param1;
    queue[queue_tail].param2 = param2;
    queue[queue_tail].processed = 0;
    queue_tail = (queue_tail + 1) % MAX_MESSAGES;
}

message *message_get_next(void)
{
    // BUG: No synchronization
    if (queue_head == queue_tail) {
        return NULL;  // Queue empty
    }
    message *msg = &queue[queue_head];
    queue_head = (queue_head + 1) % MAX_MESSAGES;
    return msg;
}
```

When `message_add()` is called while `message_get_next()` is running:
- Queue indices can be corrupted
- Messages can be lost or duplicated
- Partial writes can be read

## Steps to Reproduce

1. Process messages in main loop
2. Event handler fires and adds a message
3. Race between add and get causes corruption

## Expected Behavior

Message queue operations should be atomic or properly synchronized to prevent concurrent modification.

## Current Behavior

Concurrent access causes:
- Lost messages
- Duplicate processing
- Corrupted queue state
- Intermittent crashes

## Relevant Code

Look at `src/city/message.c`:
- `message_add()` function
- `message_get_next()` function
- Any other queue access functions

## Suggested Fix Approach

Option 1: Use a simple lock:
```c
static int queue_locked = 0;

void message_add(int type, int param1, int param2)
{
    while (queue_locked) { /* spin */ }
    queue_locked = 1;

    // ... queue operations ...

    queue_locked = 0;
}
```

Option 2: Use atomic operations for indices (preferred for single-producer single-consumer):
```c
#include <stdatomic.h>

static _Atomic int queue_tail = 0;
static _Atomic int queue_head = 0;
```

Option 3: Copy-on-write with double buffering.

## Your Task

Fix the race condition by adding proper synchronization. Your fix should:

1. Prevent concurrent modification of the queue
2. Ensure message_add and message_get_next don't interfere
3. Not introduce deadlocks
4. Maintain queue semantics (FIFO order)

Provide your fix as a unified diff (patch).
