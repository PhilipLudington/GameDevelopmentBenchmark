/**
 * Stub implementation for julius-043 prefect message queue test
 *
 * This provides mock circular queue for testing wraparound handling.
 */

#include <stdio.h>
#include <string.h>

#define MAX_PREFECT_MESSAGES 8

/* Prefect message structure */
typedef struct {
    int type;
    int building_id;
    int tick;
} prefect_message;

/* Circular queue */
static prefect_message message_queue[MAX_PREFECT_MESSAGES];
static int queue_head = 0;
static int queue_tail = 0;
static int queue_count = 0;

/**
 * Add message to queue
 *
 * BUGGY version: queue_tail = tail % MAX; then tail++
 *   - tail can grow unbounded between modulo checks
 *
 * FIXED version: queue_tail = (tail + 1) % MAX
 *   - tail always stays in [0, MAX) range
 */
void queue_prefect_message(int type, int building_id, int tick)
{
    message_queue[queue_tail].type = type;
    message_queue[queue_tail].building_id = building_id;
    message_queue[queue_tail].tick = tick;

    /* FIXED: Increment and wrap in one operation */
    queue_tail = (queue_tail + 1) % MAX_PREFECT_MESSAGES;

    if (queue_count < MAX_PREFECT_MESSAGES) {
        queue_count++;
    } else {
        /* Queue full, head advances (oldest message dropped) */
        queue_head = (queue_head + 1) % MAX_PREFECT_MESSAGES;
    }
}

/* Get message from queue (peek) */
prefect_message *get_message_at(int index)
{
    if (index < 0 || index >= MAX_PREFECT_MESSAGES) {
        return NULL;
    }
    int actual_index = (queue_head + index) % MAX_PREFECT_MESSAGES;
    return &message_queue[actual_index];
}

/* Get queue tail value (for testing) */
int get_queue_tail(void)
{
    return queue_tail;
}

/* Get queue head value (for testing) */
int get_queue_head(void)
{
    return queue_head;
}

/* Get queue count */
int get_queue_count(void)
{
    return queue_count;
}

/* Get max queue size */
int get_max_queue_size(void)
{
    return MAX_PREFECT_MESSAGES;
}

/* Clear the queue */
void clear_message_queue(void)
{
    memset(message_queue, 0, sizeof(message_queue));
    queue_head = 0;
    queue_tail = 0;
    queue_count = 0;
}

/* Check if tail is in valid range */
int is_tail_valid(void)
{
    return queue_tail >= 0 && queue_tail < MAX_PREFECT_MESSAGES;
}
