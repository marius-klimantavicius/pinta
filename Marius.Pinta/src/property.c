#include "pinta.h"

static wchar type_literal[] = { 116, 121, 112, 101, 0 };
static wchar property_table_literal[] = { 112, 114, 111, 112, 101, 114, 116, 121, 32, 116, 97, 98, 108, 101, 0 };
static wchar capacity_literal[] = { 99, 97, 112, 97, 99, 105, 116, 121, 0 };
static wchar length_literal[] = { 108, 101, 110, 103, 116, 104, 0 };
static wchar properties_literal[] = { 112, 114, 111, 112, 101, 114, 116, 105, 101, 115, 0 };
static wchar hashcode_literal[] = { 104, 97, 115, 104, 99, 111, 100, 101, 0 };
static wchar is_configurable_literal[] = { 105, 115, 95, 99, 111, 110, 102, 105, 103, 117, 114, 97, 98, 108, 101, 0 };
static wchar is_writeable_literal[] = { 105, 115, 95, 119, 114, 105, 116, 101, 97, 98, 108, 101, 0 };
static wchar is_accessor_literal[] = { 105, 115, 95, 97, 99, 99, 101, 115, 115, 111, 114, 0 };
static wchar is_native_literal[] = { 105, 115, 95, 110, 97, 116, 105, 118, 101, 0 };
static wchar token_literal[] = { 116, 111, 107, 101, 110, 0 };
static wchar get_literal[] = { 103, 101, 116, 0 };
static wchar set_literal[] = { 115, 101, 116, 0 };
static wchar value_literal[] = { 118, 97, 108, 117, 101, 0 };
static wchar key_literal[] = { 107, 101, 121, 0 };

#if PINTA_DEBUG

void pinta_debug_validate_slot(PintaReference *table, PintaPropertySlot *slot, PintaReference *key)
{
    PINTA_UNUSED(table);
    PINTA_UNUSED(slot);
    PINTA_UNUSED(key);
}

#else

#define pinta_debug_validate_slot(...)  ((void)0)

#endif /* PINTA_DEBUG */

void pinta_property_fill_entry(PintaProperty *property, u32 index, u8 is_found, PintaPropertySlot *result)
{
    if (result == NULL)
        return;

    result->is_valid = 1;
    result->is_found = is_found;
    result->property_id = index;

    if (is_found)
    {
        result->is_enumerable = pinta_property_is_enumerable(property);
        result->is_configurable = pinta_property_is_configurable(property);
        result->is_accessor = pinta_property_is_accessor(property);
        if (result->is_accessor)
            result->is_native = pinta_property_is_writeable_or_native(property);
        else
            result->is_writeable = pinta_property_is_writeable_or_native(property);
    }
}

PintaProperty *pinta_property_table_find(PintaProperty *table, u32 table_capacity, PintaReference *key, u32 key_hash_code, PintaPropertySlot *result)
{
    u32 index;
    u32 hash_index;
    u8 found;
    PintaProperty *current;
    u32 current_hash_code;

    pinta_assert(table != NULL);
    pinta_assert(key != NULL);

    if (table_capacity == 0) 
    {
        if (result != NULL) 
        {
            result->is_valid = 0;
            result->is_found = 0;
        }

        return NULL;
    }

    hash_index = key_hash_code % table_capacity;

    for (index = hash_index; index < table_capacity; index++)
    {
        current = &table[index];

        if (current->key == NULL)
        {
            pinta_property_fill_entry(current, index, 0, result);
            return current;
        }

        current_hash_code = pinta_property_get_hash_code(current);
        if (current_hash_code != key_hash_code)
            continue;

        found = pinta_string_equals(key->reference, current->key);
        if (found)
        {
            pinta_property_fill_entry(current, index, 1, result);
            return current;
        }
    }

    for (index = 0; index < hash_index; index++)
    {
        current = &table[index];

        if (current->key == NULL)
        {
            pinta_property_fill_entry(current, index, 0, result);
            return current;
        }

        current_hash_code = pinta_property_get_hash_code(current);
        if (current_hash_code != key_hash_code)
            continue;

        found = pinta_string_equals(key->reference, current->key);
        if (found)
        {
            pinta_property_fill_entry(current, index, 1, result);
            return current;
        }
    }

    if (result != NULL) 
    {
        result->is_valid = 0;
        result->is_found = 0;
    }

    return NULL;
}

void pinta_property_table_copy_property(PintaProperty *destination_table, u32 destination_capacity, PintaProperty *value)
{
    u32 hash_code;
    u32 hash_index;
    u32 index;
    PintaProperty *current;

    if (value->key == NULL)
        return;

    hash_code = pinta_property_get_hash_code(value);
    hash_index = hash_code % destination_capacity;

    for (index = hash_index; index < destination_capacity; index++)
    {
        current = &destination_table[index];
        if (current->key == NULL)
        {
            *current = *value;
            return;
        }
    }

    for (index = 0; index < hash_index; index++)
    {
        current = &destination_table[index];
        if (current->key == NULL)
        {
            *current = *value;
            return;
        }
    }
}

u32 pinta_property_table_get_next_capacity(u32 capacity)
{
    if (capacity < 4)
        return 4;
    if (capacity < 8)
        return 8;
    if (capacity < 17)
        return 17;
    if (capacity < 31)
        return 31;

    capacity = (capacity << 1) - 1;
    capacity |= capacity >> 1;
    capacity |= capacity >> 2;
    capacity |= capacity >> 4;
    capacity |= capacity >> 8;
    capacity |= capacity >> 16;
    return capacity;
}

u8 pinta_property_table_should_extend(u32 capacity, u32 length)
{
    u32 free;

    pinta_assert(length <= capacity);

    free = capacity - (capacity >> 2);
    if (capacity > 16 && length >= free)
        return 1;

    return 0;
}

/* PROPERTY TABLE accessors */

u32 pinta_property_table_get_capacity(PintaHeapObject *object)
{
    pinta_assert(object != NULL);
    pinta_assert(object->block_kind == PINTA_KIND_PROPERTY_TABLE);

    return object->data.property_table.table_capacity;
}

u32 pinta_property_table_get_length(PintaHeapObject *object)
{
    pinta_assert(object != NULL);
    pinta_assert(object->block_kind == PINTA_KIND_PROPERTY_TABLE);

    return object->data.property_table.table_length;
}

PintaProperty *pinta_property_table_get_table(PintaHeapObject *object)
{
    pinta_assert(object != NULL);
    pinta_assert(object->block_kind == PINTA_KIND_PROPERTY_TABLE);

    return (PintaProperty*)&object[1];
}

PintaProperty *pinta_property_table_get(PintaHeapObject *object, u32 property_id)
{
    PintaProperty *table;

    pinta_assert(object != NULL);
    pinta_assert(object->block_kind == PINTA_KIND_PROPERTY_TABLE);
    pinta_assert(property_id < object->data.property_table.table_capacity);

    table = pinta_property_table_get_table(object);
    return &table[property_id];
}

void pinta_property_table_set_capacity(PintaHeapObject *object, u32 value)
{
    pinta_assert(object != NULL);
    pinta_assert(object->block_kind == PINTA_KIND_PROPERTY_TABLE);

    object->data.property_table.table_capacity = value;
}

void pinta_property_table_set_length(PintaHeapObject *object, u32 value)
{
    pinta_assert(object != NULL);
    pinta_assert(object->block_kind == PINTA_KIND_PROPERTY_TABLE);

    object->data.property_table.table_length = value;
}

u32 pinta_property_table_ref_get_capacity(PintaReference *reference)
{
    pinta_assert(reference != NULL);

    return pinta_property_table_get_capacity(reference->reference);
}

u32 pinta_property_table_ref_get_length(PintaReference *reference)
{
    pinta_assert(reference != NULL);

    return pinta_property_table_get_length(reference->reference);
}

PintaProperty *pinta_property_table_ref_get_table(PintaReference *reference)
{
    pinta_assert(reference != NULL);

    return pinta_property_table_get_table(reference->reference);
}

PintaProperty *pinta_property_table_ref_get(PintaReference *reference, u32 property_id)
{
    pinta_assert(reference != NULL);

    return pinta_property_table_get(reference->reference, property_id);
}

void pinta_property_table_ref_set_capacity(PintaReference *reference, u32 value)
{
    pinta_assert(reference != NULL);

    pinta_property_table_set_capacity(reference->reference, value);
}

void pinta_property_table_ref_set_length(PintaReference *reference, u32 value)
{
    pinta_assert(reference != NULL);

    pinta_property_table_set_length(reference->reference, value);
}

/* PROPERTY accessors */

u32 pinta_property_is_free(PintaProperty *property)
{
    return property->key == NULL;
}

u32 pinta_property_is_enumerable(PintaProperty *property)
{
    return (property->key_hash_code & PINTA_PROPERTY_ENUMERABLE) != 0;
}

u32 pinta_property_is_configurable(PintaProperty *property)
{
    return (property->key_hash_code & PINTA_PROPERTY_CONFIGURABLE) != 0;
}

u32 pinta_property_is_writeable_or_native(PintaProperty *property)
{
    return (property->key_hash_code & PINTA_PROPERTY_WRITEABLE_OR_NATIVE) != 0;
}

u32 pinta_property_is_accessor(PintaProperty *property)
{
    return (property->key_hash_code & PINTA_PROPERTY_ACCESSOR) != 0;
}

u32 pinta_property_get_hash_code(PintaProperty *property)
{
    return property->key_hash_code & ~PINTA_PROPERTY_MASK;
}

PintaHeapObject *pinta_property_get_key(PintaProperty *property)
{
    pinta_assert(property != NULL);

    return property->key;
}

PintaHeapObject *pinta_property_get_value(PintaProperty *property)
{
    pinta_assert(property != NULL);

    return property->value.data.value;
}

PintaHeapObject *pinta_property_get_accessor_get(PintaProperty *property)
{
    pinta_assert(property != NULL);

    return property->value.accessor.get;
}

PintaHeapObject *pinta_property_get_accessor_set(PintaProperty *property)
{
    pinta_assert(property != NULL);

    return property->value.accessor.set;
}

PintaPropertyNativeDelegate pinta_property_get_native_delegate(PintaProperty *property)
{
    pinta_assert(property != NULL);

    return property->value.native.native_delegate;
}

u32 pinta_property_get_native_token(PintaProperty *property)
{
    pinta_assert(property != NULL);

    return property->value.native.native_token;
}

void pinta_property_set_hash_code(PintaProperty *property, u8 is_enumerable, u8 is_configurable, u8 is_writeable_or_native, u8 is_accessor, u32 hash_code)
{
    hash_code = hash_code & ~PINTA_PROPERTY_MASK;

    if (is_enumerable)
        hash_code = hash_code | PINTA_PROPERTY_ENUMERABLE;

    if (is_configurable)
        hash_code = hash_code | PINTA_PROPERTY_CONFIGURABLE;

    if (is_writeable_or_native)
        hash_code = hash_code | PINTA_PROPERTY_WRITEABLE_OR_NATIVE;

    if (is_accessor)
        hash_code = hash_code | PINTA_PROPERTY_ACCESSOR;

    property->key_hash_code = hash_code;
}

void pinta_property_set_key(PintaProperty *property, PintaHeapObject *key)
{
    pinta_assert(property != NULL);

    property->key = key;
}

void pinta_property_set_value(PintaProperty *property, PintaHeapObject *value)
{
    pinta_assert(property != NULL);

    property->value.data.value = value;
}

void pinta_property_set_accessor_get(PintaProperty *property, PintaHeapObject *get)
{
    pinta_assert(property != NULL);

    property->value.accessor.get = get;
}

void pinta_property_set_accessor_set(PintaProperty *property, PintaHeapObject *set)
{
    pinta_assert(property != NULL);

    property->value.accessor.set = set;
}

void pinta_property_set_native_delegate(PintaProperty *property, PintaPropertyNativeDelegate delegate)
{
    pinta_assert(property != NULL);

    property->value.native.native_delegate = delegate;
}

void pinta_property_set_native_token(PintaProperty *property, u32 token)
{
    pinta_assert(property != NULL);

    property->value.native.native_token = token;
}

/* PROPERTY TABLE */

void pinta_property_table_init_type(PintaType *type)
{
    type->is_string = 0;

    type->gc_walk = pinta_property_table_walk;
    type->gc_relocate = pinta_property_table_relocate;

#if PINTA_DEBUG
    type->debug_write = pinta_lib_property_table_debug_write;
#endif
}

u32 pinta_property_table_walk(PintaHeapObject *object, PintaHeapObjectWalkerState *state, PintaHeapObject **result)
{
    PintaProperty *table;
    PintaProperty *current;
    u32 capacity;
    u32 index;
    u32 field;
    u32 is_accessor;
    u32 is_native;
    PintaHeapObject *item;

    table = pinta_property_table_get_table(object);
    capacity = pinta_property_table_get_capacity(object);

    index = state->index;
    field = state->field;

    for (; index < capacity; index++)
    {
        current = &table[index];

        if (!pinta_property_is_free(current))
        {
            if (field == 0)
            {
                *result = pinta_property_get_key(current);

                state->index = index;
                state->field = 1;
                return 1;
            }

            is_accessor = pinta_property_is_accessor(current);
            if (field == 1)
            {
                if (is_accessor)
                {
                    is_native = pinta_property_is_writeable_or_native(current);
                    if (!is_native)
                    {
                        item = pinta_property_get_accessor_get(current);
                        if (item != NULL)
                        {
                            *result = item;

                            state->index = index;
                            state->field = 2;
                            return 1;
                        }
                    }
                }
                else
                {
                    item = pinta_property_get_value(current);
                    if (item != NULL)
                    {
                        *result = item;

                        state->index = (index + 1);
                        state->field = 0;
                        return 1;
                    }
                }

                field = 2;
            }

            if (field == 2)
            {
                if (is_accessor)
                {
                    is_native = pinta_property_is_writeable_or_native(current);
                    if (!is_native)
                    {
                        item = pinta_property_get_accessor_set(current);
                        if (item != NULL)
                        {
                            *result = item;

                            state->index = (index + 1);
                            state->field = 0;
                            return 1;
                        }
                    }
                }
            }
        }

        field = 0;
    }

    return 0;
}

void pinta_property_table_relocate(PintaHeapObject *object, PintaHeapReloc *reloc, u32 count)
{
    PintaProperty *table;
    PintaProperty *current;
    PintaHeapReloc *entry;
    u32 capacity;
    u32 index;
    u32 is_accessor;
    PintaHeapObject *item;

    table = pinta_property_table_get_table(object);
    capacity = pinta_property_table_get_capacity(object);

    if (capacity == 0)
        return;

    for (index = 0; index < capacity; index++)
    {
        current = &table[index];

        if (pinta_property_is_free(current))
            continue;

        item = pinta_property_get_key(current);

        entry = pinta_gc_relocate_find(item, reloc, count);
        if (entry != NULL)
            pinta_property_set_key(current, item - entry->offset);

        is_accessor = pinta_property_is_accessor(current);
        if (is_accessor)
        {
            item = pinta_property_get_accessor_get(current);
            if (item != NULL)
            {
                entry = pinta_gc_relocate_find(item, reloc, count);
                if (entry != NULL)
                    pinta_property_set_accessor_get(current, item - entry->offset);
            }

            item = pinta_property_get_accessor_set(current);
            if (item != NULL)
            {
                entry = pinta_gc_relocate_find(item, reloc, count);
                if (entry != NULL)
                    pinta_property_set_accessor_set(current, item - entry->offset);
            }
        }
        else
        {
            item = pinta_property_get_value(current);
            if (item != NULL)
            {
                entry = pinta_gc_relocate_find(item, reloc, count);
                if (entry != NULL)
                    pinta_property_set_value(current, item - entry->offset);
            }
        }
    }
}

PintaHeapObject *pinta_property_table_alloc_object(PintaCore *core, u32 table_capacity)
{
    u32 block_length;
    PintaHeapObject *result;
    PintaProperty *table;

    pinta_assert(core != NULL);

    if (table_capacity == 0 && core->cache != NULL)
        return &core->cache->property_table_empty[0];

    block_length = (sizeof(PintaHeapObject) + table_capacity * sizeof(PintaProperty) + sizeof(PintaHeapObject) - 1) / sizeof(PintaHeapObject);
    result = pinta_core_alloc(core, PINTA_KIND_PROPERTY_TABLE, PINTA_FLAG_NONE, block_length);
    if (result == NULL)
        return NULL;

    pinta_property_table_set_capacity(result, table_capacity);
    pinta_property_table_set_length(result, 0);

    table = pinta_property_table_get_table(result);
    memset(table, 0, table_capacity * sizeof(PintaProperty));

    return result;
}

PintaHeapObject *pinta_property_table_alloc_object_value(PintaCore *core, PintaReference *value, u32 table_capacity)
{
    u32 index;

    PintaProperty *value_table;
    PintaProperty *value_current;
    u32 value_capacity;
    u32 value_length;

    PintaHeapObject *result;
    PintaProperty *result_table;

    pinta_assert(core != NULL);

    pinta_assert(value != NULL);
    pinta_assert(value->reference != NULL);
    pinta_assert(value->reference->block_kind == PINTA_KIND_PROPERTY_TABLE);
    pinta_assert(table_capacity > pinta_property_table_ref_get_capacity(value));

    result = pinta_property_table_alloc_object(core, table_capacity);
    if (result == NULL)
        return NULL;

    result_table = pinta_property_table_get_table(result);

    value_table = pinta_property_table_ref_get_table(value);
    value_capacity = pinta_property_table_ref_get_capacity(value);
    value_length = pinta_property_table_ref_get_length(value);

    for (index = 0; index < value_capacity; index++)
    {
        value_current = &value_table[index];
        if (value_current->key != NULL)
            pinta_property_table_copy_property(result_table, table_capacity, value_current);
    }

    pinta_property_table_set_length(result, value_length);

    return result;
}

PintaException pinta_lib_property_table_alloc(PintaCore *core, u32 capacity, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    PintaHeapObject *value;

    pinta_assert(core != NULL);
    pinta_assert(result != NULL);

    value = pinta_property_table_alloc_object(core, capacity);
    if (value == NULL)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_MEMORY);

    result->reference = value;

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_property_table_alloc_value(PintaCore *core, PintaReference *table, u32 capacity, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    PintaHeapObject *value;
    u32 template_capacity;

    pinta_assert(core != NULL);
    pinta_assert(result != NULL);
    pinta_assert(table != NULL);

    if (table->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (table->reference->block_kind != PINTA_KIND_PROPERTY_TABLE)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    template_capacity = pinta_property_table_ref_get_capacity(table);
    if (capacity <= template_capacity)
        PINTA_THROW(PINTA_EXCEPTION_INVALID_ARGUMENTS);

    value = pinta_property_table_alloc_object_value(core, table, capacity);
    if (value == NULL)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_MEMORY);

    result->reference = value;

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_property_table_get_hash_code(PintaCore *core, PintaReference *key, u32 *hash_code)
{
    PintaException exception = PINTA_OK;
    PintaType *key_type;
    u32 string_hash_code;

    if (key->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    key_type = pinta_core_get_type(core, key);
    if (!key_type->is_string)
        PINTA_THROW(PINTA_EXCEPTION_INVALID_ARGUMENTS);

    PINTA_CHECK(pinta_lib_string_to_string(core, key, key));
    PINTA_CHECK(pinta_lib_string_get_hashcode(core, key, &string_hash_code));

    *hash_code = string_hash_code & ~PINTA_PROPERTY_MASK;

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_property_table_find(PintaCore *core, PintaReference *property_table, PintaReference *property_key, PintaPropertySlot *result)
{
    PintaException exception = PINTA_OK;
    PintaProperty *table;
    u32 capacity;
    u32 hash_code;

    pinta_assert(property_table != NULL);
    pinta_assert(property_key != NULL);
    pinta_assert(result != NULL);

    if (property_table->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (property_table->reference->block_kind != PINTA_KIND_PROPERTY_TABLE)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    PINTA_CHECK(pinta_lib_property_table_get_hash_code(core, property_key, &hash_code));

    table = pinta_property_table_ref_get_table(property_table);
    capacity = pinta_property_table_ref_get_capacity(property_table);

    pinta_property_table_find(table, capacity, property_key, hash_code, result);

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_property_table_add_value(PintaCore *core, PintaReference *property_table, PintaReference *property_key, PintaPropertySlot *property_slot, u8 property_is_enumerable, u8 property_is_configurable, u8 property_is_writeable, PintaReference *property_value)
{
    PintaException exception = PINTA_OK;
    PintaPropertySlot slot;
    PintaProperty *table;
    PintaProperty *property;
    u32 capacity, length;
    u32 hash_code;
    u8 should_extend;

    pinta_assert(property_table != NULL);
    pinta_assert(property_key != NULL);
    pinta_assert(property_value != NULL);

    if (property_table->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (property_table->reference->block_kind != PINTA_KIND_PROPERTY_TABLE)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    pinta_debug_validate_slot(property_table, property_slot, property_key);

    PINTA_CHECK(pinta_lib_property_table_get_hash_code(core, property_key, &hash_code));

    table = pinta_property_table_ref_get_table(property_table);
    capacity = pinta_property_table_ref_get_capacity(property_table);
    length = pinta_property_table_ref_get_length(property_table);

    if (property_slot == NULL || !property_slot->is_valid)
        pinta_property_table_find(table, capacity, property_key, hash_code, &slot);
    else
        slot = *property_slot;

    should_extend = 0;
    if (!slot.is_found)
        should_extend = pinta_property_table_should_extend(capacity, length);

    if (!slot.is_valid || should_extend)
    {
        slot.is_valid = 0;

        PINTA_CHECK(pinta_lib_property_table_alloc_value(core, property_table, pinta_property_table_get_next_capacity(capacity), property_table));
        PINTA_CHECK(pinta_lib_property_table_add_value(core, property_table, property_key, &slot, property_is_enumerable, property_is_configurable, property_is_writeable, property_value));

        if (property_slot != NULL)
            *property_slot = slot;

        PINTA_RETURN();
    }

    pinta_property_table_ref_set_length(property_table, length + 1);
    property = pinta_property_table_ref_get(property_table, slot.property_id);

    pinta_property_set_hash_code(property, property_is_enumerable, property_is_configurable, property_is_writeable, 0, hash_code);
    pinta_property_set_key(property, property_key->reference);
    pinta_property_set_value(property, property_value->reference);

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_property_table_add_accessor(PintaCore *core, PintaReference *property_table, PintaReference *property_key, PintaPropertySlot *property_slot, u8 property_is_enumerable, u8 property_is_configurable, PintaReference *property_accessor_get, PintaReference *property_accessor_set)
{
    PintaException exception = PINTA_OK;
    PintaPropertySlot slot;
    PintaProperty *table;
    PintaProperty *property;
    u32 capacity, length;
    u32 hash_code;
    u8 should_extend;

    pinta_assert(property_table != NULL);
    pinta_assert(property_key != NULL);
    pinta_assert(property_accessor_get != NULL);
    pinta_assert(property_accessor_set != NULL);

    if (property_table->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (property_table->reference->block_kind != PINTA_KIND_PROPERTY_TABLE)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    pinta_debug_validate_slot(property_table, property_slot, property_key);

    PINTA_CHECK(pinta_lib_property_table_get_hash_code(core, property_key, &hash_code));

    table = pinta_property_table_ref_get_table(property_table);
    capacity = pinta_property_table_ref_get_capacity(property_table);
    length = pinta_property_table_ref_get_length(property_table);

    if (property_slot == NULL || !property_slot->is_valid)
        pinta_property_table_find(table, capacity, property_key, hash_code, &slot);
    else
        slot = *property_slot;

    should_extend = 0;
    if (!slot.is_found)
        should_extend = pinta_property_table_should_extend(capacity, length);

    if (!slot.is_valid || should_extend)
    {
        slot.is_valid = 0;

        PINTA_CHECK(pinta_lib_property_table_alloc_value(core, property_table, pinta_property_table_get_next_capacity(capacity), property_table));
        PINTA_CHECK(pinta_lib_property_table_add_accessor(core, property_table, property_key, &slot, property_is_enumerable, property_is_configurable, property_accessor_get, property_accessor_set));

        if (property_slot != NULL)
            *property_slot = slot;

        PINTA_RETURN();
    }

    pinta_property_table_ref_set_length(property_table, length + 1);
    property = pinta_property_table_ref_get(property_table, slot.property_id);

    pinta_property_set_hash_code(property, property_is_enumerable, property_is_configurable, 0, 1, hash_code);
    pinta_property_set_key(property, property_key->reference);
    pinta_property_set_accessor_get(property, property_accessor_get->reference);
    pinta_property_set_accessor_set(property, property_accessor_set->reference);

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_property_table_add_native(PintaCore *core, PintaReference *property_table, PintaReference *property_key, PintaPropertySlot *property_slot, u8 property_is_enumerable, u8 property_is_configurable, PintaPropertyNativeDelegate property_native_delegate, u32 property_native_token)
{
    PintaException exception = PINTA_OK;
    PintaPropertySlot slot;
    PintaProperty *table;
    PintaProperty *property;
    u32 capacity, length;
    u32 hash_code;
    u8 should_extend;

    pinta_assert(property_table != NULL);
    pinta_assert(property_key != NULL);
    pinta_assert(property_native_delegate != NULL);

    if (property_table->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (property_table->reference->block_kind != PINTA_KIND_PROPERTY_TABLE)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    pinta_debug_validate_slot(property_table, property_slot, property_key);

    PINTA_CHECK(pinta_lib_property_table_get_hash_code(core, property_key, &hash_code));

    table = pinta_property_table_ref_get_table(property_table);
    capacity = pinta_property_table_ref_get_capacity(property_table);
    length = pinta_property_table_ref_get_length(property_table);

    if (property_slot == NULL || !property_slot->is_valid)
        pinta_property_table_find(table, capacity, property_key, hash_code, &slot);
    else
        slot = *property_slot;

    should_extend = 0;
    if (!slot.is_found)
        should_extend = pinta_property_table_should_extend(capacity, length);

    if (!slot.is_valid || should_extend)
    {
        slot.is_valid = 0;

        PINTA_CHECK(pinta_lib_property_table_alloc_value(core, property_table, pinta_property_table_get_next_capacity(capacity), property_table));
        PINTA_CHECK(pinta_lib_property_table_add_native(core, property_table, property_key, &slot, property_is_enumerable, property_is_configurable, property_native_delegate, property_native_token));

        if (property_slot != NULL)
            *property_slot = slot;

        PINTA_RETURN();
    }

    pinta_property_table_ref_set_length(property_table, length + 1);
    property = pinta_property_table_ref_get(property_table, slot.property_id);

    pinta_property_set_hash_code(property, property_is_enumerable, property_is_configurable, 1, 1, hash_code);
    pinta_property_set_key(property, property_key->reference);
    pinta_property_set_native_delegate(property, property_native_delegate);
    pinta_property_set_native_token(property, property_native_token);

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_property_table_get_value(PintaCore *core, PintaReference *property_table, PintaPropertySlot *property_slot, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    PintaProperty *property;

    PINTA_UNUSED(core);

    pinta_assert(core != NULL);
    pinta_assert(property_table != NULL);
    pinta_assert(property_slot != NULL);
    pinta_assert(property_slot->is_valid && property_slot->is_found);
    pinta_assert(result != NULL);

    if (property_table->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (property_table->reference->block_kind != PINTA_KIND_PROPERTY_TABLE)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    if (property_slot->is_accessor)
        PINTA_THROW(PINTA_EXCEPTION_INVALID_ARGUMENTS);

    property = pinta_property_table_ref_get(property_table, property_slot->property_id);
    result->reference = pinta_property_get_value(property);

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_property_table_set_value(PintaCore *core, PintaReference *property_table, PintaPropertySlot *property_slot, PintaReference *value)
{
    PintaException exception = PINTA_OK;
    PintaProperty *property;

    PINTA_UNUSED(core);

    pinta_assert(core != NULL);
    pinta_assert(property_table != NULL);
    pinta_assert(property_slot != NULL);
    pinta_assert(property_slot->is_valid && property_slot->is_found);
    pinta_assert(value != NULL);

    if (property_table->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (property_table->reference->block_kind != PINTA_KIND_PROPERTY_TABLE)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    if (property_slot->is_accessor)
        PINTA_THROW(PINTA_EXCEPTION_INVALID_ARGUMENTS);

    property = pinta_property_table_ref_get(property_table, property_slot->property_id);
    pinta_property_set_value(property, value->reference);

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_property_table_get_accessor(PintaCore *core, PintaReference *property_table, PintaPropertySlot *property_slot, PintaReference *property_accessor_get, PintaReference *property_accessor_set)
{
    PintaException exception = PINTA_OK;
    PintaProperty *property;

    PINTA_UNUSED(core);

    pinta_assert(core != NULL);
    pinta_assert(property_table != NULL);
    pinta_assert(property_slot != NULL);
    pinta_assert(property_slot->is_valid && property_slot->is_found);

    if (property_table->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (property_table->reference->block_kind != PINTA_KIND_PROPERTY_TABLE)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    if (!property_slot->is_accessor)
        PINTA_THROW(PINTA_EXCEPTION_INVALID_ARGUMENTS);

    property = pinta_property_table_ref_get(property_table, property_slot->property_id);

    if (property_accessor_get != NULL)
        property_accessor_get->reference = pinta_property_get_accessor_get(property);

    if (property_accessor_set != NULL)
        property_accessor_set->reference = pinta_property_get_accessor_set(property);

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_property_table_set_accessor(PintaCore *core, PintaReference *property_table, PintaPropertySlot *property_slot, PintaReference *property_accessor_get, PintaReference *property_accessor_set)
{
    PintaException exception = PINTA_OK;
    PintaProperty *property;

    PINTA_UNUSED(core);

    pinta_assert(core != NULL);
    pinta_assert(property_table != NULL);
    pinta_assert(property_slot != NULL);
    pinta_assert(property_slot->is_valid && property_slot->is_found);
    pinta_assert(property_accessor_get != NULL);
    pinta_assert(property_accessor_set != NULL);

    if (property_table->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (property_table->reference->block_kind != PINTA_KIND_PROPERTY_TABLE)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    if (property_slot->is_accessor)
        PINTA_THROW(PINTA_EXCEPTION_INVALID_ARGUMENTS);

    property = pinta_property_table_ref_get(property_table, property_slot->property_id);
    pinta_property_set_accessor_get(property, property_accessor_get->reference);
    pinta_property_set_accessor_set(property, property_accessor_set->reference);

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_property_table_get_native(PintaCore *core, PintaReference *property_table, PintaPropertySlot *property_slot, PintaPropertyNativeDelegate *property_native_delegate, u32 *property_native_token)
{
    PintaException exception = PINTA_OK;
    PintaProperty *property;

    PINTA_UNUSED(core);

    pinta_assert(core != NULL);
    pinta_assert(property_table != NULL);
    pinta_assert(property_slot != NULL);
    pinta_assert(property_slot->is_valid && property_slot->is_found);

    if (property_table->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (property_table->reference->block_kind != PINTA_KIND_PROPERTY_TABLE)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    if (!property_slot->is_accessor)
        PINTA_THROW(PINTA_EXCEPTION_INVALID_ARGUMENTS);

    property = pinta_property_table_ref_get(property_table, property_slot->property_id);

    if (property_native_delegate != NULL)
        *property_native_delegate = pinta_property_get_native_delegate(property);

    if (property_native_token != NULL)
        *property_native_token = pinta_property_get_native_token(property);

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_property_table_set_native(PintaCore *core, PintaReference *property_table, PintaPropertySlot *property_slot, PintaPropertyNativeDelegate property_native_delegate, u32 property_native_token)
{
    PintaException exception = PINTA_OK;
    PintaProperty *property;

    PINTA_UNUSED(core);

    pinta_assert(core != NULL);
    pinta_assert(property_table != NULL);
    pinta_assert(property_slot != NULL);
    pinta_assert(property_slot->is_valid && property_slot->is_found);

    if (property_table->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (property_table->reference->block_kind != PINTA_KIND_PROPERTY_TABLE)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    if (property_slot->is_accessor)
        PINTA_THROW(PINTA_EXCEPTION_INVALID_ARGUMENTS);

    property = pinta_property_table_ref_get(property_table, property_slot->property_id);
    pinta_property_set_native_delegate(property, property_native_delegate);
    pinta_property_set_native_token(property, property_native_token);

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_property_table_debug_write(PintaCore *core, PintaReference *value, u32 max_depth, PintaJsonWriter *writer)
{
    PintaException exception = PINTA_OK;

    u32 length, table_capacity;
    u32 index;
    PintaProperty *table_entry;
    u32 key_hash_code;
    u32 native_token = 0;
    u32 is_accessor;
    u32 is_writeable_or_native;
    u32 is_configurable;

    struct
    {
        PintaReference key;
        PintaReference value;
        PintaReference get;
        PintaReference set;
    } gc;

    PINTA_GC_ENTER(core, gc);

    table_capacity = pinta_property_table_ref_get_capacity(value);
    length = pinta_property_table_ref_get_length(value);

    PINTA_CHECK(pinta_json_write_object_start(core, writer));
    PINTA_CHECK(pinta_json_write_property_string(core, writer, type_literal, property_table_literal));

    PINTA_CHECK(pinta_json_write_property_u32(core, writer, capacity_literal, table_capacity));
    PINTA_CHECK(pinta_json_write_property_u32(core, writer, length_literal, length));

    if (max_depth > 0)
    {
        PINTA_CHECK(pinta_json_write_property_name(core, writer, properties_literal));
        PINTA_CHECK(pinta_json_write_array_start(core, writer));

        for (index = 0; index < table_capacity; index++)
        {
            table_entry = pinta_property_table_ref_get(value, index);

            if (pinta_property_is_free(table_entry))
                continue;

            key_hash_code = pinta_property_get_hash_code(table_entry);
            gc.key.reference = pinta_property_get_key(table_entry);

            is_accessor = pinta_property_is_accessor(table_entry);
            is_writeable_or_native = pinta_property_is_writeable_or_native(table_entry);
            is_configurable = pinta_property_is_configurable(table_entry);

            if (is_accessor)
            {
                if (is_writeable_or_native)
                {
                    native_token = pinta_property_get_native_token(table_entry);
                }
                else
                {
                    gc.get.reference = pinta_property_get_accessor_get(table_entry);
                    gc.set.reference = pinta_property_get_accessor_set(table_entry);
                }
            }
            else
            {
                gc.value.reference = pinta_property_get_value(table_entry);
            }

            PINTA_CHECK(pinta_json_write_object_start(core, writer));

            PINTA_CHECK(pinta_json_write_property_name(core, writer, key_literal));
            PINTA_CHECK(pinta_lib_debug_write(core, &gc.key, max_depth > 0 ? max_depth - 1 : 0, writer));

            PINTA_CHECK(pinta_json_write_property_u32(core, writer, hashcode_literal, key_hash_code));
            PINTA_CHECK(pinta_json_write_property_bool(core, writer, is_configurable_literal, is_configurable));
            PINTA_CHECK(pinta_json_write_property_bool(core, writer, is_accessor_literal, is_accessor));

            if (is_accessor)
            {
                PINTA_CHECK(pinta_json_write_property_bool(core, writer, is_native_literal, is_writeable_or_native));

                if (is_writeable_or_native)
                {
                    PINTA_CHECK(pinta_json_write_property_u32(core, writer, token_literal, native_token));
                }
                else
                {
                    PINTA_CHECK(pinta_json_write_property_name(core, writer, get_literal));
                    if (max_depth > 0)
                        PINTA_CHECK(pinta_lib_debug_write(core, &gc.get, max_depth - 1, writer));
                    else
                        PINTA_CHECK(pinta_json_write_skipped(core, writer));

                    PINTA_CHECK(pinta_json_write_property_name(core, writer, set_literal));
                    if (max_depth > 0)
                        PINTA_CHECK(pinta_lib_debug_write(core, &gc.set, max_depth - 1, writer));
                    else
                        PINTA_CHECK(pinta_json_write_skipped(core, writer));
                }
            }
            else
            {
                PINTA_CHECK(pinta_json_write_property_bool(core, writer, is_writeable_literal, is_writeable_or_native));

                PINTA_CHECK(pinta_json_write_property_name(core, writer, value_literal));
                if (max_depth > 0)
                    PINTA_CHECK(pinta_lib_debug_write(core, &gc.value, max_depth - 1, writer));
                else
                    PINTA_CHECK(pinta_json_write_skipped(core, writer));

            }

            PINTA_CHECK(pinta_json_write_end(core, writer));
        }

        PINTA_CHECK(pinta_json_write_end(core, writer));
    }
    else
    {
        PINTA_CHECK(pinta_json_write_property_name(core, writer, properties_literal));
        PINTA_CHECK(pinta_json_write_skipped(core, writer));
    }

    PINTA_CHECK(pinta_json_write_end(core, writer));

PINTA_EXIT:
    PINTA_GC_EXIT(core);
    return exception;
}