/*
 * Stub implementations for julius-011 translation overflow test
 *
 * These stubs provide minimal implementations of Julius dependencies
 * needed to compile and run the test.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Stub for core/log.h - log_info */
void log_info(const char *msg, const char *param1, int param2)
{
    (void)msg;
    (void)param1;
    (void)param2;
}

/* Stub for core/log.h - log_error */
void log_error(const char *msg, const char *param1, int param2)
{
    (void)msg;
    (void)param1;
    (void)param2;
}

/* Stub for core/string.h - string_copy */
void string_copy(char *dst, const char *src, int maxlen)
{
    if (maxlen > 0) {
        strncpy(dst, src, maxlen - 1);
        dst[maxlen - 1] = '\0';
    }
}

/* Stub for core/string.h - string_length */
int string_length(const char *str)
{
    return str ? (int)strlen(str) : 0;
}
