/*
 * Stub implementations for julius-015 building use-after-free test
 */

#include <stdio.h>

/* Stub for core/log.h */
void log_info(const char *msg, const char *param1, int param2)
{
    (void)msg;
    (void)param1;
    (void)param2;
}

void log_error(const char *msg, const char *param1, int param2)
{
    (void)msg;
    (void)param1;
    (void)param2;
}

/* Stub for map functions */
int map_building_at(int grid_offset)
{
    (void)grid_offset;
    return 0;
}

void map_building_set(int grid_offset, int building_id)
{
    (void)grid_offset;
    (void)building_id;
}
