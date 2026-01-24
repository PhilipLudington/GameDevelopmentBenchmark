/**
 * Stub implementations for Julius dependencies
 *
 * These stubs provide minimal implementations of Julius functions
 * that animal.c depends on, allowing the test to compile and run
 * against the actual animal.c code.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Stub for building_get */
void *building_get(int id)
{
    (void)id;
    return NULL;
}

/* Stub for calc_general_direction */
int calc_general_direction(int x_from, int y_from, int x_to, int y_to)
{
    (void)x_from; (void)y_from; (void)x_to; (void)y_to;
    return 0;
}

/* Stub for city_entertainment functions */
int city_entertainment_hippodrome_has_race(void) { return 0; }
void city_entertainment_set_hippodrome_has_race(int has_race) { (void)has_race; }

/* Stub for city_view_orientation */
int city_view_orientation(void) { return 0; }

/* Stub for figure_combat functions */
int figure_combat_get_target_for_wolf(int x, int y, int max_distance)
{
    (void)x; (void)y; (void)max_distance;
    return 0;
}

/* Stub for figure_create */
void *figure_create(int type, int x, int y, int dir)
{
    (void)type; (void)x; (void)y; (void)dir;
    return NULL;
}

/* Stub for figure_get */
void *figure_get(int id)
{
    (void)id;
    return NULL;
}

/* Stub for figure_image functions */
int figure_image_corpse_offset(void *f) { (void)f; return 0; }
int figure_image_direction(void *f) { (void)f; return 0; }
void figure_image_set_cart_offset(void *f, int offset) { (void)f; (void)offset; }

/* Stub for figure_movement functions */
void figure_movement_move_ticks_cross_country(void *f, int ticks)
{
    (void)f; (void)ticks;
}

void figure_movement_set_cross_country_destination(void *f, int x, int y)
{
    (void)f; (void)x; (void)y;
}

void figure_movement_set_cross_country_direction(void *f, int x, int y, int is_missile)
{
    (void)f; (void)x; (void)y; (void)is_missile;
}

/* Stub for formation_create_herd */
int formation_create_herd(int type, int x, int y)
{
    (void)type; (void)x; (void)y;
    return 0;
}

/* Stub for image_group */
int image_group(int group)
{
    (void)group;
    return 0;
}

/* Stub for map functions */
void map_figure_add(void *f) { (void)f; }
void map_figure_delete(void *f) { (void)f; }
int map_grid_offset(int x, int y)
{
    (void)x; (void)y;
    return 0;
}

/* Stub for random functions */
int random_byte(void) { return 0; }
void random_generate_next(void) {}

/* Stub for scenario_map functions */
void scenario_map_foreach_fishing_point(void (*callback)(int, int))
{
    (void)callback;
}

void scenario_map_foreach_herd_point(void (*callback)(int, int))
{
    (void)callback;
}

/* Stub for scenario_property_climate */
int scenario_property_climate(void) { return 0; }
