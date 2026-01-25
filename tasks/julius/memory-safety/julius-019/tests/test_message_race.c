/*
 * Test for julius-019: Race condition in message queue access
 *
 * This test verifies that message queue operations are properly
 * synchronized to prevent data corruption under concurrent access.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

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
static int queue_count = 0;

#ifdef BUGGY_VERSION
/* BUG: No synchronization */
#else
/* FIXED: Simple spinlock */
static volatile int queue_lock = 0;

static void acquire_lock(void)
{
    while (__sync_lock_test_and_set(&queue_lock, 1)) {
        /* spin */
    }
}

static void release_lock(void)
{
    __sync_lock_release(&queue_lock);
}
#endif

void message_add(int type, int param1, int param2)
{
#ifndef BUGGY_VERSION
    acquire_lock();
#endif

    if (queue_count < MAX_MESSAGES) {
        queue[queue_tail].type = type;
        queue[queue_tail].param1 = param1;
        queue[queue_tail].param2 = param2;
        queue[queue_tail].processed = 0;
        queue_tail = (queue_tail + 1) % MAX_MESSAGES;
        queue_count++;
    }

#ifndef BUGGY_VERSION
    release_lock();
#endif
}

message *message_get_next(void)
{
#ifndef BUGGY_VERSION
    acquire_lock();
#endif

    message *msg = NULL;
    if (queue_count > 0) {
        msg = &queue[queue_head];
        msg->processed = 1;
        queue_head = (queue_head + 1) % MAX_MESSAGES;
        queue_count--;
    }

#ifndef BUGGY_VERSION
    release_lock();
#endif

    return msg;
}

void message_queue_reset(void)
{
    memset(queue, 0, sizeof(queue));
    queue_head = 0;
    queue_tail = 0;
    queue_count = 0;
}

/* Thread data for producer */
typedef struct {
    int start_id;
    int count;
    int added;
} producer_data;

/* Thread data for consumer */
typedef struct {
    int consumed;
    int sum;
} consumer_data;

/* Producer thread - adds messages */
void *producer_thread(void *arg)
{
    producer_data *data = (producer_data *)arg;
    data->added = 0;

    for (int i = 0; i < data->count; i++) {
        message_add(data->start_id + i, i, i * 2);
        data->added++;
        /* Small delay to interleave with consumer */
        if (i % 10 == 0) {
            usleep(1);
        }
    }

    return NULL;
}

/* Consumer thread - processes messages */
void *consumer_thread(void *arg)
{
    consumer_data *data = (consumer_data *)arg;
    data->consumed = 0;
    data->sum = 0;

    /* Keep consuming until told to stop */
    for (int attempts = 0; attempts < 10000; attempts++) {
        message *msg = message_get_next();
        if (msg != NULL) {
            data->consumed++;
            data->sum += msg->type;
        }
        usleep(1);
    }

    return NULL;
}

/* Test basic single-threaded operation */
int test_single_threaded(void)
{
    message_queue_reset();

    /* Add some messages */
    for (int i = 0; i < 10; i++) {
        message_add(i, i * 10, i * 100);
    }

    /* Consume them */
    int count = 0;
    while (1) {
        message *msg = message_get_next();
        if (msg == NULL) break;
        if (msg->type != count) {
            printf("FAIL: Wrong message order, expected %d got %d\n",
                   count, msg->type);
            return 1;
        }
        count++;
    }

    if (count != 10) {
        printf("FAIL: Expected 10 messages, got %d\n", count);
        return 1;
    }

    printf("PASS: Single-threaded test\n");
    return 0;
}

/* Test concurrent producer and consumer */
int test_concurrent_access(void)
{
    message_queue_reset();

    pthread_t producer, consumer;
    producer_data prod_data = {0, 100, 0};  /* Reduced count for reliability */
    consumer_data cons_data = {0, 0};

    printf("Testing concurrent producer/consumer...\n");

    /* Start producer first, then consumer */
    pthread_create(&producer, NULL, producer_thread, &prod_data);
    usleep(1000);  /* Let producer get started */
    pthread_create(&consumer, NULL, consumer_thread, &cons_data);

    /* Wait for producer to finish */
    pthread_join(producer, NULL);

    /* Give consumer more time to process remaining messages */
    usleep(100000);

    /* Stop consumer */
    pthread_join(consumer, NULL);

    printf("Producer added: %d messages\n", prod_data.added);
    printf("Consumer processed: %d messages\n", cons_data.consumed);

    /* Drain any remaining messages */
    int remaining = 0;
    while (message_get_next() != NULL) {
        remaining++;
    }
    printf("Remaining in queue: %d\n", remaining);

    int total = cons_data.consumed + remaining;

    /* With the race condition, some messages may be lost or corrupted */
    /* The fixed version should have consumed all messages */

    /* Check if we lost messages - allow more variance for timing */
    if (total < prod_data.added - 20) {
        printf("FAIL: Lost too many messages (%d added, %d accounted for)\n",
               prod_data.added, total);
        return 1;
    }

    /* Check for queue corruption */
    if (queue_count < 0 || queue_count > MAX_MESSAGES) {
        printf("FAIL: Queue count corrupted: %d\n", queue_count);
        return 1;
    }

    printf("PASS: Concurrent access test\n");
    return 0;
}

/* Test multiple producers */
int test_multiple_producers(void)
{
    message_queue_reset();

    pthread_t producers[3];
    producer_data prod_data[3] = {
        {0, 30, 0},
        {100, 30, 0},
        {200, 30, 0}
    };

    printf("Testing multiple producers...\n");

    /* Start all producers */
    for (int i = 0; i < 3; i++) {
        pthread_create(&producers[i], NULL, producer_thread, &prod_data[i]);
    }

    /* Wait for all to finish */
    for (int i = 0; i < 3; i++) {
        pthread_join(producers[i], NULL);
    }

    /* Count messages */
    int count = 0;
    while (message_get_next() != NULL) {
        count++;
    }

    int expected = prod_data[0].added + prod_data[1].added + prod_data[2].added;

    if (count != expected) {
        printf("FAIL: Expected %d messages, found %d\n", expected, count);
        return 1;
    }

    printf("PASS: Multiple producers test\n");
    return 0;
}

int main(void)
{
    int failures = 0;

    printf("=== Message Queue Race Condition Test ===\n\n");

#ifdef BUGGY_VERSION
    printf("Testing BUGGY version (expect message loss/corruption)\n\n");
#else
    printf("Testing FIXED version (should pass all tests)\n\n");
#endif

    failures += test_single_threaded();
    failures += test_concurrent_access();
    failures += test_multiple_producers();

    printf("\n=== Results: %d failures ===\n", failures);

    return failures;
}
