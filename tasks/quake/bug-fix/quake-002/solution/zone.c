/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// zone.c - Zone memory allocator (FIXED VERSION)

#include "quakedef.h"

#define DYNAMIC_SIZE    0xc000
#define ZONEID          0x1d4a11
#define MINFRAGMENT     64

typedef struct memblock_s
{
    int     size;           // including the header and possibly tiny fragments
    int     tag;            // a tag of 0 is a free block
    int     id;             // should be ZONEID
    struct memblock_s *next, *prev;
    int     pad;            // pad to 64 bit boundary
} memblock_t;

typedef struct
{
    int         size;       // total bytes malloced, including header
    memblock_t  blocklist;  // start / end cap for linked list
    memblock_t  *rover;
} memzone_t;

memzone_t *mainzone;

/* Forward declarations */
void Z_CheckHeap(void);
void *Z_TagMalloc(int size, int tag);
void Z_FreeTags(int tag);

/*
========================
Z_ClearZone
========================
*/
void Z_ClearZone(memzone_t *zone, int size)
{
    memblock_t *block;

    // set the entire zone to one free block
    zone->blocklist.next = zone->blocklist.prev = block =
        (memblock_t *)((byte *)zone + sizeof(memzone_t));
    zone->blocklist.tag = 1;    // in use block (sentinel)
    zone->blocklist.id = 0;
    zone->blocklist.size = 0;
    zone->rover = block;

    block->prev = block->next = &zone->blocklist;
    block->tag = 0;             // free block
    block->id = ZONEID;
    block->size = size - sizeof(memzone_t);
}


/*
========================
Z_Free

FIXED: Properly handle all cases where rover might become invalid during
block coalescing. Also added size overflow check.
========================
*/
void Z_Free(void *ptr)
{
    memblock_t *block, *other;

    if (!ptr)
        Sys_Error("Z_Free: NULL pointer");

    block = (memblock_t *)((byte *)ptr - sizeof(memblock_t));
    if (block->id != ZONEID)
        Sys_Error("Z_Free: freed a pointer without ZONEID");
    if (block->tag == 0)
        Sys_Error("Z_Free: freed a freed pointer");

    block->tag = 0;     // mark as free

    other = block->prev;
    if (!other->tag)
    {   // merge with previous free block
        /* FIX: Check for size overflow before merging */
        if (other->size > 0 && block->size > 0 &&
            other->size <= INT_MAX - block->size)
        {
            other->size += block->size;
            other->next = block->next;
            other->next->prev = other;

            /* FIX: Update rover if it pointed to EITHER block being merged */
            if (block == mainzone->rover || other == mainzone->rover)
                mainzone->rover = other;

            block = other;
        }
    }

    other = block->next;
    if (!other->tag)
    {   // merge the next free block onto the end
        /* FIX: Check for size overflow before merging */
        if (block->size > 0 && other->size > 0 &&
            block->size <= INT_MAX - other->size)
        {
            block->size += other->size;
            block->next = other->next;
            block->next->prev = block;

            /* FIX: Update rover if it pointed to the block being absorbed */
            if (other == mainzone->rover)
                mainzone->rover = block;
        }
    }
}


/*
========================
Z_Malloc
========================
*/
void *Z_Malloc(int size)
{
    void *buf;

    Z_CheckHeap();  // DEBUG
    buf = Z_TagMalloc(size, 1);
    if (!buf)
        Sys_Error("Z_Malloc: failed on allocation of %i bytes", size);
    Q_memset(buf, 0, size);

    return buf;
}

void *Z_TagMalloc(int size, int tag)
{
    int         extra;
    memblock_t  *start, *rover, *newblock, *base;

    if (!tag)
        Sys_Error("Z_TagMalloc: tried to use a 0 tag");

    /* FIX: Check for size overflow before adjustments */
    if (size < 0 || size > INT_MAX - sizeof(memblock_t) - 8)
        return NULL;

    // scan through the block list looking for the first free block
    // of sufficient size
    size += sizeof(memblock_t); // account for size of block header
    size += 4;                  // space for memory trash tester
    size = (size + 7) & ~7;     // align to 8-byte boundary

    base = rover = mainzone->rover;
    start = base->prev;

    do
    {
        if (rover == start) // scanned all the way around the list
            return NULL;
        if (rover->tag)
            base = rover = rover->next;
        else
            rover = rover->next;
    } while (base->tag || base->size < size);

    // found a block big enough
    extra = base->size - size;
    if (extra > MINFRAGMENT)
    {   // there will be a free fragment after the allocated block
        newblock = (memblock_t *)((byte *)base + size);
        newblock->size = extra;
        newblock->tag = 0;          // free block
        newblock->prev = base;
        newblock->id = ZONEID;
        newblock->next = base->next;
        newblock->next->prev = newblock;
        base->next = newblock;
        base->size = size;
    }

    base->tag = tag;                // no longer a free block

    mainzone->rover = base->next;   // next allocation will start looking here

    base->id = ZONEID;

    // marker for memory trash testing
    *(int *)((byte *)base + base->size - 4) = ZONEID;

    return (void *)((byte *)base + sizeof(memblock_t));
}


/*
========================
Z_Print
========================
*/
void Z_Print(memzone_t *zone)
{
    memblock_t *block;

    Con_Printf("zone size: %i  location: %p\n", mainzone->size, mainzone);

    for (block = zone->blocklist.next; ; block = block->next)
    {
        Con_Printf("block:%p    size:%7i    tag:%3i\n",
            block, block->size, block->tag);

        if (block->next == &zone->blocklist)
            break;          // all blocks have been hit
        if ((byte *)block + block->size != (byte *)block->next)
            Con_Printf("ERROR: block size does not touch the next block\n");
        if (block->next->prev != block)
            Con_Printf("ERROR: next block doesn't have proper back link\n");
        if (!block->tag && !block->next->tag)
            Con_Printf("ERROR: two consecutive free blocks\n");
    }
}


/*
========================
Z_CheckHeap

FIX: Also validates that rover points to a valid block in the list
========================
*/
void Z_CheckHeap(void)
{
    memblock_t *block;
    int rover_found = 0;

    for (block = mainzone->blocklist.next; ; block = block->next)
    {
        /* FIX: Track if we find the rover in the list */
        if (block == mainzone->rover)
            rover_found = 1;

        if (block->next == &mainzone->blocklist)
            break;          // all blocks have been hit
        if ((byte *)block + block->size != (byte *)block->next)
            Sys_Error("Z_CheckHeap: block size does not touch the next block\n");
        if (block->next->prev != block)
            Sys_Error("Z_CheckHeap: next block doesn't have proper back link\n");
        if (!block->tag && !block->next->tag)
            Sys_Error("Z_CheckHeap: two consecutive free blocks\n");
    }

    /* FIX: Verify rover is valid */
    if (!rover_found && mainzone->rover != &mainzone->blocklist)
        Sys_Error("Z_CheckHeap: rover points to invalid block\n");
}


/*
========================
Z_FreeTags
========================
*/
void Z_FreeTags(int tag)
{
    memblock_t *block, *next;

    for (block = mainzone->blocklist.next;
         block != &mainzone->blocklist;
         block = next)
    {
        next = block->next;
        if (block->tag == tag)
            Z_Free((void *)((byte *)block + sizeof(memblock_t)));
    }
}
