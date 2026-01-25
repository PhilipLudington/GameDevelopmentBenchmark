/*
 * Test for julius-011: Stack buffer overflow in translation string copy
 *
 * This test verifies that the translation system properly handles long strings
 * without causing a stack buffer overflow.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Simulate the buggy translation system */
#define BUFFER_SIZE 64
#define MAX_OUTPUT 256

/* Mock translation storage */
static const char *mock_translations[10];
static int num_translations = 0;

/* Set up a mock translation for testing */
void set_mock_translation(int key, const char *text)
{
    if (key >= 0 && key < 10) {
        mock_translations[key] = text;
        if (key >= num_translations) {
            num_translations = key + 1;
        }
    }
}

/* Mock translation lookup */
const char *translation_for_key(int key)
{
    if (key >= 0 && key < num_translations && mock_translations[key]) {
        return mock_translations[key];
    }
    return "";
}

/*
 * This is the function being tested - simulates the buggy/fixed code
 * from src/translation/translation.c
 */
static void format_translated_string(int key, char *output, int max_len)
{
    char buffer[BUFFER_SIZE];
    const char *translation = translation_for_key(key);

#ifdef BUGGY_VERSION
    /* BUG: Unbounded copy can overflow the stack buffer */
    strcpy(buffer, translation);
#else
    /* FIXED: Use bounded copy */
    snprintf(buffer, sizeof(buffer), "%s", translation);
#endif

    /* Apply formatting transformation */
    if (buffer[0] != '\0') {
        snprintf(output, max_len, "[%s]", buffer);
    } else {
        output[0] = '\0';
    }
}

/* Test with normal-length string */
int test_normal_string(void)
{
    char output[MAX_OUTPUT];

    set_mock_translation(0, "Hello World");
    format_translated_string(0, output, MAX_OUTPUT);

    if (strcmp(output, "[Hello World]") != 0) {
        printf("FAIL: Normal string test - got '%s'\n", output);
        return 1;
    }

    printf("PASS: Normal string test\n");
    return 0;
}

/* Test with string exactly at buffer limit */
int test_boundary_string(void)
{
    char output[MAX_OUTPUT];
    /* Create a string exactly 63 chars (+ null = 64) */
    char boundary_str[64];
    memset(boundary_str, 'A', 63);
    boundary_str[63] = '\0';

    set_mock_translation(1, boundary_str);
    format_translated_string(1, output, MAX_OUTPUT);

    printf("PASS: Boundary string test (63 chars)\n");
    return 0;
}

/* Test with string that exceeds buffer - triggers overflow if buggy */
int test_overflow_string(void)
{
    char output[MAX_OUTPUT];

    /* Create a string of 100 characters - will overflow 64-byte buffer */
    char long_str[101];
    memset(long_str, 'X', 100);
    long_str[100] = '\0';

    set_mock_translation(2, long_str);

    printf("Testing with %zu character string (buffer is %d)...\n",
           strlen(long_str), BUFFER_SIZE);

    /* This call will trigger stack-buffer-overflow if buggy */
    format_translated_string(2, output, MAX_OUTPUT);

    /* If we get here without ASan abort, the fix is working */
    printf("PASS: Long string test (100 chars handled safely)\n");
    return 0;
}

/* Test with empty string */
int test_empty_string(void)
{
    char output[MAX_OUTPUT] = "initial";

    set_mock_translation(3, "");
    format_translated_string(3, output, MAX_OUTPUT);

    if (output[0] != '\0') {
        printf("FAIL: Empty string test - expected empty output\n");
        return 1;
    }

    printf("PASS: Empty string test\n");
    return 0;
}

/* Test with very long string (stress test) */
int test_very_long_string(void)
{
    char output[MAX_OUTPUT];

    /* Create a very long string - 200 characters */
    char *very_long = malloc(201);
    if (!very_long) {
        printf("FAIL: Memory allocation failed\n");
        return 1;
    }
    memset(very_long, 'Z', 200);
    very_long[200] = '\0';

    set_mock_translation(4, very_long);

    /* This should not crash with proper bounds checking */
    format_translated_string(4, output, MAX_OUTPUT);

    free(very_long);

    printf("PASS: Very long string test (200 chars handled safely)\n");
    return 0;
}

int main(void)
{
    int failures = 0;

    printf("=== Translation Buffer Overflow Test ===\n\n");

#ifdef BUGGY_VERSION
    printf("Testing BUGGY version (expect ASan to catch overflow)\n\n");
#else
    printf("Testing FIXED version (should pass all tests)\n\n");
#endif

    failures += test_normal_string();
    failures += test_boundary_string();
    failures += test_overflow_string();
    failures += test_empty_string();
    failures += test_very_long_string();

    printf("\n=== Results: %d failures ===\n", failures);

    return failures;
}
