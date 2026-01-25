/*
 * Stub implementations for julius-016 resource underflow test
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

/* Stub for city data access */
void city_data_set_resource(int type, int amount)
{
    (void)type;
    (void)amount;
}
