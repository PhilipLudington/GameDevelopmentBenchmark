/**
 * Stub implementation for julius-042 caravan follower index test
 *
 * This provides mock caravan follower management for testing index handling.
 */

#include <stdio.h>
#include <string.h>

#define MAX_CARAVAN_FOLLOWERS 4

/* Caravan follower structure */
typedef struct {
    int figure_id;
    int x, y;
    int carrying_resource;
    int was_updated;  /* Flag to track if this follower was processed */
} caravan_follower;

/* Static follower array */
static caravan_follower followers[MAX_CARAVAN_FOLLOWERS];
static int num_followers = 0;

/* Update a single follower (sets the was_updated flag) */
static void update_follower_position(caravan_follower *f)
{
    f->was_updated = 1;  /* Mark as updated */
    f->x += 1;  /* Simulated movement */
    f->y += 1;
}

/* Add a new follower */
void add_follower(int figure_id)
{
    if (num_followers >= MAX_CARAVAN_FOLLOWERS) return;
    followers[num_followers].figure_id = figure_id;
    followers[num_followers].x = 0;
    followers[num_followers].y = 0;
    followers[num_followers].was_updated = 0;
    num_followers++;
}

/**
 * Update all followers
 *
 * BUGGY version: Starts at i=1, skipping first follower
 * FIXED version: Starts at i=0, updating all followers
 */
void update_all_followers(void)
{
    /* FIXED: Start at 0 to include first follower */
    for (int i = 0; i < num_followers; i++) {
        update_follower_position(&followers[i]);
    }
}

/* Get number of followers */
int get_num_followers(void)
{
    return num_followers;
}

/* Check if a specific follower was updated */
int was_follower_updated(int index)
{
    if (index < 0 || index >= num_followers) {
        return -1;  /* Invalid index */
    }
    return followers[index].was_updated;
}

/* Get follower by index */
caravan_follower *get_follower(int index)
{
    if (index < 0 || index >= num_followers) {
        return NULL;
    }
    return &followers[index];
}

/* Clear all followers (reset for next test) */
void clear_followers(void)
{
    memset(followers, 0, sizeof(followers));
    num_followers = 0;
}

/* Count how many followers were updated */
int count_updated_followers(void)
{
    int count = 0;
    for (int i = 0; i < num_followers; i++) {
        if (followers[i].was_updated) {
            count++;
        }
    }
    return count;
}

/* Reset update flags (but keep followers) */
void reset_update_flags(void)
{
    for (int i = 0; i < num_followers; i++) {
        followers[i].was_updated = 0;
    }
}
