#include "pinta.h"

/* ARRAY accessors */

PintaHeapObject **pinta_array_get_data(PintaHeapObject *array)
{
    pinta_assert(array != NULL);
    pinta_assert(array->block_kind == PINTA_KIND_ARRAY);

    return (PintaHeapObject**)(array + 1);
}

u32 pinta_array_get_length(PintaHeapObject *array)
{
    pinta_assert(array != NULL);
    pinta_assert(array->block_kind == PINTA_KIND_ARRAY);

    return array->data.array.array_length;
}

PintaHeapObject *pinta_array_get_item(PintaHeapObject *array, u32 index)
{
    PintaHeapObject **array_data;
    pinta_assert(array != NULL);
    pinta_assert(array->block_kind == PINTA_KIND_ARRAY);
    pinta_assert(index < array->data.array.array_length);

    array_data = pinta_array_get_data(array);
    return array_data[index];
}

void pinta_array_set_length(PintaHeapObject *array, u32 length)
{
    pinta_assert(array != NULL);
    pinta_assert(array->block_kind == PINTA_KIND_ARRAY);

    array->data.array.array_length = length;
}

void pinta_array_set_item(PintaHeapObject *array, u32 index, PintaHeapObject *value)
{
    PintaHeapObject **array_data;
    pinta_assert(array != NULL);
    pinta_assert(array->block_kind == PINTA_KIND_ARRAY);
    pinta_assert(index < array->data.array.array_length);

    array_data = pinta_array_get_data(array);
    array_data[index] = value;
}

PintaHeapObject **pinta_array_ref_get_data(PintaReference *array)
{
    pinta_assert(array != NULL);

    return pinta_array_get_data(array->reference);
}

u32 pinta_array_ref_get_length(PintaReference *array)
{
    pinta_assert(array != NULL);

    return pinta_array_get_length(array->reference);
}

PintaHeapObject *pinta_array_ref_get_item(PintaReference *array, u32 index)
{
    pinta_assert(array != NULL);

    return pinta_array_get_item(array->reference, index);
}

void pinta_array_ref_set_length(PintaReference *array, u32 length)
{
    pinta_assert(array != NULL);

    pinta_array_set_length(array->reference, length);
}

void pinta_array_ref_set_item(PintaReference *array, u32 index, PintaReference *value)
{
    pinta_assert(array != NULL);
    pinta_assert(value != NULL);

    pinta_array_set_item(array->reference, index, value->reference);
}

void pinta_array_ref_set_item_null(PintaReference *array, u32 index)
{
    pinta_assert(array != NULL);

    pinta_array_set_item(array->reference, index, NULL);
}

/* ARRAY */

void pinta_array_init_type(PintaType *type)
{
    type->is_string = 0;

    type->gc_walk = pinta_array_walk;
    type->gc_relocate = pinta_array_relocate;

    type->get_member = pinta_lib_array_get_member;
    type->set_member = pinta_lib_array_set_member;

    type->get_item = pinta_lib_array_get_item;
    type->set_item = pinta_lib_array_set_item;
    type->get_length = pinta_lib_array_get_length;

#if PINTA_DEBUG
    type->debug_write = pinta_lib_array_debug_write;
#endif
}

void pinta_array_relocate(PintaHeapObject *object, PintaHeapReloc *reloc, u32 count)
{
    PintaHeapObject **data;
    PintaHeapObject *item;
    u32 array_length;
    PintaHeapReloc *entry;
    u32 index;

    pinta_gc_relocate_validate(PINTA_KIND_ARRAY, object, reloc, count);

    data = pinta_array_get_data(object);
    if (data == NULL)
        return;

    array_length = pinta_array_get_length(object);
    for (index = 0; index < array_length; index++)
    {
        item = pinta_array_get_item(object, index);
        if (item == NULL)
            continue;

        entry = pinta_gc_relocate_find(item, reloc, count);
        if (entry != NULL)
            pinta_array_set_item(object, index, item - entry->offset);
    }
}

u32 pinta_array_walk(PintaHeapObject *object, PintaHeapObjectWalkerState *state, PintaHeapObject **result)
{
    PintaHeapObject *item;
    u32 index;
    u32 array_length;

    pinta_assert(object != NULL);
    pinta_assert(state != NULL);
    pinta_assert(result != NULL);
    pinta_assert(object->block_kind == PINTA_KIND_ARRAY);

    array_length = pinta_array_get_length(object);
    for (index = state->index; index < array_length; index++)
    {
        item = pinta_array_get_item(object, index);
        if (item != NULL)
        {
            *result = item;
            state->index = index + 1;
            return 1;
        }
    }

    return 0;
}

PintaHeapObject *pinta_array_alloc_object(PintaCore *core, u32 array_length)
{
    u32 block_length;
    u32 index = 0;
    PintaHeapObject *result;

    pinta_assert(core != NULL);

    if (array_length == 0 && core->cache != NULL)
        return &core->cache->array_empty[0];

    block_length = (sizeof(PintaHeapObject) + array_length * sizeof(PintaHeapObject*) + sizeof(PintaHeapObject) - 1) / sizeof(PintaHeapObject);
    result = pinta_core_alloc(core, PINTA_KIND_ARRAY, PINTA_FLAG_NONE, block_length);
    if (result == NULL)
        return NULL;

    pinta_array_set_length(result, array_length);
    for (index = 0; index < array_length; index++)
        pinta_array_set_item(result, index, NULL);

    return result;
}

PintaHeapObject *pinta_array_alloc_object_value(PintaCore *core, u32 array_length, PintaReference *source)
{
    u32 block_length;
    u32 index = 0;
    PintaHeapObject *result;
    u32 source_length;

    pinta_assert(core != NULL);
    pinta_assert(source != NULL);

    if (array_length == 0 && core->cache != NULL)
        return &core->cache->array_empty[0];

    block_length = (sizeof(PintaHeapObject) + array_length * sizeof(PintaHeapObject*) + sizeof(PintaHeapObject) - 1) / sizeof(PintaHeapObject);
    result = pinta_core_alloc(core, PINTA_KIND_ARRAY, PINTA_FLAG_NONE, block_length);
    if (result == NULL)
        return NULL;

    pinta_array_set_length(result, array_length);

    index = 0;
    if (source != NULL && source->reference != NULL)
    {
        if (source->reference->block_kind == PINTA_KIND_ARRAY)
        {
            source_length = pinta_array_ref_get_length(source);
            if (source_length > array_length)
                source_length = array_length;

            index = index + source_length;
            pinta_array_copy_array(result, 0, source->reference, 0, source_length);
        }
        else
        {
            index++;
            pinta_array_set_item(result, index, source->reference);
        }
    }

    for (; index < array_length; index++)
        pinta_array_set_item(result, index, NULL);

    return result;
}

void pinta_array_copy(PintaHeapObject *destination_array, u32 destination_offset, PintaReference *source_data, u32 length)
{
    PintaHeapObject **destination_data;
    u32 index;

    pinta_assert(destination_array != NULL);
    pinta_assert(destination_array->block_kind == PINTA_KIND_ARRAY);
    pinta_assert(source_data != NULL);
    pinta_assert(destination_offset <= pinta_array_get_length(destination_array));
    pinta_assert(length <= pinta_array_get_length(destination_array) - destination_offset);

    destination_data = pinta_array_get_data(destination_array);
    destination_data = &destination_data[destination_offset];

    for (index = 0; index < length; index++)
        destination_data[index] = source_data[index].reference;
}

void pinta_array_copy_array(PintaHeapObject *destination_array, u32 destination_offset, PintaHeapObject *source_array, u32 source_offset, u32 length)
{
    PintaHeapObject **destination_data;
    PintaHeapObject **source_data;

    pinta_assert(destination_array != NULL);
    pinta_assert(destination_array->block_kind == PINTA_KIND_ARRAY);
    pinta_assert(source_array != NULL);
    pinta_assert(source_array->block_kind == PINTA_KIND_ARRAY);
    pinta_assert(destination_offset <= pinta_array_get_length(destination_array));
    pinta_assert(length <= pinta_array_get_length(destination_array) - destination_offset);
    pinta_assert(source_offset <= pinta_array_get_length(source_array));
    pinta_assert(length <= pinta_array_get_length(source_array) - source_offset);

    destination_data = pinta_array_get_data(destination_array);
    destination_data = &destination_data[destination_offset];

    source_data = pinta_array_get_data(source_array);
    source_data = &source_data[source_offset];

    memmove(destination_data, source_data, length * sizeof(PintaHeapObject*));
}

void pinta_array_ref_copy(PintaReference *destination_array, u32 destination_offset, PintaReference *source_data, u32 length)
{
    pinta_assert(destination_array != NULL);

    pinta_array_copy(destination_array->reference, destination_offset, source_data, length);
}

void pinta_array_ref_copy_array(PintaReference *destination_array, u32 destination_offset, PintaReference *source_array, u32 source_offset, u32 length)
{
    pinta_assert(destination_array != NULL);
    pinta_assert(source_array != NULL);

    pinta_array_copy_array(destination_array->reference, destination_offset, source_array->reference, source_offset, length);
}

PintaException pinta_lib_array_alloc(PintaCore *core, u32 array_length, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    PintaHeapObject *value;

    pinta_assert(core != NULL);
    pinta_assert(result != NULL);

    value = pinta_array_alloc_object(core, array_length);
    if (value == NULL)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_MEMORY);

    result->reference = value;

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_array_alloc_value(PintaCore *core, u32 array_length, PintaReference *source, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    PintaHeapObject *value;

    pinta_assert(core != NULL);
    pinta_assert(result != NULL);
    pinta_assert(source != NULL);

    value = pinta_array_alloc_object_value(core, array_length, source);
    if (value == NULL)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_MEMORY);

    result->reference = value;

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_array_get_index(PintaHeapObject *array, u32 index, PintaReference *result)
{
    PintaException exception = PINTA_OK;

    pinta_assert(result != NULL);

    if (array == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (array->block_kind != PINTA_KIND_ARRAY)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    if (pinta_array_get_length(array) <= index)
        PINTA_THROW(PINTA_EXCEPTION_INDEX_OUT_OF_BOUNDS);

    result->reference = pinta_array_get_item(array, index);

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_array_set_index(PintaHeapObject *array, u32 index, PintaHeapObject *value)
{
    PintaException exception = PINTA_OK;

    if (array == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (array->block_kind != PINTA_KIND_ARRAY)
        PINTA_THROW(PINTA_EXCEPTION_INVALID_OPERATION);

    if (pinta_array_get_length(array) <= index)
        PINTA_THROW(PINTA_EXCEPTION_INDEX_OUT_OF_BOUNDS);

    pinta_array_set_item(array, index, value);

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_array_get_member(PintaCore *core, PintaReference *object, PintaReference *name, u8 *is_accessor, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    i32 value;

    if (name->reference != NULL && name->reference->block_kind == PINTA_KIND_INTEGER)
    {
        value = pinta_integer_ref_get_value(name);
        if (value >= 0)
        {
            if (is_accessor != NULL)
                *is_accessor = 0;
            PINTA_CHECK(pinta_lib_array_get_item(core, object, (u32)value, result));
            PINTA_RETURN();
        }
    }

    PINTA_THROW(PINTA_EXCEPTION_NOT_IMPLEMENTED);

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_array_set_member(PintaCore *core, PintaReference *object, PintaReference *name, PintaReference *value, u8 *is_accessor, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    i32 index;

    PINTA_UNUSED(result);

    if (name->reference->block_kind == PINTA_KIND_INTEGER)
    {
        index = pinta_integer_ref_get_value(name);
        if (index >= 0)
        {
            if (is_accessor != NULL)
                *is_accessor = 0;

            PINTA_CHECK(pinta_lib_array_set_item(core, object, (u32)index, value));
            PINTA_RETURN();
        }
    }

    PINTA_THROW(PINTA_EXCEPTION_NOT_IMPLEMENTED);

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_array_get_length(PintaCore *core, PintaReference *array, u32 *result)
{
    PintaException exception = PINTA_OK;

    PINTA_UNUSED(core);

    pinta_assert(core != NULL);
    pinta_assert(array != NULL);
    pinta_assert(result != NULL);

    if (array->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (array->reference->block_kind != PINTA_KIND_ARRAY)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    *result = pinta_array_ref_get_length(array);

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_array_get_item(PintaCore *core, PintaReference *array, u32 index, PintaReference *result)
{
    PintaException exception = PINTA_OK;

    PINTA_UNUSED(core);

    pinta_assert(core != NULL);
    pinta_assert(array != NULL);
    pinta_assert(result != NULL);

    if (array->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (array->reference->block_kind != PINTA_KIND_ARRAY)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    if (pinta_array_ref_get_length(array) <= index)
        PINTA_THROW(PINTA_EXCEPTION_INDEX_OUT_OF_BOUNDS);

    result->reference = pinta_array_ref_get_item(array, index);

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_array_set_item(PintaCore *core, PintaReference *array, u32 index, PintaReference *value)
{
    PintaException exception = PINTA_OK;

    PINTA_UNUSED(core);

    pinta_assert(core != NULL);
    pinta_assert(array != NULL);
    pinta_assert(value != NULL);

    if (array->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (array->reference->block_kind != PINTA_KIND_ARRAY)
        PINTA_THROW(PINTA_EXCEPTION_INVALID_OPERATION);

    if (pinta_array_ref_get_length(array) <= index)
        PINTA_THROW(PINTA_EXCEPTION_INDEX_OUT_OF_BOUNDS);

    pinta_array_ref_set_item(array, index, value);

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_array_copy_array(PintaCore *core, PintaReference *destination, u32 destination_offset, PintaReference *source, u32 source_offset, u32 length)
{
    PintaException exception = PINTA_OK;

    u32 destination_length;
    u32 source_length;

    PINTA_UNUSED(core);

    pinta_assert(core != NULL);
    pinta_assert(destination != NULL);
    pinta_assert(source != NULL);

    if (destination->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (destination->reference->block_kind != PINTA_KIND_ARRAY)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    if (source->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (source->reference->block_kind != PINTA_KIND_ARRAY)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    destination_length = pinta_array_ref_get_length(destination);
    if (destination_offset >= destination_length)
        PINTA_THROW(PINTA_EXCEPTION_INDEX_OUT_OF_BOUNDS);

    source_length = pinta_array_ref_get_length(source);
    if (source_length >= source_offset)
        PINTA_THROW(PINTA_EXCEPTION_INDEX_OUT_OF_BOUNDS);

    if (length > destination_length - destination_offset)
        PINTA_THROW(PINTA_EXCEPTION_INDEX_OUT_OF_BOUNDS);

    if (length > source_length - source_offset)
        PINTA_THROW(PINTA_EXCEPTION_INDEX_OUT_OF_BOUNDS);

    pinta_array_copy_array(destination->reference, destination_offset, source->reference, source_offset, length);

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_array_debug_write(PintaCore *core, PintaReference *value, u32 max_depth, PintaJsonWriter *writer)
{
    PintaException exception = PINTA_OK;
    u32 length;
    u32 index;
    PintaReference item;

    PINTA_UNUSED(max_depth);

    pinta_assert(core != NULL);
    pinta_assert(value != NULL);

    if (value->reference == NULL)
        return pinta_json_write_null(core, writer);

    PINTA_GC_ENTER(core, item);

    length = pinta_array_ref_get_length(value);

    PINTA_CHECK(pinta_json_write_array_start(core, writer));

    if (length > 0)
    {
        if (max_depth == 0)
        {
            PINTA_CHECK(pinta_json_write_skipped(core, writer));
        }
        else
        {
            for (index = 0; index < length; index++)
            {
                PINTA_CHECK(pinta_lib_array_get_item(core, value, index, &item));
                PINTA_CHECK(pinta_lib_debug_write(core, &item, max_depth - 1, writer));
            }
        }
    }

    PINTA_CHECK(pinta_json_write_end(core, writer));

PINTA_EXIT:
    PINTA_GC_EXIT(core);
    return exception;
}
