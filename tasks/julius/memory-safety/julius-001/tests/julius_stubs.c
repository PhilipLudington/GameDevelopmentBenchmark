/**
 * Minimal stubs for Julius dependencies needed by smacker.c
 *
 * These stubs allow smacker.c to compile and run without the full Julius build.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Stub for core/file.h - file_close */
void file_close(FILE *fp)
{
    if (fp) {
        fclose(fp);
    }
}

/* Stub for core/log.h - log_error */
void log_error(const char *msg, int a, int b)
{
    /* Suppress log output during tests */
    (void)msg;
    (void)a;
    (void)b;
}

/* Stub for core/log.h - log_info */
void log_info(const char *msg, int a, int b)
{
    /* Suppress log output during tests */
    (void)msg;
    (void)a;
    (void)b;
}

/* Stub for core/memory.h - clear_malloc (equivalent to calloc) */
void *clear_malloc(size_t size)
{
    return calloc(1, size);
}
