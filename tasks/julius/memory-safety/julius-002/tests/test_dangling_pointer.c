/**
 * Test for dangling pointer bug in game_file_write_mission_saved_game (julius-002)
 *
 * This test validates that the localized_filename variable is declared in the
 * correct scope. The bug is that localized_filename is declared INSIDE the
 * if (locale_translate_rank_autosaves()) block, but the 'filename' pointer
 * that references it is used AFTER the block - a classic dangling pointer.
 *
 * Test approach: Static analysis of the actual file.c source code.
 * This checks that the fix has been applied by verifying the variable
 * declaration appears BEFORE the if statement, not inside it.
 *
 * Test validation:
 * - On FIXED code: localized_filename declared before if → test passes
 * - On BUGGY code: localized_filename declared inside if → test fails
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE_LEN 512
#define FILE_PATH_ENV "JULIUS_SRC"

/**
 * Find game_file_write_mission_saved_game function and analyze variable scope
 *
 * Returns:
 *   1 if localized_filename is declared BEFORE the if block (FIXED)
 *   0 if localized_filename is declared INSIDE the if block (BUGGY)
 *  -1 on error
 */
static int analyze_file_c(const char *julius_src)
{
    char filepath[1024];
    snprintf(filepath, sizeof(filepath), "%s/game/file.c", julius_src);

    FILE *f = fopen(filepath, "r");
    if (!f) {
        fprintf(stderr, "Error: Cannot open %s\n", filepath);
        return -1;
    }

    char line[MAX_LINE_LEN];
    int in_function = 0;
    int brace_depth = 0;
    int found_localized_decl = 0;
    int found_if_locale = 0;
    int decl_before_if = 0;
    int line_num = 0;

    /*
     * We're looking for this pattern in game_file_write_mission_saved_game():
     *
     * FIXED (correct):
     *   const char *filename = MISSION_SAVED_GAMES[rank];
     *   char localized_filename[FILE_NAME_MAX];  <- BEFORE if
     *   if (locale_translate_rank_autosaves()) {
     *       encoding_to_utf8(...);
     *       ...
     *   }
     *
     * BUGGY (incorrect):
     *   const char *filename = MISSION_SAVED_GAMES[rank];
     *   if (locale_translate_rank_autosaves()) {
     *       char localized_filename[FILE_NAME_MAX];  <- INSIDE if
     *       encoding_to_utf8(...);
     *       ...
     *   }
     */

    while (fgets(line, MAX_LINE_LEN, f)) {
        line_num++;

        /* Look for the function start */
        if (strstr(line, "game_file_write_mission_saved_game") &&
            strstr(line, "void")) {
            in_function = 1;
            brace_depth = 0;
            found_localized_decl = 0;
            found_if_locale = 0;
            continue;
        }

        if (!in_function) continue;

        /* Track brace depth */
        for (char *p = line; *p; p++) {
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

        if (!in_function) break;

        /* Look for localized_filename declaration */
        if (strstr(line, "char") && strstr(line, "localized_filename") &&
            strstr(line, "[")) {
            found_localized_decl = 1;
            /*
             * Key check: Is this declaration at function scope (brace_depth == 1)
             * or inside the if block (brace_depth >= 2)?
             *
             * At function scope = FIXED
             * Inside if block = BUGGY
             */
            if (brace_depth == 1) {
                decl_before_if = 1;
                printf("  Found localized_filename declaration at function scope (line %d)\n", line_num);
            } else {
                decl_before_if = 0;
                printf("  Found localized_filename declaration inside nested block (line %d, depth %d)\n",
                       line_num, brace_depth);
            }
        }

        /* Look for the if statement */
        if (strstr(line, "locale_translate_rank_autosaves")) {
            found_if_locale = 1;
        }
    }

    fclose(f);

    if (!found_localized_decl) {
        printf("  Warning: localized_filename declaration not found\n");
        return -1;
    }

    return decl_before_if;
}

/**
 * Test 1: Verify variable scope is correct
 */
static int test_variable_scope(const char *julius_src)
{
    printf("Test: localized_filename declared at correct scope...\n");

    int result = analyze_file_c(julius_src);

    if (result == -1) {
        printf("  ERROR: Could not analyze file.c\n");
        return 0;
    }

    if (result == 1) {
        printf("  PASS: Variable declared before if block (no dangling pointer)\n");
        return 1;
    } else {
        printf("  FAIL: Variable declared inside if block (dangling pointer bug!)\n");
        printf("  The localized_filename array goes out of scope after the if block,\n");
        printf("  but 'filename' still points to it - undefined behavior!\n");
        return 0;
    }
}

/**
 * Test 2: Verify the function exists and has expected structure
 */
static int test_function_exists(const char *julius_src)
{
    printf("Test: game_file_write_mission_saved_game function exists...\n");

    char filepath[1024];
    snprintf(filepath, sizeof(filepath), "%s/game/file.c", julius_src);

    FILE *f = fopen(filepath, "r");
    if (!f) {
        printf("  FAIL: Cannot open %s\n", filepath);
        return 0;
    }

    char line[MAX_LINE_LEN];
    int found_function = 0;
    int found_mission_saves = 0;
    int found_locale_check = 0;

    while (fgets(line, MAX_LINE_LEN, f)) {
        if (strstr(line, "game_file_write_mission_saved_game")) {
            found_function = 1;
        }
        if (strstr(line, "MISSION_SAVED_GAMES")) {
            found_mission_saves = 1;
        }
        if (strstr(line, "locale_translate_rank_autosaves")) {
            found_locale_check = 1;
        }
    }

    fclose(f);

    if (!found_function) {
        printf("  FAIL: Function not found\n");
        return 0;
    }
    if (!found_mission_saves) {
        printf("  FAIL: MISSION_SAVED_GAMES not found\n");
        return 0;
    }
    if (!found_locale_check) {
        printf("  FAIL: locale_translate_rank_autosaves not found\n");
        return 0;
    }

    printf("  PASS: Function has expected structure\n");
    return 1;
}

int main(void)
{
    int passed = 0;
    int total = 2;

    printf("=== Dangling Pointer Test Suite ===\n");
    printf("Testing localized_filename scope in game/file.c\n");
    printf("Bug: Variable declared inside if block, used after block exits\n\n");

    /* Get Julius source path from environment */
    const char *julius_src = getenv(FILE_PATH_ENV);
    if (!julius_src) {
        /* Try default location relative to test */
        julius_src = ".";
    }

    printf("Julius source: %s\n\n", julius_src);

    if (test_function_exists(julius_src)) {
        passed++;
    }
    printf("\n");

    if (test_variable_scope(julius_src)) {
        passed++;
    }

    printf("\n=== Test Results ===\n");
    printf("Results: %d/%d tests passed\n", passed, total);

    return (passed == total) ? 0 : 1;
}
