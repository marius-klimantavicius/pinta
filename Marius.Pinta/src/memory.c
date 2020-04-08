#include "pinta.h"

PintaNativeMemory *pinta_memory_init(void *memory, u32 length)
{
    PintaNativeMemory *result;
    u8 *start = (u8*)memory;
    u8 *end;
    uintptr_t memory_pointer;

    if (memory == NULL)
        return NULL;

    memory_pointer = (uintptr_t)memory;
    if ((memory_pointer & 0x3F) != 0)
    {
        uintptr_t remainder = 0x40 - (memory_pointer & 0x3F);
        
        length -= (u32)remainder;
        memory_pointer = memory_pointer + remainder;
    }

    if (length < (sizeof(PintaNativeMemory) + sizeof(PintaNativeBlock)))
        return NULL;

    start = (u8*)memory_pointer;
    end = &start[length];

    result = (PintaNativeMemory*)memory_pointer;
    result->start = (PintaNativeBlock*)&result[1];
    result->end = (PintaNativeBlock*)end;
    result->top = result->start;
    result->free = NULL;

    return result;
}

PintaNativeBlock *pinta_memory_remove(PintaNativeMemory *memory, PintaNativeBlock *item)
{
    PintaNativeBlock *prev;
    PintaNativeBlock *next;

    pinta_assert(memory != NULL);

    if (item == NULL)
        return NULL;

    prev = item->data.free.prev;
    next = item->data.free.next;

    if (prev != NULL)
        prev->data.free.next = next;
    else
        memory->free = next;

    if (next != NULL)
        next->data.free.prev = prev;

    return prev;
}

void pinta_memory_insert(PintaNativeMemory *memory, PintaNativeBlock *location, PintaNativeBlock *item, u32 block_length)
{
    PintaNativeBlock *next;

    pinta_assert(memory != NULL);
    pinta_assert(item != NULL);
    pinta_assert(memory->start <= item);
    pinta_assert(memory->top > item);
    pinta_assert(block_length > 0);

#if PINTA_DEBUG
    memset(item, 0xEB, sizeof(PintaNativeBlock)* block_length);
#endif

    if (location == NULL)
    {
        next = memory->free;
        if (next != NULL)
            next->data.free.prev = item;

        item->cookie = PINTA_MEMORY_COOKIE_FREE;
        item->length = block_length;
        item->data.free.next = next;
        item->data.free.prev = NULL;

        memory->free = item;
        return;
    }

    next = location->data.free.next;
    location->data.free.next = item;

    if (next != NULL)
        next->data.free.prev = item;

    item->cookie = PINTA_MEMORY_COOKIE_FREE;
    item->length = block_length;
    item->data.free.prev = location;
    item->data.free.next = next;
}

void *pinta_memory_alloc(PintaNativeMemory *memory, u32 length)
{
    u32 block_length;
    PintaNativeBlock *before;
    PintaNativeBlock *block;
    PintaNativeBlock *block_end;

    pinta_assert(memory != NULL);
    pinta_assert(memory->start != NULL);
    pinta_assert(memory->end != NULL);
    pinta_assert(memory->start <= memory->end);
    pinta_assert(memory->top != NULL);
    pinta_assert(memory->start <= memory->top);
    pinta_assert(memory->top <= memory->end);
    pinta_assert(memory->free == NULL || (memory->free >= memory->start && memory->free < memory->end));

    block_length = (length + sizeof(PintaNativeBlock) - 1) / sizeof(PintaNativeBlock);
    block_length += 1; // for header

    block = memory->top;
    block_end = &block[block_length];

    if (block_end < memory->start || block_end > memory->end)
    {
        if (memory->free == NULL)
            return NULL;

        for (block = memory->free; block != NULL; block = block->data.free.next)
        {
            if (block->length >= block_length)
                break;
        }

        if (block == NULL)
            return NULL;

        block_end = &block[block_length];

        before = pinta_memory_remove(memory, block);
        if (block->length > block_length)
            pinta_memory_insert(memory, before, block_end, block->length - block_length);
    }

    if (block != NULL)
    {
        block->cookie = PINTA_MEMORY_COOKIE_ALIVE;
        block->length = block_length;
        block->data.memory = (void*)&block[1];

        memory->top = block_end;

        return block->data.memory;
    }

    return NULL;
}

void *pinta_memory_realloc(PintaNativeMemory *memory, void *item, u32 length)
{
    PintaNativeBlock *block;
    PintaNativeBlock *end;
    PintaNativeBlock *current;
    PintaNativeBlock *prev;
    void *destination;
    u32 block_length;

    if (item == NULL)
        return pinta_memory_alloc(memory, length);

    block = (PintaNativeBlock*)item;
    block = &block[-1];

    block_length = (length + sizeof(PintaNativeBlock) - 1) / sizeof(PintaNativeBlock);
    block_length += 1; // for header

    if (block->length == block_length)
        return item;

    if (block->length > block_length)
    {
        end = &block[block_length];

        for (current = memory->free; current != NULL; current = current->data.free.next)
        {
            if (current > block)
                break;
        }

        if (current == NULL)
        {
            pinta_memory_insert(memory, NULL, end, block->length - block_length);
        }
        else if (current == &block[block->length])
        {
            u32 free_length = block->length - block_length + current->length;
            prev = pinta_memory_remove(memory, current);
            pinta_memory_insert(memory, prev, end, free_length);
        }
        else
        {
            pinta_memory_insert(memory, current->data.free.prev, end, block->length - block_length);
        }

        block->length = block_length;
        return item;
    }

    destination = pinta_memory_alloc(memory, length);
    if (destination == NULL)
        return NULL;

    memmove(destination, item, (block_length - 1) * sizeof(PintaNativeBlock));
    pinta_memory_free(memory, item);

    return destination;
}

void pinta_memory_free(PintaNativeMemory *memory, void *item)
{
    PintaNativeBlock *block;
    PintaNativeBlock *current;
    PintaNativeBlock *end;
    PintaNativeBlock *next;
    u32 block_length;

    pinta_assert(memory != NULL);
    pinta_assert(memory->start != NULL);
    pinta_assert(memory->end != NULL);
    pinta_assert(memory->start <= memory->end);
    pinta_assert(memory->start <= memory->top);
    pinta_assert(memory->top <= memory->end);
    pinta_assert(memory->free == NULL || (memory->free >= memory->start && memory->free < memory->end));

    if (item == NULL)
        return;

    block = (PintaNativeBlock*)item;
    block = &block[-1];

    pinta_assert(memory->start <= block);
    pinta_assert(block < memory->end);
    pinta_assert(block < memory->top);
    pinta_assert(block->cookie == PINTA_MEMORY_COOKIE_ALIVE);
    pinta_assert(block->data.memory == item);

    if (memory->free == NULL)
    {
        pinta_memory_insert(memory, NULL, block, block->length);
        return;
    }
    else if (memory->free > block)
    {
        next = memory->free;

        end = &block[block->length];
        if (end == next)
        {
            block_length = block->length + next->length;
            pinta_memory_remove(memory, next);
        }
        else
        {
            block_length = block->length;
        }

        pinta_memory_insert(memory, NULL, block, block_length);
        return;
    }

    for (current = memory->free; current != NULL; current = current->data.free.next)
    {
        pinta_assert(current->cookie == PINTA_MEMORY_COOKIE_FREE);
        pinta_assert(current->data.free.prev == NULL || current > current->data.free.prev);
        pinta_assert(current->data.free.next == NULL || current < current->data.free.next);

        next = current->data.free.next;
        if (current < block && (next == NULL || next > block))
        {
            break;
        }
    }

    pinta_assert(current != NULL);

    next = current->data.free.next;

    // 1. Merge with current if necessary
    end = &current[current->length];
    if (end == block)
    {
        current->length += block->length;

        // 2. Merge current with next if necessary
        end = &current[current->length];
        if (next != NULL && end == next)
        {
            block_length = current->length + next->length;
            pinta_memory_remove(memory, next);
            current->length = block_length;
        }
    }
    else
    {
        end = &block[block->length];
        if (next != NULL && end == next)
        {
            pinta_memory_remove(memory, next);
        }

        pinta_memory_insert(memory, current, block, block->length);
    }

}
