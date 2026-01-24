/**
 * Test for hotkey config ordering mismatch bug (julius-005)
 *
 * This test compiles against the actual Julius hotkey_config.c and verifies
 * that enum values correctly map to their expected string names in ini_keys[].
 *
 * The bug: In the ini_keys[] array, "build_clear_land" and "build_vacant_house"
 * are swapped relative to their enum order in hotkey_config.h.
 *
 * From the header (hotkey_config.h), the enum order is:
 *   HOTKEY_BUILD_CLEAR_LAND  = 11
 *   HOTKEY_BUILD_VACANT_HOUSE = 12
 *
 * So ini_keys[11] must be "build_clear_land" and ini_keys[12] must be
 * "build_vacant_house".
 *
 * Test validation:
 * - On FIXED code: ini_keys[11]="build_clear_land", ini_keys[12]="build_vacant_house"
 * - On BUGGY code: ini_keys[11]="build_vacant_house", ini_keys[12]="build_clear_land"
 *
 * Technique: We #include the .c file directly to access the static ini_keys[]
 * array for verification. This is a standard C testing pattern.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/*
 * IMPORTANT: Define all header guards FIRST to prevent Julius headers
 * from being included when we #include hotkey_config.c
 */
#define CORE_HOTKEY_CONFIG_H
#define CORE_FILE_H
#define CORE_LOG_H
#define GAME_SYSTEM_H
#define INPUT_HOTKEY_H
#define INPUT_KEYS_H

/*
 * Now provide stubs for all Julius dependencies that hotkey_config.c needs.
 */

/* Stub types for keys.h */
typedef int key_type;
typedef int key_modifier_type;

#define KEY_TYPE_NONE 0
#define KEY_TYPE_UP 1
#define KEY_TYPE_DOWN 2
#define KEY_TYPE_LEFT 3
#define KEY_TYPE_RIGHT 4
#define KEY_TYPE_SPACE 5
#define KEY_TYPE_P 6
#define KEY_TYPE_L 7
#define KEY_TYPE_LEFTBRACKET 8
#define KEY_TYPE_RIGHTBRACKET 9
#define KEY_TYPE_PAGEUP 10
#define KEY_TYPE_PAGEDOWN 11
#define KEY_TYPE_HOME 12
#define KEY_TYPE_END 13
#define KEY_TYPE_1 14
#define KEY_TYPE_2 15
#define KEY_TYPE_3 16
#define KEY_TYPE_4 17
#define KEY_TYPE_5 18
#define KEY_TYPE_6 19
#define KEY_TYPE_7 20
#define KEY_TYPE_8 21
#define KEY_TYPE_9 22
#define KEY_TYPE_0 23
#define KEY_TYPE_KP_0 24
#define KEY_TYPE_KP_1 25
#define KEY_TYPE_KP_2 26
#define KEY_TYPE_KP_3 27
#define KEY_TYPE_KP_4 28
#define KEY_TYPE_KP_5 29
#define KEY_TYPE_KP_6 30
#define KEY_TYPE_KP_7 31
#define KEY_TYPE_KP_8 32
#define KEY_TYPE_KP_9 33
#define KEY_TYPE_MINUS 34
#define KEY_TYPE_EQUALS 35
#define KEY_TYPE_W 36
#define KEY_TYPE_F 37
#define KEY_TYPE_D 38
#define KEY_TYPE_C 39
#define KEY_TYPE_T 40
#define KEY_TYPE_A 41
#define KEY_TYPE_O 42
#define KEY_TYPE_S 43
#define KEY_TYPE_F1 44
#define KEY_TYPE_F2 45
#define KEY_TYPE_F3 46
#define KEY_TYPE_F4 47
#define KEY_TYPE_F5 48
#define KEY_TYPE_F6 49
#define KEY_TYPE_F7 50
#define KEY_TYPE_F8 51
#define KEY_TYPE_F9 52
#define KEY_TYPE_F12 53
#define KEY_TYPE_ENTER 54

#define KEY_MOD_NONE 0
#define KEY_MOD_CTRL 1
#define KEY_MOD_ALT 2

/* Stub for input/keys.h functions */
static int key_combination_from_name(const char *name, key_type *key, key_modifier_type *mod)
{
    (void)name; (void)key; (void)mod;
    return 0;
}

static const char *key_combination_name(key_type key, key_modifier_type mod)
{
    (void)key; (void)mod;
    return "stub";
}

/* Stub for core/file.h */
static FILE *file_open(const char *name, const char *mode)
{
    return fopen(name, mode);
}

static void file_close(FILE *fp)
{
    if (fp) fclose(fp);
}

/* Stub for core/log.h */
static void log_info(const char *msg, const char *param_str, int param_int)
{
    (void)msg; (void)param_str; (void)param_int;
}

static void log_error(const char *msg, const char *param_str, int param_int)
{
    (void)msg; (void)param_str; (void)param_int;
}

/* Stub for game/system.h */
static key_type system_keyboard_key_for_symbol(const char *symbol)
{
    (void)symbol;
    return KEY_TYPE_NONE;
}

/* Stub for input/hotkey.h */
typedef struct {
    key_type key;
    key_modifier_type modifiers;
    int action;
} hotkey_mapping;

static void hotkey_install_mapping(hotkey_mapping *mappings, int count)
{
    (void)mappings; (void)count;
}

/* Hotkey action enum - must match hotkey_config.h exactly */
typedef enum {
    HOTKEY_ARROW_UP,                    /* 0 */
    HOTKEY_ARROW_DOWN,                  /* 1 */
    HOTKEY_ARROW_LEFT,                  /* 2 */
    HOTKEY_ARROW_RIGHT,                 /* 3 */
    HOTKEY_TOGGLE_PAUSE,                /* 4 */
    HOTKEY_TOGGLE_OVERLAY,              /* 5 */
    HOTKEY_CYCLE_LEGION,                /* 6 */
    HOTKEY_INCREASE_GAME_SPEED,         /* 7 */
    HOTKEY_DECREASE_GAME_SPEED,         /* 8 */
    HOTKEY_ROTATE_MAP_LEFT,             /* 9 */
    HOTKEY_ROTATE_MAP_RIGHT,            /* 10 */
    HOTKEY_BUILD_CLEAR_LAND,            /* 11 - THE KEY INDEX */
    HOTKEY_BUILD_VACANT_HOUSE,          /* 12 - THE KEY INDEX */
    HOTKEY_BUILD_ROAD,                  /* 13 */
    HOTKEY_BUILD_PLAZA,
    HOTKEY_BUILD_GARDENS,
    HOTKEY_BUILD_PREFECTURE,
    HOTKEY_BUILD_ENGINEERS_POST,
    HOTKEY_BUILD_DOCTOR,
    HOTKEY_BUILD_GRANARY,
    HOTKEY_BUILD_WAREHOUSE,
    HOTKEY_BUILD_MARKET,
    HOTKEY_BUILD_WALL,
    HOTKEY_BUILD_GATEHOUSE,
    HOTKEY_BUILD_RESERVOIR,
    HOTKEY_BUILD_AQUEDUCT,
    HOTKEY_BUILD_FOUNTAIN,
    HOTKEY_SHOW_ADVISOR_LABOR,
    HOTKEY_SHOW_ADVISOR_MILITARY,
    HOTKEY_SHOW_ADVISOR_IMPERIAL,
    HOTKEY_SHOW_ADVISOR_RATINGS,
    HOTKEY_SHOW_ADVISOR_TRADE,
    HOTKEY_SHOW_ADVISOR_POPULATION,
    HOTKEY_SHOW_ADVISOR_HEALTH,
    HOTKEY_SHOW_ADVISOR_EDUCATION,
    HOTKEY_SHOW_ADVISOR_ENTERTAINMENT,
    HOTKEY_SHOW_ADVISOR_RELIGION,
    HOTKEY_SHOW_ADVISOR_FINANCIAL,
    HOTKEY_SHOW_ADVISOR_CHIEF,
    HOTKEY_SHOW_OVERLAY_WATER,
    HOTKEY_SHOW_OVERLAY_FIRE,
    HOTKEY_SHOW_OVERLAY_DAMAGE,
    HOTKEY_SHOW_OVERLAY_CRIME,
    HOTKEY_SHOW_OVERLAY_PROBLEMS,
    HOTKEY_EDITOR_TOGGLE_BATTLE_INFO,
    HOTKEY_LOAD_FILE,
    HOTKEY_SAVE_FILE,
    HOTKEY_GO_TO_BOOKMARK_1,
    HOTKEY_GO_TO_BOOKMARK_2,
    HOTKEY_GO_TO_BOOKMARK_3,
    HOTKEY_GO_TO_BOOKMARK_4,
    HOTKEY_SET_BOOKMARK_1,
    HOTKEY_SET_BOOKMARK_2,
    HOTKEY_SET_BOOKMARK_3,
    HOTKEY_SET_BOOKMARK_4,
    HOTKEY_CENTER_WINDOW,
    HOTKEY_TOGGLE_FULLSCREEN,
    HOTKEY_RESIZE_TO_640,
    HOTKEY_RESIZE_TO_800,
    HOTKEY_RESIZE_TO_1024,
    HOTKEY_SAVE_SCREENSHOT,
    HOTKEY_SAVE_CITY_SCREENSHOT,
    HOTKEY_BUILD_CLONE,
    HOTKEY_MAX_ITEMS
} hotkey_action;

/*
 * Now include the implementation file to get access to the static ini_keys[].
 * The guards above prevent Julius headers from being processed.
 */
#include "core/hotkey_config.c"

/*
 * Test 1: Verify HOTKEY_BUILD_CLEAR_LAND maps to "build_clear_land"
 *
 * This is the core bug check. On buggy code, ini_keys[11] is "build_vacant_house".
 */
static int test_clear_land_mapping(void)
{
    printf("Test: HOTKEY_BUILD_CLEAR_LAND maps to correct string...\n");
    printf("  HOTKEY_BUILD_CLEAR_LAND = %d\n", HOTKEY_BUILD_CLEAR_LAND);
    printf("  ini_keys[%d] = \"%s\"\n", HOTKEY_BUILD_CLEAR_LAND, ini_keys[HOTKEY_BUILD_CLEAR_LAND]);

    if (strcmp(ini_keys[HOTKEY_BUILD_CLEAR_LAND], "build_clear_land") != 0) {
        printf("  FAIL: Expected \"build_clear_land\", got \"%s\"\n",
               ini_keys[HOTKEY_BUILD_CLEAR_LAND]);
        return 0;
    }

    printf("  PASS: Correctly maps to \"build_clear_land\"\n");
    return 1;
}

/*
 * Test 2: Verify HOTKEY_BUILD_VACANT_HOUSE maps to "build_vacant_house"
 */
static int test_vacant_house_mapping(void)
{
    printf("Test: HOTKEY_BUILD_VACANT_HOUSE maps to correct string...\n");
    printf("  HOTKEY_BUILD_VACANT_HOUSE = %d\n", HOTKEY_BUILD_VACANT_HOUSE);
    printf("  ini_keys[%d] = \"%s\"\n", HOTKEY_BUILD_VACANT_HOUSE, ini_keys[HOTKEY_BUILD_VACANT_HOUSE]);

    if (strcmp(ini_keys[HOTKEY_BUILD_VACANT_HOUSE], "build_vacant_house") != 0) {
        printf("  FAIL: Expected \"build_vacant_house\", got \"%s\"\n",
               ini_keys[HOTKEY_BUILD_VACANT_HOUSE]);
        return 0;
    }

    printf("  PASS: Correctly maps to \"build_vacant_house\"\n");
    return 1;
}

/*
 * Test 3: Verify mappings are not swapped (cross-check)
 */
static int test_mapping_not_swapped(void)
{
    printf("Test: Mappings are not swapped...\n");

    const char *clear_land = ini_keys[HOTKEY_BUILD_CLEAR_LAND];
    const char *vacant_house = ini_keys[HOTKEY_BUILD_VACANT_HOUSE];

    printf("  ini_keys[CLEAR_LAND] = \"%s\"\n", clear_land);
    printf("  ini_keys[VACANT_HOUSE] = \"%s\"\n", vacant_house);

    /* Check for the specific bug: swapped strings */
    if (strcmp(clear_land, "build_vacant_house") == 0 &&
        strcmp(vacant_house, "build_clear_land") == 0) {
        printf("  FAIL: Strings are swapped! This is the bug.\n");
        return 0;
    }

    /* Also check they're different */
    if (strcmp(clear_land, vacant_house) == 0) {
        printf("  FAIL: Both map to same string \"%s\"\n", clear_land);
        return 0;
    }

    printf("  PASS: Mappings are correct and not swapped\n");
    return 1;
}

/*
 * Test 4: Verify neighboring hotkeys are unaffected by the bug
 */
static int test_neighbors_unaffected(void)
{
    printf("Test: Neighboring hotkeys unaffected...\n");

    /* Check the hotkeys around the affected area */
    printf("  ini_keys[ROTATE_MAP_RIGHT=%d] = \"%s\"\n",
           HOTKEY_ROTATE_MAP_RIGHT, ini_keys[HOTKEY_ROTATE_MAP_RIGHT]);
    printf("  ini_keys[BUILD_CLEAR_LAND=%d] = \"%s\"\n",
           HOTKEY_BUILD_CLEAR_LAND, ini_keys[HOTKEY_BUILD_CLEAR_LAND]);
    printf("  ini_keys[BUILD_VACANT_HOUSE=%d] = \"%s\"\n",
           HOTKEY_BUILD_VACANT_HOUSE, ini_keys[HOTKEY_BUILD_VACANT_HOUSE]);
    printf("  ini_keys[BUILD_ROAD=%d] = \"%s\"\n",
           HOTKEY_BUILD_ROAD, ini_keys[HOTKEY_BUILD_ROAD]);

    if (strcmp(ini_keys[HOTKEY_ROTATE_MAP_RIGHT], "rotate_map_right") != 0) {
        printf("  FAIL: ROTATE_MAP_RIGHT mapping wrong\n");
        return 0;
    }

    if (strcmp(ini_keys[HOTKEY_BUILD_ROAD], "build_road") != 0) {
        printf("  FAIL: BUILD_ROAD mapping wrong\n");
        return 0;
    }

    printf("  PASS: Neighboring hotkeys unaffected\n");
    return 1;
}

int main(void)
{
    int passed = 0;
    int total = 4;

    printf("=== Hotkey Config Ordering Test Suite ===\n");
    printf("Testing ini_keys[] array ordering in hotkey_config.c\n");
    printf("Bug: build_clear_land and build_vacant_house swapped\n\n");

    if (test_clear_land_mapping()) {
        passed++;
    }
    printf("\n");

    if (test_vacant_house_mapping()) {
        passed++;
    }
    printf("\n");

    if (test_mapping_not_swapped()) {
        passed++;
    }
    printf("\n");

    if (test_neighbors_unaffected()) {
        passed++;
    }

    printf("\n=== Test Results ===\n");
    printf("Results: %d/%d tests passed\n", passed, total);

    /* Test fails if any test didn't pass */
    return (passed == total) ? 0 : 1;
}
