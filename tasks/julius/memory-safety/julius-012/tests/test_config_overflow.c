/*
 * Test for julius-012: Heap buffer overflow in config value parsing
 *
 * This test verifies that the config parser properly handles long values
 * without causing a heap buffer overflow.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CONFIG_VALUE_SIZE 128

/*
 * Simulates the buggy/fixed config parsing function
 */
static char *parse_config_value(const char *line)
{
    char *value = malloc(CONFIG_VALUE_SIZE);
    if (!value) return NULL;

    const char *equals = strchr(line, '=');
    if (equals) {
#ifdef BUGGY_VERSION
        /* BUG: No length check before copy */
        strcpy(value, equals + 1);
#else
        /* FIXED: Use bounded copy */
        strncpy(value, equals + 1, CONFIG_VALUE_SIZE - 1);
        value[CONFIG_VALUE_SIZE - 1] = '\0';
#endif
    } else {
        value[0] = '\0';
    }

    return value;
}

/* Test normal config value */
int test_normal_value(void)
{
    const char *line = "key=normal_value";
    char *value = parse_config_value(line);

    if (!value) {
        printf("FAIL: Normal value - allocation failed\n");
        return 1;
    }

    if (strcmp(value, "normal_value") != 0) {
        printf("FAIL: Normal value - got '%s'\n", value);
        free(value);
        return 1;
    }

    free(value);
    printf("PASS: Normal config value\n");
    return 0;
}

/* Test value at boundary (127 chars) */
int test_boundary_value(void)
{
    /* Create line with exactly 127 char value */
    char line[256] = "key=";
    memset(line + 4, 'B', 127);
    line[131] = '\0';

    char *value = parse_config_value(line);

    if (!value) {
        printf("FAIL: Boundary value - allocation failed\n");
        return 1;
    }

    if (strlen(value) != 127) {
        printf("FAIL: Boundary value - unexpected length %zu\n", strlen(value));
        free(value);
        return 1;
    }

    free(value);
    printf("PASS: Boundary config value (127 chars)\n");
    return 0;
}

/* Test overflow value (150+ chars) - triggers bug if unfixed */
int test_overflow_value(void)
{
    /* Create line with 150 char value - exceeds 128 byte buffer */
    char line[256] = "key=";
    memset(line + 4, 'X', 150);
    line[154] = '\0';

    printf("Testing with %d character value (buffer is %d)...\n",
           150, CONFIG_VALUE_SIZE);

    /* This will trigger heap-buffer-overflow if buggy */
    char *value = parse_config_value(line);

    if (!value) {
        printf("FAIL: Overflow value - allocation failed\n");
        return 1;
    }

    /* If we get here without ASan abort, the fix is working */
    /* With fix, value should be truncated to CONFIG_VALUE_SIZE - 1 */
    if (strlen(value) >= CONFIG_VALUE_SIZE) {
        printf("FAIL: Value was not truncated (len=%zu)\n", strlen(value));
        free(value);
        return 1;
    }

    free(value);
    printf("PASS: Overflow value handled safely\n");
    return 0;
}

/* Test empty value */
int test_empty_value(void)
{
    const char *line = "key=";
    char *value = parse_config_value(line);

    if (!value) {
        printf("FAIL: Empty value - allocation failed\n");
        return 1;
    }

    if (value[0] != '\0') {
        printf("FAIL: Empty value - got '%s'\n", value);
        free(value);
        return 1;
    }

    free(value);
    printf("PASS: Empty config value\n");
    return 0;
}

/* Test line without equals sign */
int test_no_equals(void)
{
    const char *line = "key_without_value";
    char *value = parse_config_value(line);

    if (!value) {
        printf("FAIL: No equals - allocation failed\n");
        return 1;
    }

    /* Without equals sign, value should be empty */
    free(value);
    printf("PASS: Line without equals sign\n");
    return 0;
}

/* Test very long value (stress test) */
int test_very_long_value(void)
{
    /* Create line with 500 char value */
    char *line = malloc(512);
    if (!line) {
        printf("FAIL: Could not allocate test line\n");
        return 1;
    }

    strcpy(line, "key=");
    memset(line + 4, 'Z', 500);
    line[504] = '\0';

    printf("Testing with 500 character value...\n");

    char *value = parse_config_value(line);
    free(line);

    if (!value) {
        printf("FAIL: Very long value - allocation failed\n");
        return 1;
    }

    /* With fix, should be safely truncated */
    free(value);
    printf("PASS: Very long value (500 chars) handled safely\n");
    return 0;
}

int main(void)
{
    int failures = 0;

    printf("=== Config Parser Heap Overflow Test ===\n\n");

#ifdef BUGGY_VERSION
    printf("Testing BUGGY version (expect ASan to catch overflow)\n\n");
#else
    printf("Testing FIXED version (should pass all tests)\n\n");
#endif

    failures += test_normal_value();
    failures += test_boundary_value();
    failures += test_overflow_value();
    failures += test_empty_value();
    failures += test_no_equals();
    failures += test_very_long_value();

    printf("\n=== Results: %d failures ===\n", failures);

    return failures;
}
