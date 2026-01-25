/**
 * Stub implementation for julius-039 waypoint deletion test
 *
 * This provides mock route data management for testing array compaction.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_WAYPOINTS 20

/* Waypoint structure */
typedef struct {
    int x;
    int y;
    int id;  /* Unique identifier for testing */
} waypoint;

/* Route data structure */
typedef struct {
    waypoint waypoints[MAX_WAYPOINTS];
    int num_waypoints;
} route_data;

/**
 * Delete waypoint from route at given index
 *
 * BUGGY version: Just decrements count, doesn't shift data
 * FIXED version: Shifts remaining elements and decrements count
 */
void route_delete_waypoint(route_data *route, int index)
{
    if (index < 0 || index >= route->num_waypoints) {
        return;
    }

    /* FIXED: Shift all waypoints after index down by one */
    for (int i = index; i < route->num_waypoints - 1; i++) {
        route->waypoints[i] = route->waypoints[i + 1];
    }

    route->num_waypoints--;

    /* Clear the now-unused last slot */
    memset(&route->waypoints[route->num_waypoints], 0, sizeof(waypoint));
}

/* Initialize a test route with sequential waypoints */
void route_init_test(route_data *route, int count)
{
    memset(route, 0, sizeof(route_data));
    route->num_waypoints = count;

    for (int i = 0; i < count; i++) {
        route->waypoints[i].x = i * 10;
        route->waypoints[i].y = i * 10;
        route->waypoints[i].id = i + 1;  /* IDs: 1, 2, 3, ... */
    }
}

/* Get waypoint at index */
waypoint *route_get_waypoint(route_data *route, int index)
{
    if (index < 0 || index >= route->num_waypoints) {
        return NULL;
    }
    return &route->waypoints[index];
}

/* Get number of waypoints */
int route_get_count(route_data *route)
{
    return route->num_waypoints;
}

/* Add a waypoint at the end */
void route_add_waypoint(route_data *route, int x, int y, int id)
{
    if (route->num_waypoints >= MAX_WAYPOINTS) {
        return;
    }
    route->waypoints[route->num_waypoints].x = x;
    route->waypoints[route->num_waypoints].y = y;
    route->waypoints[route->num_waypoints].id = id;
    route->num_waypoints++;
}
