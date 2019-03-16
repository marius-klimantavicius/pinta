#include "pinta.h"

/* FUNCTION BODY accessors */

PintaHeapObject *pinta_function_body_get_binding(PintaFunctionBody *function_body)
{
    pinta_assert(function_body != NULL);

    return function_body->function_binding;
}

PintaHeapObject *pinta_function_body_get_closure(PintaFunctionBody *function_body)
{
    pinta_assert(function_body != NULL);

    return function_body->function_closure;
}

PintaHeapObject *pinta_function_body_get_object(PintaFunctionBody *function_body)
{
    pinta_assert(function_body != NULL);

    return function_body->function_object;
}

PintaHeapObject *pinta_function_body_get_prototype(PintaFunctionBody *function_body)
{
    pinta_assert(function_body != NULL);

    return function_body->function_prototype;
}

void pinta_function_body_set_binding(PintaFunctionBody *function_body, PintaHeapObject *function_binding)
{
    pinta_assert(function_body != NULL);

    function_body->function_binding = function_binding;
}

void pinta_function_body_set_closure(PintaFunctionBody *function_body, PintaHeapObject *function_closure)
{
    pinta_assert(function_body != NULL);

    function_body->function_closure = function_closure;
}

void pinta_function_body_set_object(PintaFunctionBody *function_body, PintaHeapObject *function_object)
{
    pinta_assert(function_body != NULL);

    function_body->function_object = function_object;
}

void pinta_function_body_set_prototype(PintaFunctionBody *function_body, PintaHeapObject *function_prototype)
{
    pinta_assert(function_body != NULL);

    function_body->function_prototype = function_prototype;
}

u32 pinta_function_body_walk(PintaFunctionBody *function_body, PintaHeapObjectWalkerState *state, PintaHeapObject **result)
{
    PintaHeapObject *item;

    pinta_assert(function_body != NULL);
    pinta_assert(state != NULL);
    pinta_assert(result != NULL);

    switch (state->field)
    {
    case 0:
        item = pinta_function_body_get_binding(function_body);
        if (item != NULL)
        {
            *result = item;
            state->field = 1;
            return 1;
        }
        // fall-through
    case 1:
        item = pinta_function_body_get_closure(function_body);
        if (item != NULL)
        {
            *result = item;
            state->field = 2;
            return 1;
        }
        // fall-through
    case 2:
        item = pinta_function_body_get_object(function_body);
        if (item != NULL)
        {
            *result = item;
            state->field = 3;
            return 1;
        }
        // fall-through
    case 3:
        item = pinta_function_body_get_prototype(function_body);
        if (item != NULL)
        {
            *result = item;
            state->field = 4;
            return 1;
        }
        // fall-through
    }

    return 0;
}

void pinta_function_body_relocate(PintaFunctionBody *function_body, PintaHeapReloc *reloc, u32 count)
{
    PintaHeapObject *item;
    PintaHeapReloc *entry;

    pinta_assert(function_body != NULL);

    item = pinta_function_body_get_binding(function_body);
    if (item != NULL)
    {
        entry = pinta_gc_relocate_find(item, reloc, count);
        if (entry != NULL)
            pinta_function_body_set_binding(function_body, item - entry->offset);
    }

    item = pinta_function_body_get_closure(function_body);
    if (item != NULL)
    {
        entry = pinta_gc_relocate_find(item, reloc, count);
        if (entry != NULL)
            pinta_function_body_set_binding(function_body, item - entry->offset);
    }

    item = pinta_function_body_get_object(function_body);
    if (item != NULL)
    {
        entry = pinta_gc_relocate_find(item, reloc, count);
        if (entry != NULL)
            pinta_function_body_set_object(function_body, item - entry->offset);
    }

    item = pinta_function_body_get_prototype(function_body);
    if (item != NULL)
    {
        entry = pinta_gc_relocate_find(item, reloc, count);
        if (entry != NULL)
            pinta_function_body_set_prototype(function_body, item - entry->offset);
    }
}
