/*
 * Stub implementations for julius-013 figure uninitialized test
 */

#include <stdio.h>
#include <stdlib.h>

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
int map_grid_offset(int x, int y)
{
    return y * 162 + x;
}
