/**
 * Stub implementation for julius-041 formation rank overflow test
 *
 * This provides mock formation rank management for testing bounds checking.
 */

#include <stdio.h>
#include <string.h>

#define MAX_FORMATION_RANKS 16

/* Formation rank entry */
typedef struct {
    int formation_id;
    int rank;
    int position;
} formation_rank_entry;

/* Static array with fixed size */
static formation_rank_entry rank_assignments[MAX_FORMATION_RANKS];
static int num_ranks = 0;

/* Calculate position (simple mock) */
static int calculate_position(int rank)
{
    return rank * 10;
}

/**
 * Assign formation to a rank
 *
 * BUGGY version: No bounds check - writes past array end
 * FIXED version: Checks bounds before writing
 *
 * Returns: 1 on success, 0 if array full
 */
int assign_formation_rank(int formation_id, int rank)
{
    /* FIXED: Check array bounds */
    if (num_ranks >= MAX_FORMATION_RANKS) {
        return 0;  /* Array full */
    }

    rank_assignments[num_ranks].formation_id = formation_id;
    rank_assignments[num_ranks].rank = rank;
    rank_assignments[num_ranks].position = calculate_position(rank);
    num_ranks++;

    return 1;  /* Success */
}

/* Get current number of ranks */
int get_num_ranks(void)
{
    return num_ranks;
}

/* Get max ranks constant */
int get_max_ranks(void)
{
    return MAX_FORMATION_RANKS;
}

/* Get rank entry by index */
formation_rank_entry *get_rank_entry(int index)
{
    if (index < 0 || index >= num_ranks) {
        return NULL;
    }
    return &rank_assignments[index];
}

/* Clear all rank assignments */
void clear_formation_ranks(void)
{
    memset(rank_assignments, 0, sizeof(rank_assignments));
    num_ranks = 0;
}
