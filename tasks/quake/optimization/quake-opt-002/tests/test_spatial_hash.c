/*
 * Spatial Hash / Entity Collision Test Harness
 *
 * Tests correctness and performance of entity collision detection.
 * Both versions should produce identical collision results.
 *
 * Test categories:
 * 1. Correctness - Collision results must match
 * 2. Performance - Measure time with varying entity counts
 * 3. Scaling - Verify O(n) vs O(n²) behavior
 * 4. Edge cases - Boundary conditions, large entities
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>

/* ============== MOCK QUAKE TYPES ============== */

typedef unsigned char byte;
typedef float vec_t;
typedef vec_t vec3_t[3];
typedef int qboolean;

#define true 1
#define false 0

#define DotProduct(a, b) ((a)[0] * (b)[0] + (a)[1] * (b)[1] + (a)[2] * (b)[2])
#define VectorCopy(s, d) ((d)[0] = (s)[0], (d)[1] = (s)[1], (d)[2] = (s)[2])
#define VectorClear(v) ((v)[0] = (v)[1] = (v)[2] = 0)
#define VectorAdd(a, b, c) ((c)[0] = (a)[0] + (b)[0], (c)[1] = (a)[1] + (b)[1], (c)[2] = (a)[2] + (b)[2])
#define VectorSubtract(a, b, c) ((c)[0] = (a)[0] - (b)[0], (c)[1] = (a)[1] - (b)[1], (c)[2] = (a)[2] - (b)[2])
#define VectorMA(v, s, b, d) ((d)[0] = (v)[0] + (s) * (b)[0], (d)[1] = (v)[1] + (s) * (b)[1], (d)[2] = (v)[2] + (s) * (b)[2])

#define SOLID_NOT       0
#define SOLID_TRIGGER   1
#define SOLID_BBOX      2
#define SOLID_SLIDEBOX  3
#define SOLID_BSP       4

#define AREA_SOLID      1
#define AREA_TRIGGERS   2

#define MAX_EDICTS      1024

/* ============== ENTITY STRUCTURE ============== */

typedef struct link_s {
    struct link_s   *prev, *next;
} link_t;

typedef struct edict_s {
    struct {
        link_t      *prev, *next;
    } area;
    qboolean    free;
    qboolean    linked;
    int         solid;
    vec3_t      origin;
    vec3_t      angles;
    vec3_t      mins, maxs;
    vec3_t      absmin, absmax;
    vec3_t      velocity;
    int         id;
} edict_t;

/* ============== WORLD IMPLEMENTATION ============== */

#ifdef TEST_OPTIMIZED

/* === OPTIMIZED VERSION WITH SPATIAL HASH === */

#define AREA_DEPTH          4
#define AREA_NODES          32
#define GRID_CELL_SIZE      64.0f
#define GRID_HASH_SIZE      2048
#define MAX_CELL_ENTITIES   32
#define HASH_PRIME_X        73856093
#define HASH_PRIME_Y        19349663
#define HASH_PRIME_Z        83492791
#define MAX_OVERFLOW_CELLS  256

typedef struct areanode_s {
    int             axis;
    float           dist;
    struct areanode_s   *children[2];
    link_t          trigger_edicts;
    link_t          solid_edicts;
} areanode_t;

typedef struct grid_cell_s {
    edict_t         *entities[MAX_CELL_ENTITIES];
    int             count;
    struct grid_cell_s  *overflow;
} grid_cell_t;

typedef struct {
    grid_cell_t     cells[GRID_HASH_SIZE];
    int             num_overflow;
} spatial_hash_t;

static areanode_t   sv_areanodes[AREA_NODES];
static int          sv_numareanodes;
static edict_t      sv_edicts[MAX_EDICTS];
static int          sv_num_edicts;
static vec3_t       world_mins, world_maxs;
static spatial_hash_t   solid_hash;
static spatial_hash_t   trigger_hash;
static grid_cell_t  overflow_pool[MAX_OVERFLOW_CELLS];
static int          overflow_used;

static void ClearLink(link_t *l) { l->prev = l->next = l; }
static void RemoveLink(link_t *l) { l->next->prev = l->prev; l->prev->next = l->next; }
static void InsertLinkBefore(link_t *l, link_t *before) {
    l->next = before; l->prev = before->prev;
    l->prev->next = l; l->next->prev = l;
}

static inline int Hash_CellCoord(float pos) { return (int)floorf(pos / GRID_CELL_SIZE); }
static inline unsigned int Hash_Position(int cx, int cy, int cz) {
    return ((unsigned int)(cx * HASH_PRIME_X) ^ (unsigned int)(cy * HASH_PRIME_Y) ^
            (unsigned int)(cz * HASH_PRIME_Z)) & (GRID_HASH_SIZE - 1);
}
static grid_cell_t *Hash_GetCell(spatial_hash_t *hash, unsigned int idx) { return &hash->cells[idx]; }
static void Hash_Clear(spatial_hash_t *hash) {
    for (int i = 0; i < GRID_HASH_SIZE; i++) { hash->cells[i].count = 0; hash->cells[i].overflow = NULL; }
}
static grid_cell_t *Hash_AllocOverflow(void) {
    if (overflow_used >= MAX_OVERFLOW_CELLS) return NULL;
    grid_cell_t *cell = &overflow_pool[overflow_used++];
    cell->count = 0; cell->overflow = NULL;
    return cell;
}
static void Hash_InsertIntoCell(grid_cell_t *cell, edict_t *ent) {
    for (int i = 0; i < cell->count; i++) if (cell->entities[i] == ent) return;
    if (cell->count < MAX_CELL_ENTITIES) { cell->entities[cell->count++] = ent; }
    else { if (!cell->overflow) cell->overflow = Hash_AllocOverflow();
           if (cell->overflow) Hash_InsertIntoCell(cell->overflow, ent); }
}
static void Hash_RemoveFromCell(grid_cell_t *cell, edict_t *ent) {
    for (int i = 0; i < cell->count; i++) {
        if (cell->entities[i] == ent) { cell->entities[i] = cell->entities[--cell->count]; return; }
    }
    if (cell->overflow) Hash_RemoveFromCell(cell->overflow, ent);
}
static void Hash_InsertEntity(spatial_hash_t *hash, edict_t *ent) {
    int min_cx = Hash_CellCoord(ent->absmin[0]), min_cy = Hash_CellCoord(ent->absmin[1]), min_cz = Hash_CellCoord(ent->absmin[2]);
    int max_cx = Hash_CellCoord(ent->absmax[0]), max_cy = Hash_CellCoord(ent->absmax[1]), max_cz = Hash_CellCoord(ent->absmax[2]);
    for (int cz = min_cz; cz <= max_cz; cz++)
        for (int cy = min_cy; cy <= max_cy; cy++)
            for (int cx = min_cx; cx <= max_cx; cx++)
                Hash_InsertIntoCell(Hash_GetCell(hash, Hash_Position(cx, cy, cz)), ent);
}
static void Hash_RemoveEntity(spatial_hash_t *hash, edict_t *ent) {
    int min_cx = Hash_CellCoord(ent->absmin[0]), min_cy = Hash_CellCoord(ent->absmin[1]), min_cz = Hash_CellCoord(ent->absmin[2]);
    int max_cx = Hash_CellCoord(ent->absmax[0]), max_cy = Hash_CellCoord(ent->absmax[1]), max_cz = Hash_CellCoord(ent->absmax[2]);
    for (int cz = min_cz; cz <= max_cz; cz++)
        for (int cy = min_cy; cy <= max_cy; cy++)
            for (int cx = min_cx; cx <= max_cx; cx++)
                Hash_RemoveFromCell(Hash_GetCell(hash, Hash_Position(cx, cy, cz)), ent);
}

static areanode_t *SV_CreateAreaNode(int depth, vec3_t mins, vec3_t maxs) {
    areanode_t *anode = &sv_areanodes[sv_numareanodes++];
    ClearLink(&anode->trigger_edicts); ClearLink(&anode->solid_edicts);
    if (depth == AREA_DEPTH) { anode->axis = -1; anode->children[0] = anode->children[1] = NULL; return anode; }
    vec3_t size; VectorSubtract(maxs, mins, size);
    anode->axis = (size[0] > size[1]) ? 0 : 1;
    anode->dist = 0.5f * (maxs[anode->axis] + mins[anode->axis]);
    vec3_t mins1, maxs1, mins2, maxs2;
    VectorCopy(mins, mins1); VectorCopy(mins, mins2); VectorCopy(maxs, maxs1); VectorCopy(maxs, maxs2);
    maxs1[anode->axis] = mins2[anode->axis] = anode->dist;
    anode->children[0] = SV_CreateAreaNode(depth + 1, mins2, maxs2);
    anode->children[1] = SV_CreateAreaNode(depth + 1, mins1, maxs1);
    return anode;
}

void SV_ClearWorld(vec3_t mins, vec3_t maxs) {
    VectorCopy(mins, world_mins); VectorCopy(maxs, world_maxs);
    sv_numareanodes = 0; SV_CreateAreaNode(0, mins, maxs);
    Hash_Clear(&solid_hash); Hash_Clear(&trigger_hash); overflow_used = 0;
    sv_num_edicts = 0; memset(sv_edicts, 0, sizeof(sv_edicts));
}

void SV_UnlinkEdict(edict_t *ent) {
    if (!ent->linked) return;
    if (ent->solid == SOLID_TRIGGER) Hash_RemoveEntity(&trigger_hash, ent);
    else Hash_RemoveEntity(&solid_hash, ent);
    if (ent->area.prev) { RemoveLink((link_t*)&ent->area); ent->area.prev = ent->area.next = NULL; }
    ent->linked = false;
}

void SV_LinkEdict(edict_t *ent) {
    if (ent->linked) SV_UnlinkEdict(ent);
    if (ent->free) return;
    VectorAdd(ent->origin, ent->mins, ent->absmin);
    VectorAdd(ent->origin, ent->maxs, ent->absmax);
    if (ent->solid == SOLID_TRIGGER) Hash_InsertEntity(&trigger_hash, ent);
    else Hash_InsertEntity(&solid_hash, ent);
    areanode_t *node = sv_areanodes;
    while (1) {
        if (node->axis == -1) break;
        if (ent->absmin[node->axis] > node->dist) node = node->children[0];
        else if (ent->absmax[node->axis] < node->dist) node = node->children[1];
        else break;
    }
    link_t *list = (ent->solid == SOLID_TRIGGER) ? &node->trigger_edicts : &node->solid_edicts;
    InsertLinkBefore((link_t*)&ent->area, list);
    ent->linked = true;
}

int SV_AreaEdicts(vec3_t mins, vec3_t maxs, edict_t **list, int maxcount, int type) {
    spatial_hash_t *hash = (type == AREA_SOLID) ? &solid_hash : &trigger_hash;
    int count = 0;
    static edict_t *seen[MAX_EDICTS]; static int seen_count = 0; seen_count = 0;
    int min_cx = Hash_CellCoord(mins[0]), min_cy = Hash_CellCoord(mins[1]), min_cz = Hash_CellCoord(mins[2]);
    int max_cx = Hash_CellCoord(maxs[0]), max_cy = Hash_CellCoord(maxs[1]), max_cz = Hash_CellCoord(maxs[2]);
    for (int cz = min_cz; cz <= max_cz; cz++) {
        for (int cy = min_cy; cy <= max_cy; cy++) {
            for (int cx = min_cx; cx <= max_cx; cx++) {
                grid_cell_t *cell = Hash_GetCell(hash, Hash_Position(cx, cy, cz));
                while (cell) {
                    for (int i = 0; i < cell->count; i++) {
                        edict_t *ent = cell->entities[i];
                        if (ent->solid == SOLID_NOT) continue;
                        int already_seen = 0;
                        for (int j = 0; j < seen_count; j++) if (seen[j] == ent) { already_seen = 1; break; }
                        if (already_seen) continue;
                        if (seen_count < MAX_EDICTS) seen[seen_count++] = ent;
                        if (ent->absmin[0] > maxs[0] || ent->absmin[1] > maxs[1] || ent->absmin[2] > maxs[2] ||
                            ent->absmax[0] < mins[0] || ent->absmax[1] < mins[1] || ent->absmax[2] < mins[2]) continue;
                        if (count >= maxcount) return count;
                        list[count++] = ent;
                    }
                    cell = cell->overflow;
                }
            }
        }
    }
    return count;
}

#else

/* === UNOPTIMIZED VERSION WITH LINKED LISTS === */

#define AREA_DEPTH  4
#define AREA_NODES  32

typedef struct areanode_s {
    int             axis;
    float           dist;
    struct areanode_s   *children[2];
    link_t          trigger_edicts;
    link_t          solid_edicts;
} areanode_t;

static areanode_t   sv_areanodes[AREA_NODES];
static int          sv_numareanodes;
static edict_t      sv_edicts[MAX_EDICTS];
static int          sv_num_edicts;
static vec3_t       world_mins, world_maxs;

static void ClearLink(link_t *l) { l->prev = l->next = l; }
static void RemoveLink(link_t *l) { l->next->prev = l->prev; l->prev->next = l->next; }
static void InsertLinkBefore(link_t *l, link_t *before) {
    l->next = before; l->prev = before->prev;
    l->prev->next = l; l->next->prev = l;
}
#define EDICT_FROM_AREA(l) ((edict_t *)((byte *)l - offsetof(edict_t, area)))

static areanode_t *SV_CreateAreaNode(int depth, vec3_t mins, vec3_t maxs) {
    areanode_t *anode = &sv_areanodes[sv_numareanodes++];
    ClearLink(&anode->trigger_edicts); ClearLink(&anode->solid_edicts);
    if (depth == AREA_DEPTH) { anode->axis = -1; anode->children[0] = anode->children[1] = NULL; return anode; }
    vec3_t size; VectorSubtract(maxs, mins, size);
    anode->axis = (size[0] > size[1]) ? 0 : 1;
    anode->dist = 0.5f * (maxs[anode->axis] + mins[anode->axis]);
    vec3_t mins1, maxs1, mins2, maxs2;
    VectorCopy(mins, mins1); VectorCopy(mins, mins2); VectorCopy(maxs, maxs1); VectorCopy(maxs, maxs2);
    maxs1[anode->axis] = mins2[anode->axis] = anode->dist;
    anode->children[0] = SV_CreateAreaNode(depth + 1, mins2, maxs2);
    anode->children[1] = SV_CreateAreaNode(depth + 1, mins1, maxs1);
    return anode;
}

void SV_ClearWorld(vec3_t mins, vec3_t maxs) {
    VectorCopy(mins, world_mins); VectorCopy(maxs, world_maxs);
    sv_numareanodes = 0; SV_CreateAreaNode(0, mins, maxs);
    sv_num_edicts = 0; memset(sv_edicts, 0, sizeof(sv_edicts));
}

void SV_UnlinkEdict(edict_t *ent) {
    if (!ent->area.prev) return;
    RemoveLink((link_t*)&ent->area);
    ent->area.prev = ent->area.next = NULL;
}

void SV_LinkEdict(edict_t *ent) {
    if (ent->area.prev) SV_UnlinkEdict(ent);
    if (ent->free) return;
    VectorAdd(ent->origin, ent->mins, ent->absmin);
    VectorAdd(ent->origin, ent->maxs, ent->absmax);
    areanode_t *node = sv_areanodes;
    while (1) {
        if (node->axis == -1) break;
        if (ent->absmin[node->axis] > node->dist) node = node->children[0];
        else if (ent->absmax[node->axis] < node->dist) node = node->children[1];
        else break;
    }
    link_t *list = (ent->solid == SOLID_TRIGGER) ? &node->trigger_edicts : &node->solid_edicts;
    InsertLinkBefore((link_t*)&ent->area, list);
    ent->linked = true;
}

static edict_t *area_list[MAX_EDICTS];
static int area_count, area_maxcount, area_type;
static vec3_t area_mins, area_maxs;

static void SV_AreaEdicts_r(areanode_t *node) {
    link_t *start = (area_type == AREA_SOLID) ? &node->solid_edicts : &node->trigger_edicts;
    for (link_t *l = start->next; l != start; l = l->next) {
        edict_t *check = EDICT_FROM_AREA(l);
        if (check->solid == SOLID_NOT) continue;
        if (check->absmin[0] > area_maxs[0] || check->absmin[1] > area_maxs[1] || check->absmin[2] > area_maxs[2] ||
            check->absmax[0] < area_mins[0] || check->absmax[1] < area_mins[1] || check->absmax[2] < area_mins[2]) continue;
        if (area_count >= area_maxcount) return;
        area_list[area_count++] = check;
    }
    if (node->axis == -1) return;
    if (area_maxs[node->axis] > node->dist) SV_AreaEdicts_r(node->children[0]);
    if (area_mins[node->axis] < node->dist) SV_AreaEdicts_r(node->children[1]);
}

int SV_AreaEdicts(vec3_t mins, vec3_t maxs, edict_t **list, int maxcount, int type) {
    VectorCopy(mins, area_mins); VectorCopy(maxs, area_maxs);
    area_count = 0; area_maxcount = maxcount; area_type = type;
    SV_AreaEdicts_r(sv_areanodes);
    for (int i = 0; i < area_count && i < maxcount; i++) list[i] = area_list[i];
    return area_count;
}

#endif

/* === COMMON FUNCTIONS === */

edict_t *SV_CreateEdict(void) {
    if (sv_num_edicts >= MAX_EDICTS) return NULL;
    edict_t *ent = &sv_edicts[sv_num_edicts];
    sv_num_edicts++;
    memset(ent, 0, sizeof(*ent));
    ent->free = false;
    ent->solid = SOLID_BBOX;
    return ent;
}

void SV_FreeEdict(edict_t *ent) {
    SV_UnlinkEdict(ent);
    ent->free = true;
}

qboolean SV_TestEntityPosition(edict_t *ent) {
    edict_t *touched[MAX_EDICTS];
    int num_touched = SV_AreaEdicts(ent->absmin, ent->absmax, touched, MAX_EDICTS, AREA_SOLID);
    for (int i = 0; i < num_touched; i++) {
        if (touched[i] == ent) continue;
        if (touched[i]->solid == SOLID_NOT) continue;
        return true;
    }
    return false;
}

void SV_RunPhysics(float frametime) {
    for (int i = 0; i < sv_num_edicts; i++) {
        edict_t *ent = &sv_edicts[i];
        if (ent->free) continue;
        VectorMA(ent->origin, frametime, ent->velocity, ent->origin);
        SV_LinkEdict(ent);
        if (SV_TestEntityPosition(ent)) {
            VectorMA(ent->origin, -frametime, ent->velocity, ent->origin);
            VectorClear(ent->velocity);
            SV_LinkEdict(ent);
        }
    }
}

int SV_GetEdictCount(void) { return sv_num_edicts; }

/* ============== TEST FRAMEWORK ============== */

int tests_run = 0;
int tests_passed = 0;

#define TEST(name) void test_##name(void)
#define RUN_TEST(name) do { printf("  %-50s ", #name); fflush(stdout); test_##name(); tests_run++; } while(0)
#define ASSERT(cond) do { if (!(cond)) { printf("FAIL\n    Assertion failed: %s\n", #cond); return; } } while(0)
#define ASSERT_MSG(cond, msg) do { if (!(cond)) { printf("FAIL\n    %s\n", msg); return; } } while(0)
#define PASS() do { printf("PASS\n"); tests_passed++; } while(0)

static double get_time_us(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1e6 + ts.tv_nsec / 1e3;
}

/* ============== HELPER FUNCTIONS ============== */

static void setup_world(float size) {
    vec3_t mins = {-size, -size, -size};
    vec3_t maxs = {size, size, size};
    SV_ClearWorld(mins, maxs);
}

static edict_t *create_entity_at(float x, float y, float z, float size) {
    edict_t *ent = SV_CreateEdict();
    if (!ent) return NULL;
    ent->origin[0] = x;
    ent->origin[1] = y;
    ent->origin[2] = z;
    ent->mins[0] = ent->mins[1] = ent->mins[2] = -size/2;
    ent->maxs[0] = ent->maxs[1] = ent->maxs[2] = size/2;
    ent->solid = SOLID_BBOX;
    ent->id = sv_num_edicts - 1;
    SV_LinkEdict(ent);
    return ent;
}

static void create_random_entities(int count, float world_size, float entity_size) {
    srand(12345);  // Fixed seed for reproducibility
    for (int i = 0; i < count; i++) {
        float x = ((float)rand() / RAND_MAX) * world_size * 2 - world_size;
        float y = ((float)rand() / RAND_MAX) * world_size * 2 - world_size;
        float z = ((float)rand() / RAND_MAX) * world_size * 2 - world_size;
        edict_t *ent = create_entity_at(x, y, z, entity_size);
        if (ent) {
            ent->velocity[0] = ((float)rand() / RAND_MAX) * 200 - 100;
            ent->velocity[1] = ((float)rand() / RAND_MAX) * 200 - 100;
            ent->velocity[2] = ((float)rand() / RAND_MAX) * 50 - 25;
        }
    }
}

/* ============== CORRECTNESS TESTS ============== */

TEST(basic_entity_creation) {
    setup_world(1000);
    edict_t *ent = create_entity_at(0, 0, 0, 32);
    ASSERT(ent != NULL);
    ASSERT(ent->linked == true);
    ASSERT(SV_GetEdictCount() == 1);
    PASS();
}

TEST(entity_query_single) {
    setup_world(1000);
    create_entity_at(100, 100, 0, 32);

    edict_t *found[64];
    vec3_t mins = {80, 80, -16};
    vec3_t maxs = {120, 120, 16};
    int count = SV_AreaEdicts(mins, maxs, found, 64, AREA_SOLID);

    ASSERT(count == 1);
    PASS();
}

TEST(entity_query_miss) {
    setup_world(1000);
    create_entity_at(100, 100, 0, 32);

    edict_t *found[64];
    vec3_t mins = {-200, -200, -16};
    vec3_t maxs = {-100, -100, 16};
    int count = SV_AreaEdicts(mins, maxs, found, 64, AREA_SOLID);

    ASSERT(count == 0);
    PASS();
}

TEST(entity_query_multiple) {
    setup_world(1000);
    create_entity_at(0, 0, 0, 32);
    create_entity_at(50, 0, 0, 32);
    create_entity_at(100, 0, 0, 32);
    create_entity_at(500, 0, 0, 32);  // Far away

    edict_t *found[64];
    vec3_t mins = {-50, -50, -50};
    vec3_t maxs = {150, 50, 50};
    int count = SV_AreaEdicts(mins, maxs, found, 64, AREA_SOLID);

    ASSERT(count == 3);  // Should find first 3, not the far one
    PASS();
}

TEST(collision_detection) {
    setup_world(1000);
    edict_t *ent1 = create_entity_at(0, 0, 0, 32);
    edict_t *ent2 = create_entity_at(20, 0, 0, 32);  // Overlapping
    edict_t *ent3 = create_entity_at(200, 0, 0, 32); // Not overlapping

    ASSERT(SV_TestEntityPosition(ent1) == true);   // Collides with ent2
    ASSERT(SV_TestEntityPosition(ent3) == false);  // No collision
    PASS();
}

TEST(entity_movement) {
    setup_world(1000);
    edict_t *ent = create_entity_at(0, 0, 0, 32);
    ent->velocity[0] = 100;
    ent->velocity[1] = 0;
    ent->velocity[2] = 0;

    float old_x = ent->origin[0];
    SV_RunPhysics(0.1f);  // 100ms
    float new_x = ent->origin[0];

    ASSERT(fabsf(new_x - old_x - 10.0f) < 0.01f);  // Should move 10 units
    PASS();
}

TEST(large_entity_spanning_cells) {
    setup_world(1000);
    // Create a large entity that spans multiple cells (cell size = 64)
    edict_t *big = create_entity_at(0, 0, 0, 256);  // 256 units wide

    edict_t *found[64];
    vec3_t mins = {100, -10, -10};
    vec3_t maxs = {110, 10, 10};
    int count = SV_AreaEdicts(mins, maxs, found, 64, AREA_SOLID);

    ASSERT(count == 1);  // Should still find the big entity
    PASS();
}

/* ============== PERFORMANCE TESTS ============== */

TEST(performance_64_entities) {
    setup_world(2000);
    create_random_entities(64, 1500, 32);

    double start = get_time_us();
    for (int i = 0; i < 100; i++) {
        SV_RunPhysics(0.016f);  // 16ms frame
    }
    double end = get_time_us();

    double avg_ms = (end - start) / 100.0 / 1000.0;
    printf("PASS (%.2f ms avg per frame, 64 entities)\n", avg_ms);
    tests_passed++;
}

TEST(performance_128_entities) {
    setup_world(2000);
    create_random_entities(128, 1500, 32);

    double start = get_time_us();
    for (int i = 0; i < 100; i++) {
        SV_RunPhysics(0.016f);
    }
    double end = get_time_us();

    double avg_ms = (end - start) / 100.0 / 1000.0;
    printf("PASS (%.2f ms avg per frame, 128 entities)\n", avg_ms);
    tests_passed++;
}

TEST(performance_256_entities) {
    setup_world(2000);
    create_random_entities(256, 1500, 32);

    double start = get_time_us();
    for (int i = 0; i < 100; i++) {
        SV_RunPhysics(0.016f);
    }
    double end = get_time_us();

    double avg_ms = (end - start) / 100.0 / 1000.0;
    printf("PASS (%.2f ms avg per frame, 256 entities)\n", avg_ms);
    tests_passed++;
}

TEST(performance_512_entities) {
    setup_world(3000);
    create_random_entities(512, 2500, 32);

    double start = get_time_us();
    for (int i = 0; i < 50; i++) {
        SV_RunPhysics(0.016f);
    }
    double end = get_time_us();

    double avg_ms = (end - start) / 50.0 / 1000.0;
    printf("PASS (%.2f ms avg per frame, 512 entities)\n", avg_ms);
    tests_passed++;
}

/* ============== SCALING TEST ============== */

TEST(scaling_behavior) {
    double times[4];
    int counts[] = {64, 128, 256, 512};

    for (int t = 0; t < 4; t++) {
        setup_world(3000);
        create_random_entities(counts[t], 2500, 32);

        double start = get_time_us();
        for (int i = 0; i < 50; i++) {
            SV_RunPhysics(0.016f);
        }
        double end = get_time_us();
        times[t] = (end - start) / 50.0;
    }

    // Check scaling: O(n²) would show ~4x time for 2x entities
    // O(n) would show ~2x time for 2x entities
    double ratio_128_64 = times[1] / times[0];
    double ratio_256_128 = times[2] / times[1];
    double ratio_512_256 = times[3] / times[2];

    printf("PASS (scaling ratios: %.1fx, %.1fx, %.1fx)\n",
           ratio_128_64, ratio_256_128, ratio_512_256);

#ifdef TEST_OPTIMIZED
    // Optimized version should scale roughly linearly
    ASSERT_MSG(ratio_256_128 < 3.0, "Scaling should be better than O(n²)");
#endif
    tests_passed++;
}

/* ============== EDGE CASE TESTS ============== */

TEST(empty_world_query) {
    setup_world(1000);

    edict_t *found[64];
    vec3_t mins = {-100, -100, -100};
    vec3_t maxs = {100, 100, 100};
    int count = SV_AreaEdicts(mins, maxs, found, 64, AREA_SOLID);

    ASSERT(count == 0);
    PASS();
}

TEST(entity_at_origin) {
    setup_world(1000);
    edict_t *ent = create_entity_at(0, 0, 0, 32);

    edict_t *found[64];
    vec3_t mins = {-1, -1, -1};
    vec3_t maxs = {1, 1, 1};
    int count = SV_AreaEdicts(mins, maxs, found, 64, AREA_SOLID);

    ASSERT(count == 1);
    PASS();
}

TEST(entity_at_boundary) {
    setup_world(1000);
    // Entity right at cell boundary (64 units)
    edict_t *ent = create_entity_at(64, 64, 64, 32);

    edict_t *found[64];
    vec3_t mins = {48, 48, 48};
    vec3_t maxs = {80, 80, 80};
    int count = SV_AreaEdicts(mins, maxs, found, 64, AREA_SOLID);

    ASSERT(count == 1);
    PASS();
}

TEST(max_entities) {
    setup_world(5000);

    // Create many entities
    int created = 0;
    for (int i = 0; i < 800; i++) {
        float x = (i % 20) * 100;
        float y = ((i / 20) % 20) * 100;
        float z = (i / 400) * 100;
        if (create_entity_at(x, y, z, 16)) created++;
    }

    ASSERT(created > 700);  // Should create most of them
    PASS();
}

/* ============== MAIN ============== */

int main(int argc, char **argv) {
    printf("Spatial Hash / Entity Collision Tests\n");
    printf("=====================================\n\n");

#ifdef TEST_OPTIMIZED
    printf("Testing: OPTIMIZED version (spatial hash)\n");
#else
    printf("Testing: UNOPTIMIZED version (linked list)\n");
#endif
    printf("\n");

    printf("Correctness Tests:\n");
    RUN_TEST(basic_entity_creation);
    RUN_TEST(entity_query_single);
    RUN_TEST(entity_query_miss);
    RUN_TEST(entity_query_multiple);
    RUN_TEST(collision_detection);
    RUN_TEST(entity_movement);
    RUN_TEST(large_entity_spanning_cells);
    printf("\n");

    printf("Performance Tests:\n");
    RUN_TEST(performance_64_entities);
    RUN_TEST(performance_128_entities);
    RUN_TEST(performance_256_entities);
    RUN_TEST(performance_512_entities);
    printf("\n");

    printf("Scaling Test:\n");
    RUN_TEST(scaling_behavior);
    printf("\n");

    printf("Edge Case Tests:\n");
    RUN_TEST(empty_world_query);
    RUN_TEST(entity_at_origin);
    RUN_TEST(entity_at_boundary);
    RUN_TEST(max_entities);
    printf("\n");

    printf("=====================================\n");
    printf("Results: %d/%d tests passed\n", tests_passed, tests_run);

    return (tests_passed == tests_run) ? 0 : 1;
}
