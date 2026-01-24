/**
 * Test for tooltip trailing newline bug (julius-004)
 *
 * This test validates that the text layout algorithm checks the width boundary
 * BEFORE adding word_width, not AFTER. The bug causes extra newlines in tooltips.
 *
 * Test approach: Static analysis of the actual text.c source code.
 * This checks that the fix has been applied by verifying the condition pattern.
 *
 * Test validation:
 * - On FIXED code: checks "current_width + word_width >= box_width" BEFORE adding
 * - On BUGGY code: adds word_width first, then checks "current_width >= box_width"
 *
 * Key patterns to detect:
 *   BUGGY:  current_width += word_width;
 *           if (current_width >= box_width)
 *
 *   FIXED:  if (current_width + word_width >= box_width) {
 *               ...
 *           }
 *           current_width += word_width;
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE_LEN 512
#define FILE_PATH_ENV "JULIUS_SRC"

/**
 * Check if a line contains the fixed pattern: "current_width + word_width >= box_width"
 */
static int has_fixed_pattern(const char *line)
{
    /* Look for the pre-check pattern: current_width + word_width >= box_width */
    const char *p = strstr(line, "current_width");
    if (!p) return 0;

    p = strstr(p, "+");
    if (!p) return 0;

    p = strstr(p, "word_width");
    if (!p) return 0;

    p = strstr(p, ">=");
    if (!p) return 0;

    p = strstr(p, "box_width");
    if (!p) return 0;

    return 1;
}

/**
 * Check if a line contains the buggy pattern: "current_width >= box_width"
 * (without the + word_width pre-check)
 */
static int has_buggy_pattern(const char *line)
{
    /* Look for: current_width >= box_width (without + word_width) */
    const char *p = strstr(line, "current_width");
    if (!p) return 0;

    /* Check there's no "+" between current_width and >= */
    const char *ge = strstr(p, ">=");
    if (!ge) return 0;

    /* If there's a "+" between current_width and >=, it's the fixed pattern */
    const char *plus = strstr(p, "+");
    if (plus && plus < ge) {
        /* Has a + before >=, could be fixed pattern */
        return 0;
    }

    /* No + before >=, this is the buggy pattern */
    if (strstr(ge, "box_width")) {
        return 1;
    }

    return 0;
}

/**
 * Analyze text_draw_multiline function for the bug pattern
 *
 * Returns:
 *   1 if the FIXED pattern is found (check before add)
 *   0 if the BUGGY pattern is found (add before check)
 *  -1 on error
 */
static int analyze_text_draw_multiline(const char *julius_src)
{
    char filepath[1024];
    snprintf(filepath, sizeof(filepath), "%s/graphics/text.c", julius_src);

    FILE *f = fopen(filepath, "r");
    if (!f) {
        fprintf(stderr, "Error: Cannot open %s\n", filepath);
        return -1;
    }

    char line[MAX_LINE_LEN];
    int in_function = 0;
    int brace_depth = 0;
    int found_fixed_pattern = 0;
    int found_buggy_pattern = 0;
    int line_num = 0;

    while (fgets(line, MAX_LINE_LEN, f)) {
        line_num++;

        /* Look for the function start */
        if (strstr(line, "text_draw_multiline") && strstr(line, "(")) {
            /* Make sure it's the function definition, not a call */
            if (strstr(line, "int") || strstr(line, "void")) {
                in_function = 1;
                brace_depth = 0;
                continue;
            }
        }

        if (!in_function) continue;

        /* Track brace depth */
        for (const char *p = line; *p; p++) {
            if (*p == '{') brace_depth++;
            else if (*p == '}') {
                brace_depth--;
                if (brace_depth == 0) {
                    /* End of function */
                    in_function = 0;
                    break;
                }
            }
        }

        if (!in_function && brace_depth == 0) break;

        /* Check for the fixed pattern */
        if (has_fixed_pattern(line)) {
            found_fixed_pattern = 1;
            printf("  Found FIXED pattern at line %d: check before add\n", line_num);
            printf("    %s", line);
        }

        /* Check for the buggy pattern */
        if (has_buggy_pattern(line)) {
            found_buggy_pattern = 1;
            printf("  Found BUGGY pattern at line %d: check after add\n", line_num);
            printf("    %s", line);
        }
    }

    fclose(f);

    if (found_fixed_pattern && !found_buggy_pattern) {
        return 1;  /* Fixed */
    } else if (found_buggy_pattern) {
        return 0;  /* Buggy */
    }

    /* Neither pattern found - check if this is a valid function */
    printf("  Warning: Expected patterns not found in text_draw_multiline\n");
    return -1;
}

/**
 * Analyze text_measure_multiline function for the bug pattern
 * (Same bug exists in both functions)
 */
static int analyze_text_measure_multiline(const char *julius_src)
{
    char filepath[1024];
    snprintf(filepath, sizeof(filepath), "%s/graphics/text.c", julius_src);

    FILE *f = fopen(filepath, "r");
    if (!f) {
        fprintf(stderr, "Error: Cannot open %s\n", filepath);
        return -1;
    }

    char line[MAX_LINE_LEN];
    int in_function = 0;
    int brace_depth = 0;
    int found_fixed_pattern = 0;
    int found_buggy_pattern = 0;
    int line_num = 0;

    while (fgets(line, MAX_LINE_LEN, f)) {
        line_num++;

        /* Look for the function start */
        if (strstr(line, "text_measure_multiline") && strstr(line, "(")) {
            if (strstr(line, "int") || strstr(line, "void")) {
                in_function = 1;
                brace_depth = 0;
                continue;
            }
        }

        if (!in_function) continue;

        /* Track brace depth */
        for (const char *p = line; *p; p++) {
            if (*p == '{') brace_depth++;
            else if (*p == '}') {
                brace_depth--;
                if (brace_depth == 0) {
                    in_function = 0;
                    break;
                }
            }
        }

        if (!in_function && brace_depth == 0) break;

        if (has_fixed_pattern(line)) {
            found_fixed_pattern = 1;
            printf("  Found FIXED pattern at line %d: check before add\n", line_num);
            printf("    %s", line);
        }

        if (has_buggy_pattern(line)) {
            found_buggy_pattern = 1;
            printf("  Found BUGGY pattern at line %d: check after add\n", line_num);
            printf("    %s", line);
        }
    }

    fclose(f);

    if (found_fixed_pattern && !found_buggy_pattern) {
        return 1;
    } else if (found_buggy_pattern) {
        return 0;
    }

    printf("  Warning: Expected patterns not found in text_measure_multiline\n");
    return -1;
}

/**
 * Test 1: Verify text.c exists and has the expected functions
 */
static int test_file_exists(const char *julius_src)
{
    printf("Test: graphics/text.c exists with expected functions...\n");

    char filepath[1024];
    snprintf(filepath, sizeof(filepath), "%s/graphics/text.c", julius_src);

    FILE *f = fopen(filepath, "r");
    if (!f) {
        printf("  FAIL: Cannot open %s\n", filepath);
        return 0;
    }

    char line[MAX_LINE_LEN];
    int found_draw = 0;
    int found_measure = 0;
    int found_word_width = 0;

    while (fgets(line, MAX_LINE_LEN, f)) {
        if (strstr(line, "text_draw_multiline")) found_draw = 1;
        if (strstr(line, "text_measure_multiline")) found_measure = 1;
        if (strstr(line, "get_word_width") || strstr(line, "word_width")) {
            found_word_width = 1;
        }
    }

    fclose(f);

    if (!found_draw) {
        printf("  FAIL: text_draw_multiline not found\n");
        return 0;
    }
    if (!found_measure) {
        printf("  FAIL: text_measure_multiline not found\n");
        return 0;
    }
    if (!found_word_width) {
        printf("  FAIL: word_width handling not found\n");
        return 0;
    }

    printf("  PASS: File has expected structure\n");
    return 1;
}

/**
 * Test 2: Verify text_draw_multiline has fixed pattern
 */
static int test_draw_multiline_fixed(const char *julius_src)
{
    printf("Test: text_draw_multiline checks width BEFORE adding...\n");

    int result = analyze_text_draw_multiline(julius_src);

    if (result == -1) {
        printf("  ERROR: Could not analyze text_draw_multiline\n");
        return 0;
    }

    if (result == 1) {
        printf("  PASS: Width checked before adding (no trailing newline bug)\n");
        return 1;
    } else {
        printf("  FAIL: Width checked AFTER adding (trailing newline bug!)\n");
        printf("  The algorithm adds word_width first, then checks >= box_width.\n");
        printf("  This causes incorrect line breaks and extra trailing newlines.\n");
        return 0;
    }
}

/**
 * Test 3: Verify text_measure_multiline has fixed pattern
 */
static int test_measure_multiline_fixed(const char *julius_src)
{
    printf("Test: text_measure_multiline checks width BEFORE adding...\n");

    int result = analyze_text_measure_multiline(julius_src);

    if (result == -1) {
        printf("  ERROR: Could not analyze text_measure_multiline\n");
        return 0;
    }

    if (result == 1) {
        printf("  PASS: Width checked before adding (no trailing newline bug)\n");
        return 1;
    } else {
        printf("  FAIL: Width checked AFTER adding (trailing newline bug!)\n");
        printf("  The algorithm adds word_width first, then checks >= box_width.\n");
        printf("  This causes incorrect measurement and extra trailing newlines.\n");
        return 0;
    }
}

int main(void)
{
    int passed = 0;
    int total = 3;

    printf("=== Tooltip Trailing Newline Test Suite ===\n");
    printf("Testing text layout algorithm in graphics/text.c\n");
    printf("Bug: Width checked AFTER adding word, causing incorrect line breaks\n\n");

    /* Get Julius source path from environment */
    const char *julius_src = getenv(FILE_PATH_ENV);
    if (!julius_src) {
        julius_src = ".";
    }

    printf("Julius source: %s\n\n", julius_src);

    if (test_file_exists(julius_src)) {
        passed++;
    }
    printf("\n");

    if (test_draw_multiline_fixed(julius_src)) {
        passed++;
    }
    printf("\n");

    if (test_measure_multiline_fixed(julius_src)) {
        passed++;
    }

    printf("\n=== Test Results ===\n");
    printf("Results: %d/%d tests passed\n", passed, total);

    return (passed == total) ? 0 : 1;
}
