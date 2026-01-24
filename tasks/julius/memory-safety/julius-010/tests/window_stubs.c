/**
 * Stub implementation for julius-010 use-after-free test
 *
 * This provides mock UI callback functions for testing.
 * The actual Julius source is verified via static analysis.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void *pending_callback_data = NULL;

/**
 * BUGGY version: Stores pointer and immediately frees, causing use-after-free
 *
 * void ui_trigger_callback_unsafe(void *data)
 * {
 *     pending_callback_data = data;
 *     free(data);  // Frees but keeps reference - UAF!
 * }
 *
 * FIXED version: Either don't free, or set pointer to NULL after free
 */
void ui_trigger_callback_unsafe(void *data)
{
    /* FIXED: Either keep ownership of data, or clear the pointer */
    /* Option 1: Take ownership, don't free yet */
    pending_callback_data = data;
    /* Don't free here - caller transfers ownership */
}

void *ui_get_callback_data(void)
{
    return pending_callback_data;
}

/* Cleanup function for proper memory management */
void ui_cleanup_callback_data(void)
{
    if (pending_callback_data) {
        free(pending_callback_data);
        pending_callback_data = NULL;
    }
}
