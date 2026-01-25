/**
 * Stub implementation for julius-038 granary cart pathfinding test
 *
 * This provides mock route following functions for testing arrival detection.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Route status */
#define ROUTE_CONTINUE 0
#define ROUTE_ARRIVED 1

/* Waypoint structure */
typedef struct {
    int x;
    int y;
} route_waypoint;

/* Figure structure (simplified) */
typedef struct {
    int x;
    int y;
    int route_start_x;
    int route_start_y;
    int destination_x;
    int destination_y;
    int route_index;
    route_waypoint *route;
    int route_length;
} figure;

/* Mock route for testing */
static route_waypoint test_route[10];
static int test_route_length = 0;

/* Set up a test route */
void test_setup_route(figure *f, int start_x, int start_y, int dest_x, int dest_y)
{
    f->x = start_x;
    f->y = start_y;
    f->route_start_x = start_x;
    f->route_start_y = start_y;
    f->destination_x = dest_x;
    f->destination_y = dest_y;
    f->route_index = 0;

    /* Create simple route: start -> middle -> destination */
    test_route[0].x = start_x;
    test_route[0].y = start_y;
    test_route[1].x = (start_x + dest_x) / 2;
    test_route[1].y = (start_y + dest_y) / 2;
    test_route[2].x = dest_x;
    test_route[2].y = dest_y;
    test_route_length = 3;

    f->route = test_route;
    f->route_length = test_route_length;
}

/* Get next waypoint */
route_waypoint *get_next_waypoint(figure *f)
{
    if (f->route_index < f->route_length) {
        return &f->route[f->route_index++];
    }
    /* Loop back (for testing infinite loop detection) */
    f->route_index = 0;
    return &f->route[0];
}

/**
 * Follow route and check for arrival
 *
 * BUGGY version: Checks against source_x/source_y
 * FIXED version: Checks against dest_x/dest_y
 */
int figure_route_follow(figure *f)
{
    int source_x = f->route_start_x;
    int source_y = f->route_start_y;
    int dest_x = f->destination_x;
    int dest_y = f->destination_y;

    /* Suppress unused variable warnings */
    (void)source_x;
    (void)source_y;

    /* Move to next waypoint */
    route_waypoint *wp = get_next_waypoint(f);
    f->x = wp->x;
    f->y = wp->y;

    /* FIXED: Check against DESTINATION, not source */
    if (f->x == dest_x && f->y == dest_y) {
        return ROUTE_ARRIVED;
    }

    return ROUTE_CONTINUE;
}

/* Simulate following route until arrival or max steps */
int simulate_route_follow(figure *f, int max_steps)
{
    for (int step = 0; step < max_steps; step++) {
        int status = figure_route_follow(f);
        if (status == ROUTE_ARRIVED) {
            return step + 1;  /* Return steps taken to arrive */
        }
    }
    return -1;  /* Did not arrive (stuck in loop) */
}

/* Get current position */
void figure_get_position(figure *f, int *x, int *y)
{
    *x = f->x;
    *y = f->y;
}
