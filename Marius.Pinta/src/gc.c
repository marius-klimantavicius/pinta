#include "pinta.h"

void pinta_gc_mark_object(PintaCore *core, PintaHeapObject *object)
{
    PintaHeapObjectWalker walk;
    PintaHeapObject *internal;
    PintaType *type;
    PintaHeapObjectWalkerState state;

    if (object == NULL)
        return;

    if (object->block_flags & PINTA_FLAG_ALIVE)
        return;

    if (!(object->block_flags & PINTA_FLAG_KEEP_ALIVE))
        object->block_flags |= PINTA_FLAG_ALIVE;

    type = pinta_core_get_object_type(core, object);
    walk = type->gc_walk;
    if (walk != NULL)
    {
        state.index = 0;
        state.field = 0;

        while (walk(object, &state, &internal))
        {
            if (internal == NULL)
                continue;

            pinta_gc_mark_object(core, internal);
        }
    }
}

void pinta_gc_mark_range(PintaCore *core, PintaReference *start, PintaReference *end)
{
    PintaReference *current = start;

    pinta_assert(core != NULL);

    if (current == NULL)
        return;

    for (; current < end; current++)
    {
        if (current->reference == NULL)
            continue;

        pinta_gc_mark_object(core, current->reference);
    }
}

void pinta_gc_mark(PintaCore *core)
{
    PintaModuleDomain *domain;
    PintaNativeFrame *native;
    PintaHeapHandle *heap_handle;
    PintaThread *thread;
    PintaHeap *heap;
    PintaHeapObject *current;

    pinta_assert(core != NULL);
    pinta_gc_mark_object(core, core->externals.reference);
    pinta_gc_mark_object(core, core->output.reference);

    for (domain = core->domains; domain != NULL; domain = domain->next)
    {
        pinta_gc_mark_object(core, domain->global_object.reference);
        pinta_gc_mark_object(core, domain->globals.reference);
        pinta_gc_mark_object(core, domain->strings.reference);
    }

    for (native = core->native; native != NULL; native = native->next)
    {
        pinta_gc_mark_range(core, native->references, &native->references[native->length]);
    }

    for (heap_handle = core->heap_handles; heap_handle != NULL; heap_handle = heap_handle->next)
    {
        pinta_gc_mark_object(core, heap_handle->reference.reference);
    }

    for (thread = core->threads; thread != NULL; thread = thread->next)
    {
        PintaStackFrame *frame;

        for (frame = thread->frame; frame != NULL; frame = frame->prev)
        {
            pinta_gc_mark_object(core, frame->function_this.reference);
            pinta_gc_mark_object(core, frame->function_closure.reference);
            pinta_gc_mark_object(core, frame->function_locals.reference);
            pinta_gc_mark_object(core, frame->function_arguments.reference);
            pinta_gc_mark_range(core, frame->stack_start, &frame->stack[1]);
        }
    }

    for (heap = core->heap; heap != NULL; heap = heap->next)
    {
        for (current = heap->start; current < heap->top; current = &current[current->block_length])
        {
            if (current->block_kind == PINTA_KIND_FREE)
                continue;

            if (current->block_flags & PINTA_FLAG_KEEP_ALIVE)
                pinta_gc_mark_object(core, current);
        }
    }
}

u32 pinta_debug_get_alive_count(PintaHeap *heap)
{
    u32 result = 0;
    PintaHeapObject *current;

    for (current = heap->start; current < heap->top; current = &current[current->block_length])
    {
        if (current->block_kind != PINTA_KIND_FREE)
            result++;
    }

    return result;
}

#if PINTA_DEBUG
void pinta_debug_validate_heap(PintaHeap *heap)
{
    PintaHeapObject *last_free = NULL;
    PintaHeapObject *free_next, *free_prev;
    PintaHeapObject *last_alive = NULL;
    u32 largest_free = 0;

    PintaHeapObject *current;
    for (current = heap->start; current < heap->top; current = &current[current->block_length])
    {
        if (current->block_kind == PINTA_KIND_FREE)
        {
            free_prev = pinta_free_get_prev(current);
            free_next = pinta_free_get_next(current);

            pinta_assert(free_prev == NULL || free_prev < current);
            pinta_assert(free_next == NULL || free_next > current);

            pinta_assert(free_prev == NULL || (free_prev->block_kind == PINTA_KIND_FREE && pinta_free_get_next(free_prev) == current));
            pinta_assert(free_next == NULL || (free_next->block_kind == PINTA_KIND_FREE && pinta_free_get_prev(free_next) == current && free_next < heap->top));
            pinta_assert(last_free == NULL || (&last_free[last_free->block_length] != current));
            pinta_assert(last_free == free_prev);

            if (current->block_length > largest_free)
                largest_free = current->block_length;

            last_free = current;
        }
        else if (current->block_kind < PINTA_KIND_LENGTH)
        {
            last_alive = current;
        }
        else
        {
            pinta_assert(0 && "Unknown object type");
            last_alive = current;
        }
    }

    pinta_assert(last_alive == NULL || heap->top == &last_alive[last_alive->block_length]);
    pinta_assert(last_free == NULL || pinta_free_get_next(last_free) == NULL);
}

#endif

void pinta_gc_sweep(PintaHeap *heap)
{
    PintaHeapObject *current;
    PintaHeapObject *next;
    PintaHeapObject *last_free = NULL, *before_free = NULL;
    PintaHeapObject *alive_last = NULL;

#if PINTA_DEBUG
    u32 block_length;
#endif

    pinta_assert(heap != NULL);
    pinta_assert(heap->start != NULL);
    pinta_assert(heap->start <= heap->top);
    pinta_assert(heap->start < heap->end);
    pinta_assert(heap->top <= heap->end);

    heap->largest_free_block_length = 0;

    current = heap->start;
    // this function recreates the free list, so until this function returns, the free list might be in invalid state
    // cannot change to for-loop because heap is modified in this function and we rely on `next` to hold next reference
    while (current < heap->top)
    {
        next = &current[current->block_length]; // get next pointer before doing any shenanigans

        if (current->block_kind != PINTA_KIND_FREE && (current->block_flags & (PINTA_FLAG_ALIVE | PINTA_FLAG_KEEP_ALIVE)))
        {
            current->block_flags &= ~PINTA_FLAG_ALIVE; // clear the alive flag (note that this does NOT clear the KEEPALIVE)
            alive_last = current;
            before_free = last_free;
        }
        else
        {
#if PINTA_DEBUG
            block_length = current->block_length;
            memset(current, 0xDD, sizeof(PintaHeapObject)* block_length);
            current->block_length = block_length;
#endif

            current->block_kind = PINTA_KIND_FREE;
            pinta_free_set_next(current, NULL);
            pinta_free_set_prev(current, NULL);

            if (last_free != NULL)
            {
                if (&last_free[last_free->block_length] == current) // current block is free and next to last free block - merge; and by merge I mean increase block length by current->block_length
                {
                    last_free->block_length += current->block_length;
                    if (last_free->block_length > heap->largest_free_block_length)
                        heap->largest_free_block_length = last_free->block_length;

#if PINTA_DEBUG
                    memset(current, 0xDD, sizeof(PintaHeapObject) * current->block_length);
#endif
                }
                else
                {
                    pinta_free_set_next(last_free, current);
                    pinta_free_set_prev(current, last_free);

                    last_free = current;

                    if (last_free->block_length > heap->largest_free_block_length)
                        heap->largest_free_block_length = last_free->block_length;
                }
            }
            else
            {
                last_free = current;
                pinta_free_set_prev(current, NULL);

                heap->free = last_free;
                heap->largest_free_block_length = last_free->block_length;
            }
        }

        current = next;
    }

    if (alive_last != NULL)
        heap->top = &alive_last[alive_last->block_length];
    else
        heap->top = heap->start;

    if (heap->free != NULL)
    {
        if (heap->free >= heap->top)
            heap->free = NULL;

        pinta_assert(heap->free == NULL || before_free != NULL);
        if (before_free != NULL && pinta_free_get_next(before_free) != NULL)
        {
            pinta_assert(pinta_free_get_next(before_free) >= heap->top);
            pinta_free_set_next(before_free, NULL);
        }
    }

    pinta_debug_validate_heap(heap);
}

void pinta_gc_clear_weak(PintaCore *core)
{
    PintaHeapObject *alive;
    PintaHeapObject *current;
    PintaHeapObject *target;

    pinta_assert(core != NULL);

    alive = NULL;
    current = core->weak.reference;
    while (current != NULL)
    {
        pinta_assert(current->block_kind == PINTA_KIND_WEAK);

        if (current->block_flags & (PINTA_FLAG_ALIVE | PINTA_FLAG_KEEP_ALIVE))
        {
            target = pinta_weak_get_target(current);
            if (target != NULL && !(target->block_flags & (PINTA_FLAG_ALIVE | PINTA_FLAG_KEEP_ALIVE)))
                pinta_weak_set_target(current, NULL);

            if (alive != NULL)
                pinta_weak_set_next(alive, current);
            alive = current;
        }

        current = pinta_weak_get_next(current);
    }

    if (alive != NULL)
    {
        pinta_weak_set_next(alive, NULL);
        core->weak.reference = alive;
    }
}

i32 pinta_gc_relocate_compare(const void *key, const void *value)
{
    PintaHeapObject *object = (PintaHeapObject*)key;
    PintaHeapReloc *reloc = (PintaHeapReloc*)value;

    if (object >= reloc->start && object < reloc->end)
        return 0;

    if (object < reloc->start)
        return -1;

    return 1;
}

#if PINTA_DEBUG
void pinta_gc_relocate_validate(u8 kind, PintaHeapObject *object, PintaHeapReloc *reloc, u32 count)
{
    PINTA_UNUSED(kind);
    PINTA_UNUSED(object);
    PINTA_UNUSED(reloc);
    PINTA_UNUSED(count);

    pinta_assert(reloc != NULL);
    pinta_assert(count > 0);
    pinta_assert(object != NULL);
    pinta_assert(object->block_kind == kind);
}
#endif

PintaHeapReloc *pinta_gc_relocate_find(void *value, PintaHeapReloc *reloc, u32 count)
{
    PintaHeapReloc *try;
    i32 sign;

    while (count > 0)
    {
        try = &reloc[count / 2];
        sign = pinta_gc_relocate_compare(value, try);

        if (!sign)
        {
            return try;
        }
        else if (count == 1)
        {
            break;
        }
        else if (sign < 0)
        {
            count /= 2;
        }
        else
        {
            reloc = try;
            count -= count / 2;
        }
    }
    return NULL;
}

void pinta_gc_relocate_single(PintaReference *value, PintaHeapReloc *reloc, u32 count)
{
    PintaHeapReloc *entry;

    pinta_assert(reloc != NULL);
    pinta_assert(count > 0);

    if (value == NULL)
        return;

    if (value->reference == NULL)
        return;

    entry = pinta_gc_relocate_find(value->reference, reloc, count);
    if (entry != NULL)
        value->reference = value->reference - entry->offset;
}

void pinta_gc_relocate_heap(PintaCore *core, PintaHeap *heap, PintaHeapReloc *reloc, u32 count)
{
    PintaHeapObject *current;
    PintaHeapObjectRelocate relocate;
    PintaType *type;

    pinta_assert(heap != NULL);
    pinta_assert(reloc != NULL);
    pinta_assert(count > 0);

    if (heap == NULL)
        return;

    for (current = heap->start; current < heap->top; current = &current[current->block_length])
    {
        if (current->block_kind == PINTA_KIND_FREE)
            continue;

        type = pinta_core_get_object_type(core, current);
        relocate = type->gc_relocate;
        if (relocate != NULL)
            relocate(current, reloc, count);
    }
}

void pinta_gc_relocate_range(PintaReference *start, PintaReference *end, PintaHeapReloc *reloc, u32 count)
{
    pinta_assert(reloc != NULL);
    pinta_assert(count > 0);

    if (start == NULL)
        return;

    for (; start < end; start++)
        pinta_gc_relocate_single(start, reloc, count);
}

void pinta_gc_relocate_frame(PintaStackFrame *frame, PintaHeapReloc *reloc, u32 count)
{
    pinta_assert(reloc != NULL);
    pinta_assert(count > 0);

    if (frame != NULL)
    {
        pinta_gc_relocate_single(&frame->function_this, reloc, count);
        pinta_gc_relocate_single(&frame->function_closure, reloc, count);
        pinta_gc_relocate_single(&frame->function_arguments, reloc, count);
        pinta_gc_relocate_single(&frame->function_locals, reloc, count);
        pinta_gc_relocate_range(frame->stack_start, &frame->stack[1], reloc, count);
    }
}

void pinta_gc_relocate_native(PintaNativeFrame *native, PintaHeapReloc *reloc, u32 count)
{
    pinta_assert(reloc != NULL);
    pinta_assert(count > 0);

    if (native != NULL && native->references != NULL)
        pinta_gc_relocate_range(native->references, &native->references[native->length], reloc, count);
}

void pinta_gc_relocate(PintaCore *core, PintaHeapReloc *reloc, u32 count)
{
    PintaModuleDomain *domain;
    PintaHeap *heap;
    PintaThread *current;
    PintaStackFrame *frame;
    PintaNativeFrame *native;
    PintaHeapHandle *heap_handle;

    pinta_assert(core != NULL);
    pinta_assert(reloc != NULL);
    pinta_assert(count > 0);

    if (core->weak.reference != NULL)
        pinta_gc_relocate_single(&core->weak, reloc, count);

    if (core->externals.reference != NULL)
        pinta_gc_relocate_single(&core->externals, reloc, count);

    if (core->output.reference != NULL)
        pinta_gc_relocate_single(&core->output, reloc, count);

    for (domain = core->domains; domain != NULL; domain = domain->next)
    {
        if (domain->global_object.reference != NULL)
            pinta_gc_relocate_single(&domain->global_object, reloc, count);

        if (domain->globals.reference != NULL)
            pinta_gc_relocate_single(&domain->globals, reloc, count);

        if (domain->strings.reference != NULL)
            pinta_gc_relocate_single(&domain->strings, reloc, count);
    }

    for (native = core->native; native != NULL; native = native->next)
        pinta_gc_relocate_native(native, reloc, count);

    for (heap_handle = core->heap_handles; heap_handle != NULL; heap_handle = heap_handle->next)
        pinta_gc_relocate_single(&heap_handle->reference, reloc, count);

    for (heap = core->heap; heap != NULL; heap = heap->next)
        pinta_gc_relocate_heap(core, heap, reloc, count);

    for (current = core->threads; current != NULL; current = current->next)
    {
        for (frame = current->frame; frame != NULL; frame = frame->prev)
            pinta_gc_relocate_frame(frame, reloc, count);
    }
}

void pinta_gc_compact_worker(PintaCore *core, PintaHeap *heap, PintaHeapReloc *reloc, u32 reloc_count)
{
    u32 count = 0;

    u32 available = 0;
    PintaHeapObject *next = NULL, *alive_last = NULL;
    PintaHeapObject *free = NULL, *free_last = NULL, *free_before = NULL;

    PintaHeapObject *current;

    pinta_assert(core != NULL);
    pinta_assert(heap != NULL);
    pinta_assert(reloc != NULL);
    pinta_assert(reloc_count > 0);

#if PINTA_DEBUG
    //pinta_debug_print("compacting...\n");
#endif

    pinta_debug_validate_heap(heap);

    current = heap->start;
    // cannot change this to for because heap is modified and we rely on `next` to hold next pointer
    while (current < heap->top)
    {
        next = &current[current->block_length]; // get next before we start modifying object

        // at this point the heap is in valid state
        if (count >= reloc_count)
        {
            pinta_gc_relocate(core, reloc, count);
            count = 0;
        }

        if (current->block_kind == PINTA_KIND_FREE)
        {
            // this is a free block
            // basically we just skip it, next alive block that fits can go here
            if (free_last != NULL && &free_last[free_last->block_length] == current) // we can merge adjacent free blocks
            {
                while (current < heap->top && current->block_kind == PINTA_KIND_FREE && &free_last[free_last->block_length] == current)
                {
                    pinta_free_merge(heap, free_last);
                    current = &free_last[free_last->block_length];
                }

                next = current;
            }
            else
            {
                free_last = current;
            }
        }
        else if (!(current->block_flags & PINTA_FLAG_PINNED)) // this is alive and can be moved
        {
            if (free_last == NULL) // we do not have a free block yet, do nothing
            {
                alive_last = current;
                current = next;
                continue;
            }

            // traverse the free list to find big enough space
            // NOTE that in normal case (without pinned objects) the free space will be a single huge block
            // we will only move downwards

            free = heap->free;
            while (free != NULL && free < current)
            {
                if (free->block_length <= current->block_length || &free[free->block_length] == current)
                    break;

                free = pinta_free_get_next(free);
            }

            // we either found a block big enough to contain current
            // or we reached a free block that is just lower the current
            if (free == NULL || free > current)
            {
                alive_last = current;
                current = next;
                continue;
            }

            if (&free[free->block_length] == current) // (best case) in case we found an adjacent free block, we can ignore the blocks length, we are sure that length(free + current) > length(current)
            {
                PintaHeapObject *reloc_start = current;
                PintaHeapObject *reloc_end = &current[current->block_length];

                available = free->block_length;

                free_before = pinta_free_remove(heap, free);

                // relocate
                memmove(free, current, sizeof(PintaHeapObject) * current->block_length);
                if (alive_last < free)
                    alive_last = free;

                // at this point we must have at least one free entry in reloc
                // this is ensured (and actual relocation performed) at the beggining of the loop

                pinta_trace2("relocate: %p -> %p", current, free);

                reloc[count].start = reloc_start;
                reloc[count].end = reloc_end;
                reloc[count].offset = (u32)(current - free);
                count++;

                free = &free[free->block_length];
                pinta_free_insert_after(heap, free_before, free, available);
                if (free > free_last)
                    free_last = free;

                next = free;
            }
            else if (free->block_length >= current->block_length)
            {
                // so we found a block that fits perfectly
                // the current block will become free block, but to ensure the invariant, that free blocks are ordered by their addresses
                // the current free block must be inserted in appropriate place
                // unfortunately between free and current there might be more free blocks, thus we have free_last
                // if free and free_last are equal, then insertion is simple
                u32 current_block_length = current->block_length;
                PintaHeapObject *free_before_last = NULL;

                PintaHeapObject *reloc_start = current;
                PintaHeapObject *reloc_end = &current[current->block_length];

                if (free_last != NULL)
                    free_before_last = pinta_free_get_prev(free_last);

                available = free->block_length - current->block_length;

                // remove the current free block from free list
                free_before = pinta_free_remove(heap, free);

                // relocate
                memmove(free, current, sizeof(PintaHeapObject) * current->block_length);
                if (alive_last < free)
                    alive_last = free;

                // at this point we must have at least one free entry in reloc
                // this is ensured (and actual relocation performed) at the beginning of the loop

                pinta_trace2("relocate: %p -> %p", current, free);

                reloc[count].start = reloc_start;
                reloc[count].end = reloc_end;
                reloc[count].offset = (u32)(current - free);
                count++;

                if (available != 0)
                {
                    free = &free[current_block_length];
                    pinta_free_insert_after(heap, free_before, free, available);

                    if (free > free_last)
                        free_last = free;
                }
                else
                {
                    // we have consumed the free object
                    // it is possible that free == free_last in that case free_last must be free_last->prev
                    // but hold on, in case free == free_last, we have corrupted the entry, thus we have free_before_last

                    if (free_last == free)
                        free_last = free_before_last;
                }

                // there are TWO free inserts because: if available != we have partial free block, thus the first insert
                // after we have moved current, it is whole a free block, thus the second insert
                pinta_free_insert_after(heap, free_last, current, current_block_length);
                if (free_last != NULL && &free_last[free_last->block_length] == current)
                    next = current;
                else if (free_last != NULL)
                    free_last = current;
            }
        }
        else
        {
            // this is a pinned non-free object, make it last
            alive_last = current;
        }

        current = next;
    }

    if (alive_last != NULL)
        heap->top = &alive_last[alive_last->block_length];
    else
        heap->top = heap->start;

    if (heap->free != NULL)
    {
        // compaction screws the free list (the invariant that last free block is < heap->top might no longer be true), fix it (note that this normally should be very quick unless there are a lot of fragmentation due to pinned objects)
        if (heap->free >= heap->top)
            heap->free = NULL;

        free = heap->free;
        while (free != NULL && pinta_free_get_next(free) < heap->top)
            free = pinta_free_get_next(free);

        pinta_assert(free == NULL || pinta_free_get_next(free) == heap->top);

        if (free != NULL)
            pinta_free_set_next(free, NULL);
    }

    if (count > 0)
        pinta_gc_relocate(core, reloc, count);

    pinta_debug_validate_heap(heap);
}

void pinta_gc_compact(PintaCore *core)
{
    PintaThread *current;
    PintaHeapReloc single;
    PintaHeapReloc *reloc = &single;
    PintaHeap *heap;
    u32 reloc_count = 0, count = 0;

    pinta_assert(core != NULL);

#if PINTA_DEBUG
    //pinta_debug_print("compact\n");
#endif

    for (current = core->threads; current != NULL; current = current->next)
    {
        // look through threads for space for reloc info, use the one that has the most free space [all mutators must be stopped at this point]
        // in the worst case we will use single entry allocated in the native stack

        count = (u32)((PintaHeapReloc*)&current->frame->stack_end[0] - (PintaHeapReloc*)&current->frame->stack[1]);

        if (count > reloc_count)
        {
            reloc = (PintaHeapReloc*)&current->frame->stack[1];
            reloc_count = count;
        }
    }

    if (reloc_count == 0)
    {
        reloc = &single;
        reloc_count = 1;
    }

    for (heap = core->heap; heap != NULL; heap = heap->next)
        pinta_gc_compact_worker(core, heap, reloc, reloc_count);
}
