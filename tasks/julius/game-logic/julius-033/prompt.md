# Bug Report: Storage Building Undo Linking Corruption

## Summary

When a player places a storage building (warehouse or granary) and then undoes the placement, the storage building linked list becomes corrupted. The previous and next pointers are not properly updated, causing some buildings to be skipped during resource distribution.

## Environment

- Project: Julius (Caesar III reimplementation)
- File: `src/building/storage.c`
- Severity: Medium (gameplay integrity issue, resource distribution broken)

## Bug Description

Storage buildings (warehouses and granaries) are maintained in a doubly-linked list for efficient iteration during resource distribution. When a storage building is placed, it's added to the list. When placement is undone, the building should be removed from the list.

The bug is in the removal code:

```c
typedef struct storage_building {
    int building_id;
    int capacity;
    struct storage_building *prev;
    struct storage_building *next;
} storage_building;

static storage_building *storage_list_head = NULL;

void storage_building_remove(storage_building *storage)
{
    if (storage->prev) {
        storage->prev->next = storage->next;
    } else {
        storage_list_head = storage->next;
    }

    // BUG: Missing update of next->prev pointer!
    // The next building still points back to the removed building

    free(storage);
}
```

After removal, if another building was linked after the removed one, its `prev` pointer still points to freed memory. This causes:
1. Resource distribution to skip buildings
2. Potential crashes when traversing backwards
3. Memory corruption when the freed memory is reused

## Steps to Reproduce

1. Build a warehouse (A)
2. Build a second warehouse (B) - B is linked after A
3. Build a third warehouse (C) - C is linked after B
4. Undo building B
5. B is removed but C->prev still points to B
6. Backward traversal from C crashes or skips to wrong building

## Expected Behavior

When removing a storage building from the linked list:
1. Update previous building's next pointer
2. Update next building's prev pointer (MISSING!)
3. Update list head if removing first element
4. Free the removed building

## Current Behavior

Only the previous building's next pointer is updated. The next building's prev pointer still references the removed (freed) building.

## Relevant Code

Look at `src/building/storage.c`:
- `storage_building_remove()` function
- `storage_building_add()` function for reference
- List traversal functions that might expose the bug

## Suggested Fix Approach

Add the missing prev pointer update:

```c
void storage_building_remove(storage_building *storage)
{
    if (storage->prev) {
        storage->prev->next = storage->next;
    } else {
        storage_list_head = storage->next;
    }

    // ADDED: Update next building's prev pointer
    if (storage->next) {
        storage->next->prev = storage->prev;
    }

    free(storage);
}
```

## Your Task

Fix the linked list corruption by properly updating the next building's prev pointer during removal. Your fix should:

1. Update both prev->next and next->prev when removing from middle
2. Handle removal from head correctly
3. Handle removal from tail correctly
4. Not crash when removing the only element
5. Maintain list integrity for all edge cases

Provide your fix as a unified diff (patch).
