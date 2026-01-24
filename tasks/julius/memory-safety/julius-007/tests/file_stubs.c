/**
 * Stub implementation for julius-007 buffer overflow test
 *
 * This provides the file_construct_path function for testing.
 * The actual Julius source is verified via static analysis.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILE_MAX_PATH_LENGTH 260

/**
 * BUGGY version: No bounds checking - will overflow if dir + filename > 260
 *
 * char *file_construct_path(const char *dir, const char *filename)
 * {
 *     if (!dir || !filename) return NULL;
 *     char *buffer = malloc(FILE_MAX_PATH_LENGTH);
 *     if (!buffer) return NULL;
 *     strcpy(buffer, dir);
 *     strcat(buffer, "/");
 *     strcat(buffer, filename);
 *     return buffer;
 * }
 *
 * FIXED version: Checks bounds before copying
 */
char *file_construct_path(const char *dir, const char *filename)
{
    if (!dir || !filename) return NULL;

    size_t dir_len = strlen(dir);
    size_t file_len = strlen(filename);
    size_t total_len = dir_len + 1 + file_len; /* dir + '/' + filename */

    /* Check if path would exceed buffer - FIXED: added bounds check */
    if (total_len >= FILE_MAX_PATH_LENGTH) {
        return NULL; /* Reject oversized path */
    }

    char *buffer = malloc(FILE_MAX_PATH_LENGTH);
    if (!buffer) return NULL;

    strcpy(buffer, dir);
    strcat(buffer, "/");
    strcat(buffer, filename);
    return buffer;
}
