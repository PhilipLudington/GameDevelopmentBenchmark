/*
 * Stub implementations for julius-014 sound double-free test
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

/* Stub for sound device functions */
int sound_device_init(void)
{
    return 1;
}

void sound_device_shutdown(void)
{
}
