#include "pinta.h"

#if PINTA_DEBUG

void pinta_debug_validate_handle_register(PintaCore *core, PintaHeapHandle *handle)
{
    PintaHeapHandle *root;

    for (root = core->heap_handles; root != NULL; root = root->next)
        pinta_assert(root != handle);
}

void pinta_debug_validate_handle_unregister(PintaCore *core, PintaHeapHandle *handle)
{
    PintaHeapHandle *root;
    u8 contains_handle = 0;

    for (root = core->heap_handles; root != NULL && !contains_handle; root = root->next)
        contains_handle = contains_handle || root == handle;

    pinta_assert(contains_handle);
}

#endif

void pinta_heap_object_init(PintaHeapObject *object, u8 block_kind, u8 block_flags, u32 block_length)
{
    pinta_assert(object != NULL);
    pinta_assert(block_length > 0);

    if (block_flags & PINTA_FLAG_ZERO)
        memset(object, 0, sizeof(PintaHeapObject)* block_length);

    object->block_kind = block_kind;
    object->block_flags = block_flags;
    object->block_tag = 0;
    object->block_length = block_length;
}

PintaHeap *pinta_heap_init(PintaNativeMemory* memory, u32 length_in_bytes)
{
    PintaHeap *heap;
    u32 elements;

    if (memory == NULL)
        return NULL;

    // do we need more memory? there is no point in trying to allocate memory for less than 3 objects
    if (length_in_bytes < (sizeof(PintaHeap) + 3 * sizeof(PintaHeapObject)))
        return NULL;

    heap = (PintaHeap*)pinta_memory_alloc(memory, sizeof(PintaHeap));

    elements = length_in_bytes / sizeof(PintaHeapObject);

    heap->start = (PintaHeapObject*)pinta_memory_alloc(memory, length_in_bytes);
    heap->top = heap->start;
    heap->end = (PintaHeapObject*)&heap->start[elements]; // past the heap (last valid element is at heap->end[-1])

    heap->next = NULL;
    heap->prev = NULL;

    heap->free = NULL;

    return heap;
}

PintaHeapObject *pinta_heap_alloc(PintaHeap *heap, u8 block_kind, u8 block_flags, u32 block_length)
{
    PintaHeapObject *result = NULL;
    PintaHeapObject *free, *next;
    PintaHeapObject *free_before;
    u32 available;
    u32 largest = 0;

    // try fast alloc first
    PintaHeapObject *top;

    pinta_assert(heap != NULL);

    top = &heap->top[block_length];
    if (top < heap->end && top > heap->top) // fast sliding alloc
    {
        result = heap->top;
        pinta_heap_object_init(result, block_kind, block_flags, block_length);

        heap->top = top;
        return result;
    }

    // we can't do a fast alloc, try looking through the free list
    if (heap->free == NULL) // we do not have free list, return failure
        return NULL;

    // NOTE that this is ONLY a hint, this function does not recalculate largest_free_block_length for performance, largest_free_block_length is calculated only on sweep (except if we walk whole heap and fail)
    // so this means, that there can only be blocks <= largest_free_block_length but there is not necessarilly a block of length largest_free_block_length
    if (heap->largest_free_block_length < block_length) // we do not have large enough free block
        return NULL;

    next = NULL;
    available = 0;
    largest = 0;

    // find a free block that is at least of requested length
    free = heap->free;
    while (free != NULL)
    {
        next = pinta_free_get_next(free);
        if (free->block_length >= block_length)
        {
            result = free;
            break;
        }

        if (free->block_length > largest)
            largest = free->block_length;

        free = next;
    }

    if (result) // if we found big enough free block, either split or consume it [fix prev/next pointers]
    {
        free_before = pinta_free_remove(heap, result);
        available = free->block_length - block_length;

        if (available != 0)
        {
            free = &result[block_length];
            pinta_free_insert_after(heap, free_before, free, available);
        }

        pinta_heap_object_init(result, block_kind, block_flags, block_length);
    }
    else // it seems that largest_free_block_length failed us
    {
        // when we allocate we do not fix the largest_free_block_length, nor we can have accurate value after compaction
        heap->largest_free_block_length = largest;
    }

    return result;
}

PintaHeapObject *pinta_free_get_next(PintaHeapObject *object)
{
    pinta_assert(object != NULL);
    pinta_assert(object->block_kind == PINTA_KIND_FREE);

    return object->data.free.next;
}

PintaHeapObject *pinta_free_get_prev(PintaHeapObject *object)
{
    pinta_assert(object != NULL);
    pinta_assert(object->block_kind == PINTA_KIND_FREE);

    return object->data.free.prev;
}

void pinta_free_set_next(PintaHeapObject *object, PintaHeapObject *value)
{
    pinta_assert(object != NULL);
    pinta_assert(object->block_kind == PINTA_KIND_FREE);

    object->data.free.next = value;
}

void pinta_free_set_prev(PintaHeapObject *object, PintaHeapObject *value)
{
    pinta_assert(object != NULL);
    pinta_assert(object->block_kind == PINTA_KIND_FREE);

    object->data.free.prev = value;
}

// returns pointer to node before item (NULL if item is first in list)
PintaHeapObject *pinta_free_remove(PintaHeap *heap, PintaHeapObject *item)
{
    PintaHeapObject *prev;
    PintaHeapObject *next;

    pinta_assert(heap != NULL);

    if (item == NULL)
        return NULL;

    pinta_assert(item->block_kind == PINTA_KIND_FREE);

    prev = pinta_free_get_prev(item);
    next = pinta_free_get_next(item);

    pinta_assert(prev == NULL || prev->block_kind == PINTA_KIND_FREE);
    pinta_assert(next == NULL || next->block_kind == PINTA_KIND_FREE);

    if (prev != NULL)
        pinta_free_set_next(prev, next);
    else
        heap->free = next;

    if (next != NULL)
        pinta_free_set_prev(next, prev);

    return prev;
}

void pinta_free_insert_after(PintaHeap *heap, PintaHeapObject *location, PintaHeapObject *item, u32 block_length)
{
    PintaHeapObject *next;

    pinta_assert(heap != NULL);
    pinta_assert(item != NULL);
    pinta_assert(heap->start <= item);
    pinta_assert(heap->top > item);
    pinta_assert(block_length > 0);

    pinta_assert(location == NULL || location->block_kind == PINTA_KIND_FREE);

#if PINTA_DEBUG
    memset(item, 0xDD, sizeof(PintaHeapObject)* block_length);
#endif

    if (location == NULL)
    {
        next = heap->free;
        if (next != NULL)
            pinta_free_set_prev(next, item);

        pinta_assert(next == NULL || next->block_kind == PINTA_KIND_FREE);

        pinta_heap_object_init(item, PINTA_KIND_FREE, PINTA_FLAG_NONE, block_length);
        pinta_free_set_next(item, next);
        pinta_free_set_prev(item, NULL);
        heap->free = item;
        return;
    }

    next = pinta_free_get_next(location);
    pinta_free_set_next(location, item);

    pinta_assert(next == NULL || next->block_kind == PINTA_KIND_FREE);

    if (next != NULL)
        pinta_free_set_prev(next, item);

    pinta_heap_object_init(item, PINTA_KIND_FREE, PINTA_FLAG_NONE, block_length);
    pinta_free_set_prev(item, location);
    pinta_free_set_next(item, next);
}

void pinta_free_merge(PintaHeap *heap, PintaHeapObject *free)
{
    PintaHeapObject *next;

    pinta_assert(heap != NULL);
    pinta_assert(free != NULL);
    pinta_assert(free->block_kind == PINTA_KIND_FREE);

    next = &free[free->block_length];
    if (next >= heap->top)
        return;

    if (next->block_kind != PINTA_KIND_FREE)
        return;

    pinta_assert(pinta_free_get_next(free) == next);

    pinta_free_remove(heap, next);
    free->block_length += next->block_length;

    if (free->block_length > heap->largest_free_block_length)
        heap->largest_free_block_length = free->block_length;

#if PINTA_DEBUG
    memset(next, 0xDD, sizeof(PintaHeapObject) * next->block_length);
#endif
}

PintaException pinta_heap_handle_register(PintaCore *core, PintaHeapHandle *heap_handle)
{
    PintaHeapHandle *root;

    pinta_assert(core != NULL);
    pinta_assert(heap_handle != NULL);
    
    pinta_debug_validate_handle_register(core, heap_handle);

    root = core->heap_handles;

    heap_handle->reference.reference = NULL;

    heap_handle->prev = NULL;
    heap_handle->next = root;

    if (root)
        root->prev = heap_handle;

    core->heap_handles = heap_handle;

    return PINTA_OK;
}

PintaException pinta_heap_handle_unregister(PintaCore *core, PintaHeapHandle *heap_handle)
{
    PintaHeapHandle *root, *next, *prev;

    pinta_assert(core != NULL);
    pinta_assert(heap_handle != NULL);

    pinta_debug_validate_handle_unregister(core, heap_handle);

    root = core->heap_handles;

    next = heap_handle->next;
    prev = heap_handle->prev;

    if (next != NULL)
        next->prev = prev;

    if (prev != NULL)
        prev->next = next;

    if (root == heap_handle)
        core->heap_handles = next;

    return PINTA_OK;
}
