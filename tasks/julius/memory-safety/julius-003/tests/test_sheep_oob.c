/**
 * Test for out-of-bounds destination bug in herd animal movement (julius-003)
 *
 * This test compiles against the actual Julius animal.c and triggers the
 * out-of-bounds bug by placing a herd formation at the map corner.
 *
 * When the formation destination is at (160, 160) and animals have positive
 * offsets in the formation, the buggy code calculates destinations like
 * (162, 161) which exceed the 162x162 map bounds.
 *
 * Test validation:
 * - On FIXED code: map_grid_bound() clamps coordinates, no OOB access
 * - On BUGGY code: Coordinates exceed bounds, ASan detects OOB access
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Include actual Julius headers for correct structure definitions */
#include "figure/figure.h"
#include "figure/formation.h"
#include "figuretype/animal.h"

/* Map dimensions - Caesar III uses 162x162 */
#define MAP_WIDTH 162
#define MAP_HEIGHT 162
#define MAP_SIZE (MAP_WIDTH * MAP_HEIGHT)

/* Global map array - ASan will catch out-of-bounds access */
static uint8_t map_terrain[MAP_SIZE];

/* Test formation at map corner */
static formation test_formation;

/*
 * Stub: formation_get
 * Returns our test formation positioned at the map corner.
 * This overrides any other definition.
 */
formation *formation_get(int formation_id)
{
    (void)formation_id;
    return &test_formation;
}

/*
 * Stub: formation_layout_position_x
 * Returns formation offsets - these can push coordinates past map bounds.
 * Real offsets from Julius formation_layout.c
 */
int formation_layout_position_x(int layout, int index)
{
    /* FORMATION_HERD offsets from formation_layout.c */
    static const int herd_offsets_x[] = {
        0, 0, 2, -2, 0, 3, -3, 1, -1, 4, -4, 1, -1, 2, -2, 2
    };
    (void)layout;
    if (index < 0 || index >= 16) return 0;
    return herd_offsets_x[index];
}

/*
 * Stub: formation_layout_position_y
 * Returns formation Y offsets.
 */
int formation_layout_position_y(int layout, int index)
{
    /* FORMATION_HERD offsets from formation_layout.c */
    static const int herd_offsets_y[] = {
        0, -2, 0, 0, 2, 0, 0, -1, -1, 0, 0, 1, 1, -2, -2, 2
    };
    (void)layout;
    if (index < 0 || index >= 16) return 0;
    return herd_offsets_y[index];
}

/*
 * Stub: map_grid_bound
 * This is the key fix function - clamps coordinates to valid range.
 * In buggy code, herd_get_destination() doesn't exist so this isn't called
 * for destination calculation. In fixed code, it IS called.
 */
void map_grid_bound(int *x, int *y)
{
    if (*x < 0) *x = 0;
    if (*y < 0) *y = 0;
    if (*x >= MAP_WIDTH) *x = MAP_WIDTH - 1;
    if (*y >= MAP_HEIGHT) *y = MAP_HEIGHT - 1;
}

/* Stubs for other functions called by figure_sheep_action */
void city_figures_add_animal(void) {}
void figure_image_increase_offset(figure *f, int amount) { (void)f; (void)amount; }
void figure_combat_handle_attack(figure *f) { (void)f; }
void figure_combat_handle_corpse(figure *f) { (void)f; }
void figure_route_remove(figure *f) { (void)f; }

/* Direction constants */
#define DIR_FIGURE_AT_DESTINATION 8

void figure_movement_move_ticks(figure *f, int num_ticks)
{
    (void)num_ticks;
    /* Simulate reaching destination so the test completes */
    f->direction = DIR_FIGURE_AT_DESTINATION;
}

/*
 * Test: Herd animal at map corner with positive formation offset
 *
 * This exercises the bug where:
 * - Formation is at (160, 160)
 * - Animal has index 2, offset (+2, 0)
 * - Buggy: destination = (162, 160) - OUT OF BOUNDS!
 * - Fixed: destination = (161, 160) - clamped to valid range
 */
static int test_sheep_at_corner(void)
{
    figure f;
    memset(&f, 0, sizeof(f));

    printf("Test: Sheep destination at map corner...\n");
    printf("  Formation destination at (%d, %d)\n",
           test_formation.destination_x, test_formation.destination_y);

    /* Set up figure to trigger destination calculation.
     * Use index 4 which has offset (0, +2) in Y direction.
     * With formation at y=161, this gives y=163 which is OOB.
     */
    f.id = 1;
    f.formation_id = 1;
    f.index_in_formation = 4;  /* Offset +0, +2 (positive Y) */
    f.action_state = FIGURE_ACTION_196_HERD_ANIMAL_AT_REST;
    f.wait_ticks = 500;  /* > 400, triggers transition to moving */

    printf("  Animal index %d has offset (%d, %d)\n",
           f.index_in_formation,
           formation_layout_position_x(0, f.index_in_formation),
           formation_layout_position_y(0, f.index_in_formation));

    /* Call the actual Julius function */
    figure_sheep_action(&f);

    printf("  Computed destination: (%d, %d)\n", f.destination_x, f.destination_y);

    /* Check expected behavior */
    int expected_raw_x = test_formation.destination_x + formation_layout_position_x(0, 4);
    int expected_raw_y = test_formation.destination_y + formation_layout_position_y(0, 4);
    printf("  Expected raw (unbounded): (%d, %d)\n", expected_raw_x, expected_raw_y);

    /* Verify coordinates are within bounds */
    if (f.destination_x >= MAP_WIDTH || f.destination_y >= MAP_HEIGHT) {
        printf("  ERROR: Destination out of bounds!\n");
        printf("  This should have been caught by bounds checking.\n");

        /*
         * Trigger the actual OOB access so ASan can detect it.
         * In real game code, this happens when the figure uses
         * these coordinates to access map data.
         */
        int index = f.destination_y * MAP_WIDTH + f.destination_x;
        printf("  Accessing map at index %d (max valid: %d)\n", index, MAP_SIZE - 1);

        /* This will cause ASan heap-buffer-overflow on buggy code */
        map_terrain[index] = 1;

        return 0;  /* Test fails */
    }

    /* Safe access - coordinates are bounded */
    int index = f.destination_y * MAP_WIDTH + f.destination_x;
    map_terrain[index] = 1;

    printf("  PASS: Destination properly bounded\n");
    return 1;
}

/*
 * Test: Multiple animals with various formation positions
 */
static int test_all_formation_positions(void)
{
    printf("Test: All formation positions at corner...\n");

    /* Reset formation to corner - y=161 so positive y offsets go OOB */
    test_formation.destination_x = MAP_WIDTH - 2;  /* 160 */
    test_formation.destination_y = MAP_HEIGHT - 1; /* 161 */

    for (int i = 0; i < 16; i++) {
        figure f;
        memset(&f, 0, sizeof(f));

        f.id = i + 1;
        f.formation_id = 1;
        f.index_in_formation = i;
        f.action_state = FIGURE_ACTION_196_HERD_ANIMAL_AT_REST;
        f.wait_ticks = 500;

        figure_sheep_action(&f);

        int offset_x = formation_layout_position_x(0, i);
        int offset_y = formation_layout_position_y(0, i);

        if (f.destination_x >= MAP_WIDTH || f.destination_y >= MAP_HEIGHT) {
            printf("  Position %d (offset %+d, %+d): FAIL - (%d, %d) out of bounds\n",
                   i, offset_x, offset_y, f.destination_x, f.destination_y);

            /* Trigger OOB access for ASan */
            int index = f.destination_y * MAP_WIDTH + f.destination_x;
            map_terrain[index] = 1;
            return 0;
        }
    }

    printf("  PASS: All 16 positions properly bounded\n");
    return 1;
}

/*
 * Test: Formation at (0, 0) with negative offsets
 */
static int test_negative_offset_corner(void)
{
    printf("Test: Sheep at origin with negative offset...\n");

    /* Move formation to origin */
    test_formation.destination_x = 0;
    test_formation.destination_y = 0;

    figure f;
    memset(&f, 0, sizeof(f));

    f.id = 1;
    f.formation_id = 1;
    f.index_in_formation = 3;  /* Offset -2 in X direction */
    f.action_state = FIGURE_ACTION_196_HERD_ANIMAL_AT_REST;
    f.wait_ticks = 500;

    int offset_x = formation_layout_position_x(0, 3);
    int offset_y = formation_layout_position_y(0, 3);
    printf("  Formation at (0, 0), offset (%+d, %+d)\n", offset_x, offset_y);

    figure_sheep_action(&f);

    printf("  Computed destination: (%d, %d)\n", f.destination_x, f.destination_y);

    /* Check for underflow (would wrap around for uint8_t if unchecked) */
    /* With buggy code: 0 + (-2) = -2, stored as uint8_t becomes 254 */
    if (f.destination_x > MAP_WIDTH || f.destination_y > MAP_HEIGHT) {
        printf("  ERROR: Coordinate underflow detected!\n");

        /* This would be OOB if we try to use it */
        int index = f.destination_y * MAP_WIDTH + f.destination_x;
        printf("  Index would be %d (max valid: %d)\n", index, MAP_SIZE - 1);
        if (index >= MAP_SIZE) {
            /* Trigger ASan error */
            map_terrain[index] = 1;
        }
        return 0;
    }

    /* Safe access */
    int index = f.destination_y * MAP_WIDTH + f.destination_x;
    map_terrain[index] = 1;

    printf("  PASS: Negative offset handled correctly\n");
    return 1;
}

int main(void)
{
    int passed = 0;
    int total = 3;

    printf("=== Sheep Out-of-Bounds Test Suite ===\n");
    printf("Testing against actual Julius animal.c\n\n");

    /* Initialize map */
    memset(map_terrain, 0, sizeof(map_terrain));

    /* Set up formation at bottom-right corner of map.
     * Use y = MAP_HEIGHT - 1 = 161 so that when buggy code adds
     * a positive y offset, the resulting index exceeds the array bounds.
     * For example: (160, 161) + offset(0, +2) = (160, 163)
     * index = 163 * 162 + 160 = 26406 + 160 = 26566 > 26243 (OOB!)
     */
    memset(&test_formation, 0, sizeof(test_formation));
    test_formation.in_use = 1;
    test_formation.destination_x = MAP_WIDTH - 2;  /* 160 */
    test_formation.destination_y = MAP_HEIGHT - 1; /* 161 */

    if (test_sheep_at_corner()) {
        passed++;
    }

    if (test_all_formation_positions()) {
        passed++;
    }

    if (test_negative_offset_corner()) {
        passed++;
    }

    printf("\n=== Test Results ===\n");
    printf("Results: %d/%d tests passed\n", passed, total);

    return (passed == total) ? 0 : 1;
}
