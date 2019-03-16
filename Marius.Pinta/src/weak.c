#include "pinta.h"

/* WEAK accessors */

PintaHeapObject *pinta_weak_get_target(PintaHeapObject *weak)
{
    pinta_assert(weak != NULL);
    pinta_assert(weak->block_kind == PINTA_KIND_WEAK);

    return weak->data.weak.target;
}

PintaHeapObject *pinta_weak_get_next(PintaHeapObject *weak)
{
    pinta_assert(weak != NULL);
    pinta_assert(weak->block_kind == PINTA_KIND_WEAK);

    return weak->data.weak.next;
}

void pinta_weak_set_target(PintaHeapObject *weak, PintaHeapObject *target)
{
    pinta_assert(weak != NULL);
    pinta_assert(weak->block_kind == PINTA_KIND_WEAK);

    weak->data.weak.target = target;
}

void pinta_weak_set_next(PintaHeapObject *weak, PintaHeapObject *next)
{
    pinta_assert(weak != NULL);
    pinta_assert(weak->block_kind == PINTA_KIND_WEAK);

    weak->data.weak.next = next;
}

PintaHeapObject *pinta_weak_ref_get_target(PintaReference *weak)
{
    pinta_assert(weak != NULL);

    return pinta_weak_get_target(weak->reference);
}

PintaHeapObject *pinta_weak_ref_get_next(PintaReference *weak)
{
    pinta_assert(weak != NULL);

    return pinta_weak_get_next(weak->reference);
}

void pinta_weak_ref_set_target(PintaReference *weak, PintaReference *target)
{
    pinta_assert(weak != NULL);
    pinta_assert(target != NULL);

    pinta_weak_set_target(weak->reference, target->reference);
}

void pinta_weak_ref_set_target_null(PintaReference *weak)
{
    pinta_assert(weak != NULL);

    pinta_weak_set_target(weak->reference, NULL);
}

void pinta_weak_ref_set_next(PintaReference *weak, PintaReference *next)
{
    pinta_assert(weak != NULL);
    pinta_assert(next != NULL);

    pinta_weak_set_next(weak->reference, next->reference);
}

void pinta_weak_ref_set_next_null(PintaReference *weak)
{
    pinta_assert(weak != NULL);

    pinta_weak_set_next(weak->reference, NULL);
}

/* WEAK */

void pinta_weak_init_type(PintaType *type)
{
    type->is_string = 0;

    type->gc_relocate = pinta_weak_relocate;

#if PINTA_DEBUG
    type->debug_write = pinta_lib_weak_debug_write;
#endif
}

void pinta_weak_relocate(PintaHeapObject *object, PintaHeapReloc *reloc, u32 count)
{
    PintaHeapObject *target;
    PintaHeapReloc *entry;

    pinta_gc_relocate_validate(PINTA_KIND_WEAK, object, reloc, count);

    target = pinta_weak_get_target(object);
    if (target != NULL)
    {
        entry = pinta_gc_relocate_find(target, reloc, count);
        if (entry != NULL)
            pinta_weak_set_target(object, target - entry->offset);
    }

    target = pinta_weak_get_next(object);
    if (target != NULL)
    {
        entry = pinta_gc_relocate_find(target, reloc, count);
        if (entry != NULL)
            pinta_weak_set_next(object, target - entry->offset);
    }
}

PintaHeapObject *pinta_weak_alloc_object_value(PintaCore *core, PintaReference *target)
{
    PintaHeapObject *result;

    pinta_assert(core != NULL);

    result = pinta_core_alloc(core, PINTA_KIND_WEAK, PINTA_FLAG_NONE, 1);

    if (target != NULL)
        pinta_weak_set_target(result, target->reference);
    else
        pinta_weak_set_target(result, NULL);

    pinta_weak_set_next(result, core->weak.reference);
    core->weak.reference = result;

    return result;
}

PintaException pinta_lib_weak_alloc(PintaCore *core, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    PintaHeapObject *value;

    pinta_assert(core != NULL);
    pinta_assert(result != NULL);

    value = pinta_weak_alloc_object_value(core, NULL);
    if (value == NULL)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_MEMORY);

    result->reference = value;

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_weak_alloc_value(PintaCore *core, PintaReference *target, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    PintaHeapObject *value;

    pinta_assert(core != NULL);
    pinta_assert(result != NULL);
    pinta_assert(target != NULL);

    value = pinta_weak_alloc_object_value(core, target);
    if (value == NULL)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_MEMORY);

    result->reference = value;

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_weak_get_target(PintaCore *core, PintaReference *weak, PintaReference *result)
{
    PintaException exception = PINTA_OK;

    PINTA_UNUSED(core);

    pinta_assert(core != NULL);
    pinta_assert(weak != NULL);
    pinta_assert(result != NULL);

    if (weak->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (weak->reference->block_kind != PINTA_KIND_WEAK)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    result->reference = pinta_weak_ref_get_target(weak);

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_weak_set_target(PintaCore *core, PintaReference *weak, PintaReference *target)
{
    PintaException exception = PINTA_OK;

    PINTA_UNUSED(core);

    pinta_assert(core != NULL);
    pinta_assert(weak != NULL);
    pinta_assert(target != NULL);

    if (weak->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (weak->reference->block_kind != PINTA_KIND_WEAK)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    pinta_weak_ref_set_target(weak, target);

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_weak_debug_write(PintaCore *core, PintaReference *value, u32 max_depth, PintaJsonWriter *writer)
{
    PintaException exception = PINTA_OK;

    PintaReference body;

    PINTA_GC_ENTER(core, body);

    PINTA_CHECK(pinta_json_write_object_start(core, writer));
    PINTA_CHECK(pinta_json_write_property_name_c(core, writer, "type"));
    PINTA_CHECK(pinta_json_write_string_c(core, writer, "weak", 4));

    body.reference = pinta_weak_ref_get_target(value);

    PINTA_CHECK(pinta_json_write_property_name_c(core, writer, "body"));
    if (max_depth > 0)
        PINTA_CHECK(pinta_lib_debug_write(core, &body, max_depth - 1, writer));
    else
        PINTA_CHECK(pinta_json_write_skipped(core, writer));

    PINTA_CHECK(pinta_json_write_end(core, writer));

PINTA_EXIT:
    PINTA_GC_EXIT(core);
    return exception;
}
