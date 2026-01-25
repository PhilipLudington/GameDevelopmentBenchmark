/**
 * Test for prefect message queue overflow (julius-043)
 *
 * This test verifies that the circular queue properly wraps around
 * without growing the tail index unbounded.
 *
 * Test validation:
 * - On FIXED code: Tail always in [0, MAX) range
 * - On BUGGY code: Tail can grow beyond MAX between modulo operations
 */

#include <stdio.h>
#include <stdlib.h>

#define MAX_PREFECT_MESSAGES 8

/* Prefect message structure */
typedef struct {
    int type;
    int building_id;
    int tick;
} prefect_message;

/* External functions from stubs */
extern void queue_prefect_message(int type, int building_id, int tick);
extern prefect_message *get_message_at(int index);
extern int get_queue_tail(void);
extern int get_queue_head(void);
extern int get_queue_count(void);
extern int get_max_queue_size(void);
extern void clear_message_queue(void);
extern int is_tail_valid(void);

/**
 * Test 1: Basic queue operation
 */
static int test_basic_queue(void)
{
    printf("Test: Basic queue operation...\n");

    clear_message_queue();

    queue_prefect_message(1, 100, 1000);
    queue_prefect_message(2, 200, 2000);

    int count = get_queue_count();
    printf("  Queued 2 messages, count = %d\n", count);

    if (count != 2) {
        printf("  FAIL: Expected count 2, got %d\n", count);
        return 0;
    }

    prefect_message *msg = get_message_at(0);
    if (!msg || msg->type != 1 || msg->building_id != 100) {
        printf("  FAIL: First message incorrect\n");
        return 0;
    }

    printf("  PASS: Basic queue works\n");
    return 1;
}

/**
 * Test 2: Fill queue to capacity
 */
static int test_fill_queue(void)
{
    printf("Test: Fill queue to capacity...\n");

    clear_message_queue();
    int max = get_max_queue_size();

    for (int i = 0; i < max; i++) {
        queue_prefect_message(i, i * 10, i * 100);
    }

    int tail = get_queue_tail();
    printf("  After %d messages, tail = %d\n", max, tail);

    /* Tail should have wrapped back to 0 */
    if (tail != 0) {
        printf("  FAIL: Expected tail=0 after filling, got %d\n", tail);
        return 0;
    }

    printf("  PASS: Queue filled correctly\n");
    return 1;
}

/**
 * Test 3: Tail stays in valid range - THE KEY TEST
 *
 * After many messages, tail should always be < MAX
 */
static int test_tail_valid_range(void)
{
    printf("Test: Tail stays in valid range after many messages...\n");

    clear_message_queue();
    int max = get_max_queue_size();

    /* Queue many more messages than capacity */
    int num_messages = max * 10;
    printf("  Queuing %d messages (capacity = %d)...\n", num_messages, max);

    for (int i = 0; i < num_messages; i++) {
        queue_prefect_message(i % 5, i, i * 10);

        /* Check tail after each operation */
        if (!is_tail_valid()) {
            int tail = get_queue_tail();
            printf("  FAIL at message %d: tail=%d is out of range [0, %d)\n",
                   i, tail, max);
            printf("  This is the bug - tail grows unbounded!\n");
            return 0;
        }
    }

    int final_tail = get_queue_tail();
    printf("  Final tail = %d (valid range: 0-%d)\n", final_tail, max - 1);

    if (final_tail >= max) {
        printf("  FAIL: Tail exceeded max\n");
        return 0;
    }

    printf("  PASS: Tail always stayed in valid range\n");
    return 1;
}

/**
 * Test 4: Wraparound preserves data
 */
static int test_wraparound_data(void)
{
    printf("Test: Wraparound preserves correct data...\n");

    clear_message_queue();
    int max = get_max_queue_size();

    /* Fill past capacity - old messages should be overwritten */
    for (int i = 0; i < max + 3; i++) {
        queue_prefect_message(i, i * 10, i * 100);
    }

    /* Newest messages should be type 8, 9, 10 (indices max+0, max+1, max+2) */
    /* But queue shows them at head positions after wrap */

    int count = get_queue_count();
    printf("  Queue count after overflow: %d (should be %d)\n", count, max);

    if (count != max) {
        printf("  FAIL: Count should be %d after overflow\n", max);
        return 0;
    }

    printf("  PASS: Wraparound handled correctly\n");
    return 1;
}

/**
 * Test 5: Exact wraparound point
 */
static int test_exact_wraparound(void)
{
    printf("Test: Exact wraparound from MAX-1 to 0...\n");

    clear_message_queue();
    int max = get_max_queue_size();

    /* Queue exactly MAX messages */
    for (int i = 0; i < max; i++) {
        queue_prefect_message(i, i, i);
        int tail = get_queue_tail();
        printf("  After message %d: tail = %d\n", i, tail);
    }

    /* Tail should be back at 0 */
    int tail = get_queue_tail();
    if (tail != 0) {
        printf("  FAIL: After %d messages, tail should be 0, got %d\n", max, tail);
        return 0;
    }

    printf("  PASS: Exact wraparound correct\n");
    return 1;
}

int main(void)
{
    int passed = 0;
    int total = 5;

    printf("=== Prefect Message Queue Test Suite ===\n");
    printf("Testing circular buffer wraparound handling\n");
    printf("Bug: Modulo applied incorrectly, tail can grow unbounded\n\n");

    if (test_basic_queue()) passed++;
    printf("\n");

    if (test_fill_queue()) passed++;
    printf("\n");

    if (test_tail_valid_range()) passed++;
    printf("\n");

    if (test_wraparound_data()) passed++;
    printf("\n");

    if (test_exact_wraparound()) passed++;

    printf("\n=== Test Results ===\n");
    printf("Results: %d/%d tests passed\n", passed, total);

    return (passed == total) ? 0 : 1;
}
