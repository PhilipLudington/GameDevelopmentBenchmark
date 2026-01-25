/**
 * Stub implementation for julius-033 storage building undo test
 *
 * This provides mock storage building linked list functions for testing.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Storage building structure with doubly-linked list */
typedef struct storage_building {
    int building_id;
    int capacity;
    struct storage_building *prev;
    struct storage_building *next;
} storage_building;

/* List head */
static storage_building *storage_list_head = NULL;

/* Create a new storage building */
storage_building *storage_building_create(int building_id, int capacity)
{
    storage_building *storage = malloc(sizeof(storage_building));
    if (!storage) return NULL;

    storage->building_id = building_id;
    storage->capacity = capacity;
    storage->prev = NULL;
    storage->next = NULL;

    return storage;
}

/* Add storage building to the list */
void storage_building_add(storage_building *storage)
{
    storage->prev = NULL;
    storage->next = storage_list_head;

    if (storage_list_head) {
        storage_list_head->prev = storage;
    }

    storage_list_head = storage;
}

/**
 * Remove storage building from the list
 *
 * BUGGY version: Does not update next->prev pointer
 * FIXED version: Updates both prev->next and next->prev
 */
void storage_building_remove(storage_building *storage)
{
    if (storage->prev) {
        storage->prev->next = storage->next;
    } else {
        storage_list_head = storage->next;
    }

    /* FIXED: Update next building's prev pointer */
    if (storage->next) {
        storage->next->prev = storage->prev;
    }

    free(storage);
}

/* Get list head for testing */
storage_building *storage_get_list_head(void)
{
    return storage_list_head;
}

/* Clear all storage buildings */
void storage_clear_all(void)
{
    storage_building *current = storage_list_head;
    while (current) {
        storage_building *next = current->next;
        free(current);
        current = next;
    }
    storage_list_head = NULL;
}

/* Count buildings in forward direction */
int storage_count_forward(void)
{
    int count = 0;
    storage_building *current = storage_list_head;
    while (current) {
        count++;
        current = current->next;
    }
    return count;
}

/* Count buildings from a node going backward */
int storage_count_backward_from(storage_building *start)
{
    int count = 0;
    storage_building *current = start;
    while (current) {
        count++;
        current = current->prev;
    }
    return count;
}

/* Find last building in list */
storage_building *storage_find_last(void)
{
    storage_building *current = storage_list_head;
    if (!current) return NULL;

    while (current->next) {
        current = current->next;
    }
    return current;
}

/* Find building by ID */
storage_building *storage_find_by_id(int building_id)
{
    storage_building *current = storage_list_head;
    while (current) {
        if (current->building_id == building_id) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

/* Validate list integrity */
int storage_validate_list(void)
{
    storage_building *current = storage_list_head;
    storage_building *prev = NULL;

    while (current) {
        /* Check that prev pointer is correct */
        if (current->prev != prev) {
            return 0; /* Corruption detected */
        }
        prev = current;
        current = current->next;
    }

    return 1; /* List is valid */
}
