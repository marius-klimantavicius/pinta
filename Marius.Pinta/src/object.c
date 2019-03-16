#include "pinta.h"

static wchar null_literal[] = { 110, 117, 108, 108, 0 };
static wchar type_literal[] = { 116, 121, 112, 101, 0 };
static wchar object_literal[] = { 111, 98, 106, 101, 99, 116, 0 };
static wchar prototype_literal[] = { 112, 114, 111, 116, 111, 116, 121, 112, 101, 0 };
static wchar body_literal[] = { 98, 111, 100, 121, 0 };

/* OBJECT accessors */

PintaHeapObject *pinta_object_get_prototype(PintaHeapObject *object)
{
    pinta_assert(object != NULL);
    pinta_assert(object->block_kind == PINTA_KIND_OBJECT);

    return object->data.object.object_prototype;
}

PintaHeapObject *pinta_object_get_body(PintaHeapObject *object)
{
    pinta_assert(object != NULL);
    pinta_assert(object->block_kind == PINTA_KIND_OBJECT);

    return object->data.object.object_body;
}

void pinta_object_set_body(PintaHeapObject *object, PintaHeapObject *object_body)
{
    pinta_assert(object != NULL);
    pinta_assert(object->block_kind == PINTA_KIND_OBJECT);

    object->data.object.object_body = object_body;
}

void pinta_object_set_prototype(PintaHeapObject *object, PintaHeapObject *object_prototype)
{
    pinta_assert(object != NULL);
    pinta_assert(object->block_kind == PINTA_KIND_OBJECT);

    object->data.object.object_prototype = object_prototype;
}

PintaHeapObject *pinta_object_ref_get_prototype(PintaReference *reference)
{
    pinta_assert(reference != NULL);

    return pinta_object_get_prototype(reference->reference);
}

PintaHeapObject *pinta_object_ref_get_body(PintaReference *reference)
{
    pinta_assert(reference != NULL);

    return pinta_object_get_body(reference->reference);
}

void pinta_object_ref_set_body(PintaReference *reference, PintaReference *object_body)
{
    pinta_assert(reference != NULL);
    pinta_assert(object_body != NULL);

    pinta_object_set_body(reference->reference, object_body->reference);
}

void pinta_object_ref_set_prototype(PintaReference *reference, PintaReference *object_prototype)
{
    pinta_assert(reference != NULL);
    pinta_assert(object_prototype != NULL);

    pinta_object_set_body(reference->reference, object_prototype->reference);
}

/* OBJECT */

void pinta_object_init_type(PintaType *type)
{
    type->is_string = 0;

    type->gc_walk = pinta_object_walk;
    type->gc_relocate = pinta_object_relocate;

    type->to_integer = pinta_object_throw_engine;
    type->to_integer_value = pinta_object_to_integer_value;
    type->to_decimal = pinta_object_throw_engine;
    type->to_string = pinta_object_throw_engine;
    type->to_numeric = pinta_object_throw_engine;
    type->to_bool = pinta_object_to_bool;

    type->to_zero = pinta_object_to_zero;

    type->get_member = pinta_lib_object_get_member;
    type->set_member = pinta_lib_object_set_member;

    type->get_item = pinta_object_get_item;
    type->set_item = pinta_object_set_item;

#if PINTA_DEBUG
    type->debug_write = pinta_lib_object_debug_write;
#endif
}

u32 pinta_object_walk(PintaHeapObject *object, PintaHeapObjectWalkerState *state, PintaHeapObject **result)
{
    PintaHeapObject *item;

    switch (state->field)
    {
    case 0:
        item = pinta_object_get_body(object);
        if (item != NULL)
        {
            *result = item;
            state->field = 1;
            return 1;
        }
        // fall-through
    case 1:
        item = pinta_object_get_prototype(object);
        if (item != NULL)
        {
            *result = item;
            state->field = 2;
            return 1;
        }

        // fall-through
    }

    return 0;
}

void pinta_object_relocate(PintaHeapObject *object, PintaHeapReloc *reloc, u32 count)
{
    PintaHeapReloc *entry;
    PintaHeapObject *item;

    item = pinta_object_get_body(object);
    if (item != NULL)
    {
        entry = pinta_gc_relocate_find(item, reloc, count);
        if (entry != NULL)
            pinta_object_set_body(object, item - entry->offset);
    }

    item = pinta_object_get_prototype(object);
    if (item != NULL)
    {
        entry = pinta_gc_relocate_find(item, reloc, count);
        if (entry != NULL)
            pinta_object_set_prototype(object, item - entry->offset);
    }
}

PintaHeapObject *pinta_object_alloc(PintaCore *core, PintaReference *body, PintaReference *prototype)
{
    PintaHeapObject *result;

    pinta_assert(core != NULL);

    result = pinta_core_alloc(core, PINTA_KIND_OBJECT, PINTA_FLAG_NONE, 1);
    if (result == NULL)
        return NULL;

    if (body != NULL)
        pinta_object_set_body(result, body->reference);
    else
        pinta_object_set_body(result, NULL);

    if (prototype != NULL)
        pinta_object_set_prototype(result, prototype->reference);
    else
        pinta_object_set_prototype(result, NULL);

    return result;
}

PintaException pinta_object_throw_engine(PintaCore *core, PintaReference *value, PintaReference *result)
{
    PINTA_UNUSED(core);
    PINTA_UNUSED(value);
    PINTA_UNUSED(result);

    return PINTA_EXCEPTION(PINTA_EXCEPTION_ENGINE);
}

PintaException pinta_object_to_integer_value(PintaCore *core, PintaReference *value, i32 *result)
{
    PINTA_UNUSED(core);
    PINTA_UNUSED(value);
    PINTA_UNUSED(result);

    return PINTA_EXCEPTION(PINTA_EXCEPTION_ENGINE);
}

PintaException pinta_object_to_bool(PintaCore *core, PintaReference *value, u8 *result)
{
    PINTA_UNUSED(core);
    PINTA_UNUSED(value);
    PINTA_UNUSED(result);

    return PINTA_EXCEPTION(PINTA_EXCEPTION_ENGINE);
}

PintaException pinta_object_to_zero(PintaCore *core, PintaReference *result)
{
    PINTA_UNUSED(core);
    PINTA_UNUSED(result);

    return PINTA_EXCEPTION(PINTA_EXCEPTION_ENGINE);
}

PintaException pinta_object_get_item(PintaCore *core, PintaReference *array, u32 index, PintaReference *result)
{
    PINTA_UNUSED(core);
    PINTA_UNUSED(array);
    PINTA_UNUSED(index);
    PINTA_UNUSED(result);

    return PINTA_EXCEPTION(PINTA_EXCEPTION_ENGINE);
}

PintaException pinta_object_set_item(PintaCore *core, PintaReference *array, u32 index, PintaReference *value)
{
    PINTA_UNUSED(core);
    PINTA_UNUSED(array);
    PINTA_UNUSED(index);
    PINTA_UNUSED(value);

    return PINTA_EXCEPTION(PINTA_EXCEPTION_ENGINE);
}

PintaException pinta_lib_object_alloc(PintaCore *core, PintaReference *body, PintaReference *prototype, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    PintaHeapObject *object;

    pinta_assert(core != NULL);
    pinta_assert(result != NULL);

    object = pinta_object_alloc(core, body, prototype);
    if (object == NULL)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_MEMORY);

    result->reference = object;

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_object_get_property(PintaCore *core, PintaReference *object, PintaReference *name, PintaReference *property_owner, PintaPropertySlot *property_slot)
{
    PintaException exception = PINTA_OK;
    PintaType *key_type;
    PintaPropertySlot slot;
    PintaPropertySlot own_slot_value, *own_slot = NULL;
    struct
    {
        PintaReference key;
        PintaReference body;
        PintaReference owner;
    } gc;

    pinta_assert(core != NULL);
    pinta_assert(object != NULL);
    pinta_assert(name != NULL);
    pinta_assert(property_owner != NULL);
    pinta_assert(property_slot != NULL);

    if (object->reference == NULL)
        return (PINTA_EXCEPTION_NULL_REFERENCE);

    if (object->reference->block_kind != PINTA_KIND_OBJECT)
        return (PINTA_EXCEPTION_TYPE_MISMATCH);

    PINTA_GC_ENTER(core, gc);

    if (name->reference != NULL)
    {
        gc.key.reference = name->reference;
        key_type = pinta_core_get_type(core, &gc.key);

        if (!key_type->is_string)
            PINTA_CHECK(key_type->to_string(core, &gc.key, &gc.key));
    }
    else
    {
        PINTA_CHECK(pinta_lib_string_alloc_value(core, null_literal, PINTA_LITERAL_LENGTH(null_literal), &gc.key));
    }

    PINTA_CHECK(pinta_lib_string_to_string(core, &gc.key, &gc.key));

    own_slot_value.is_valid = 0;

    gc.owner.reference = object->reference;
    while (gc.owner.reference != NULL)
    {
        gc.body.reference = pinta_object_ref_get_body(&gc.owner);
        if (gc.body.reference != NULL)
        {
            PINTA_CHECK(pinta_lib_property_table_find(core, &gc.body, &gc.key, &slot));

            if (own_slot == NULL)
            {
                own_slot_value = slot;
                own_slot = &own_slot_value;
            }

            if (slot.is_valid && slot.is_found)
            {
                property_owner->reference = gc.body.reference;
                *property_slot = slot;
                PINTA_RETURN();
            }
        }

        gc.owner.reference = pinta_object_ref_get_prototype(&gc.owner);
    }

    property_owner->reference = pinta_object_ref_get_body(object);
    *property_slot = own_slot_value;

    PINTA_GC_RETURN(core);
}

PintaException pinta_lib_object_get_own_property(PintaCore *core, PintaReference *object, PintaReference *name, PintaReference *property_owner, PintaPropertySlot *property_slot)
{
    PintaException exception = PINTA_OK;
    PintaType *key_type;
    PintaPropertySlot slot;
    struct
    {
        PintaReference key;
        PintaReference body;
    } gc;

    pinta_assert(core != NULL);
    pinta_assert(object != NULL);
    pinta_assert(name != NULL);
    pinta_assert(property_owner != NULL);
    pinta_assert(property_slot != NULL);

    if (object->reference == NULL)
        return (PINTA_EXCEPTION_NULL_REFERENCE);

    if (object->reference->block_kind != PINTA_KIND_OBJECT)
        return (PINTA_EXCEPTION_TYPE_MISMATCH);

    PINTA_GC_ENTER(core, gc);

    if (name->reference != NULL)
    {
        gc.key.reference = name->reference;
        key_type = pinta_core_get_type(core, &gc.key);

        if (!key_type->is_string)
            PINTA_CHECK(key_type->to_string(core, &gc.key, &gc.key));
    }
    else
    {
        PINTA_CHECK(pinta_lib_string_alloc_value(core, null_literal, PINTA_LITERAL_LENGTH(null_literal), &gc.key));
    }

    PINTA_CHECK(pinta_lib_string_to_string(core, &gc.key, &gc.key));

    slot.is_valid = 0;

    gc.body.reference = pinta_object_ref_get_body(object);
    if (gc.body.reference != NULL)
    {
        PINTA_CHECK(pinta_lib_property_table_find(core, &gc.body, &gc.key, &slot));

        property_owner->reference = gc.body.reference;
        *property_slot = slot;
        PINTA_RETURN();
    }

    property_owner->reference = gc.body.reference;
    *property_slot = slot;

    PINTA_GC_RETURN(core);
}

PintaException pinta_lib_object_define_property(PintaCore *core, PintaReference *object, PintaReference *name, u8 is_enumerable, u8 is_configurable, u8 is_writeable, PintaReference *value, PintaReference *get, PintaReference *set)
{
    PintaException exception = PINTA_OK;
    PintaPropertySlot property_slot;
    PintaReference property_owner;

    pinta_assert(core != NULL);
    pinta_assert(object != NULL);
    pinta_assert(name != NULL);
    pinta_assert(value != NULL);
    pinta_assert(get != NULL);
    pinta_assert(set != NULL);

    PINTA_GC_ENTER(core, property_owner);

    if ((is_writeable || value->reference != NULL) && (get->reference != NULL || set->reference != NULL))
        PINTA_THROW(PINTA_EXCEPTION_INVALID_ARGUMENTS);

    PINTA_CHECK(pinta_lib_object_get_property(core, object, name, &property_owner, &property_slot));

    if (property_slot.is_valid && property_slot.is_found)
    {
        if (!property_slot.is_configurable)
            PINTA_THROW(PINTA_EXCEPTION_INVALID_ARGUMENTS);
    }

    if (property_owner.reference == NULL)
    {
        PINTA_CHECK(pinta_lib_property_table_alloc(core, 4, &property_owner));

        if (get->reference != NULL || set->reference != NULL)
            PINTA_CHECK(pinta_lib_property_table_add_accessor(core, &property_owner, name, NULL, is_enumerable, is_configurable, get, set));
        else
            PINTA_CHECK(pinta_lib_property_table_add_value(core, &property_owner, name, NULL, is_enumerable, is_configurable, is_writeable, value));
    }
    else
    {
        if (get->reference != NULL || set->reference != NULL)
            PINTA_CHECK(pinta_lib_property_table_add_accessor(core, &property_owner, name, &property_slot, is_enumerable, is_configurable, get, set));
        else
            PINTA_CHECK(pinta_lib_property_table_add_value(core, &property_owner, name, &property_slot, is_enumerable, is_configurable, is_writeable, value));
    }

    pinta_object_ref_set_body(object, &property_owner);

    PINTA_GC_RETURN(core);
}

PintaException pinta_lib_object_define_property_native(PintaCore *core, PintaReference *object, PintaReference *name, u8 is_enumerable, u8 is_configurable, PintaPropertyNativeDelegate property_native_delegate, u32 property_native_token)
{
    PintaException exception = PINTA_OK;
    PintaPropertySlot property_slot;
    PintaReference property_owner;

    pinta_assert(core != NULL);
    pinta_assert(object != NULL);
    pinta_assert(name != NULL);
    pinta_assert(property_native_delegate != NULL);

    PINTA_GC_ENTER(core, property_owner);

    PINTA_CHECK(pinta_lib_object_get_property(core, object, name, &property_owner, &property_slot));

    if (property_slot.is_valid && property_slot.is_found)
    {
        if (!property_slot.is_configurable)
            PINTA_THROW(PINTA_EXCEPTION_INVALID_ARGUMENTS);
    }

    if (property_owner.reference == NULL)
    {
        PINTA_CHECK(pinta_lib_property_table_alloc(core, 4, &property_owner));
        PINTA_CHECK(pinta_lib_property_table_add_native(core, &property_owner, name, NULL, is_enumerable, is_configurable, property_native_delegate, property_native_token));
    }
    else
    {
        PINTA_CHECK(pinta_lib_property_table_add_native(core, &property_owner, name, &property_slot, is_enumerable, is_configurable, property_native_delegate, property_native_token));
    }

    pinta_object_ref_set_body(object, &property_owner);

    PINTA_GC_RETURN(core);
}

PintaException pinta_lib_object_get_member(PintaCore *core, PintaReference *object, PintaReference *name, u8 *is_accessor, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    PintaPropertySlot property_slot;
    PintaPropertyNativeDelegate property_native_delegate;
    u32 property_native_token;
    PintaReference property_owner;

    pinta_assert(core != NULL);
    pinta_assert(object != NULL);
    pinta_assert(name != NULL);
    pinta_assert(result != NULL);

    PINTA_GC_ENTER(core, property_owner);

    PINTA_CHECK(pinta_lib_object_get_property(core, object, name, &property_owner, &property_slot));

    if (!property_slot.is_valid || !property_slot.is_found)
        PINTA_THROW(PINTA_EXCEPTION_INVALID_OPERATION);

    if (property_slot.is_accessor)
    {
        if (property_slot.is_native)
        {
            *is_accessor = 0;
            PINTA_CHECK(pinta_lib_property_table_get_native(core, &property_owner, &property_slot, &property_native_delegate, &property_native_token));
            if (property_native_delegate == NULL)
                PINTA_THROW(PINTA_EXCEPTION_INVALID_OPERATION);

            PINTA_CHECK(property_native_delegate(core, object, name, property_native_token, 0, result));
        }
        else
        {
            *is_accessor = 1;
            PINTA_CHECK(pinta_lib_property_table_get_accessor(core, &property_owner, &property_slot, result, NULL));
            if (result->reference == NULL)
                PINTA_THROW(PINTA_EXCEPTION_INVALID_OPERATION);
        }
    }
    else
    {
        *is_accessor = 0;
        PINTA_CHECK(pinta_lib_property_table_get_value(core, &property_owner, &property_slot, result));
    }

    PINTA_GC_RETURN(core);
}

PintaException pinta_lib_object_set_member(PintaCore *core, PintaReference *object, PintaReference *name, PintaReference *value, u8 *is_accessor, PintaReference *setter)
{
    PintaException exception = PINTA_OK;
    PintaPropertySlot property_slot;
    PintaPropertySlot prototype_slot;
    PintaPropertyNativeDelegate property_native_delegate;
    u32 property_native_token;
    struct
    {
        PintaReference property_owner;
        PintaReference prototype_owner;
    } gc;

    pinta_assert(core != NULL);
    pinta_assert(object != NULL);
    pinta_assert(name != NULL);
    pinta_assert(value != NULL);

    PINTA_GC_ENTER(core, gc);

    PINTA_CHECK(pinta_lib_object_get_own_property(core, object, name, &gc.property_owner, &property_slot));

    if (!property_slot.is_valid || !property_slot.is_found)
    {
        PINTA_CHECK(pinta_lib_object_get_property(core, object, name, &gc.prototype_owner, &prototype_slot));
        if (prototype_slot.is_found)
        {
            if (prototype_slot.is_accessor)
            {
                property_slot = prototype_slot;
                gc.property_owner.reference = gc.prototype_owner.reference;
            }
            else if (!prototype_slot.is_writeable)
            {
                PINTA_THROW(PINTA_EXCEPTION_INVALID_OPERATION);
            }
        }
    }

    if (!property_slot.is_valid || !property_slot.is_found)
    {
        *is_accessor = 0;
        if (gc.property_owner.reference == NULL)
        {
            PINTA_CHECK(pinta_lib_property_table_alloc(core, 4, &gc.property_owner));
            PINTA_CHECK(pinta_lib_property_table_add_value(core, &gc.property_owner, name, NULL, 1, 1, 1, value));
        }
        else
        {
            PINTA_CHECK(pinta_lib_property_table_add_value(core, &gc.property_owner, name, &property_slot, 1, 1, 1, value));
        }

        pinta_object_ref_set_body(object, &gc.property_owner);
        PINTA_RETURN();
    }

    if (property_slot.is_accessor)
    {
        if (property_slot.is_native)
        {
            *is_accessor = 0;
            PINTA_CHECK(pinta_lib_property_table_get_native(core, &gc.property_owner, &property_slot, &property_native_delegate, &property_native_token));
            if (property_native_delegate == NULL)
                PINTA_THROW(PINTA_EXCEPTION_INVALID_OPERATION);

            PINTA_CHECK(property_native_delegate(core, object, name, property_native_token, 1, value));
        }
        else
        {
            *is_accessor = 1;
            PINTA_CHECK(pinta_lib_property_table_get_accessor(core, &gc.property_owner, &property_slot, NULL, setter));
            if (setter->reference == NULL)
                PINTA_THROW(PINTA_EXCEPTION_INVALID_OPERATION);
        }
    }
    else
    {
        *is_accessor = 0;
        PINTA_CHECK(pinta_lib_property_table_set_value(core, &gc.property_owner, &property_slot, value));
    }

    PINTA_GC_RETURN(core);
}

PintaException pinta_lib_object_debug_write(PintaCore *core, PintaReference *value, u32 max_depth, PintaJsonWriter *writer)
{
    PintaException exception = PINTA_OK;
    
    struct
    {
        PintaReference prototype;
        PintaReference body;
    } gc;

    PINTA_GC_ENTER(core, gc);

    PINTA_CHECK(pinta_json_write_object_start(core, writer));
    PINTA_CHECK(pinta_json_write_property_string(core, writer, type_literal, object_literal));

    if (max_depth > 0)
    {
        gc.prototype.reference = pinta_object_ref_get_prototype(value);
        PINTA_CHECK(pinta_json_write_property_name(core, writer, prototype_literal));
        PINTA_CHECK(pinta_lib_debug_write(core, &gc.prototype, max_depth - 1, writer));

        gc.body.reference = pinta_object_ref_get_body(value);
        PINTA_CHECK(pinta_json_write_property_name(core, writer, body_literal));
        PINTA_CHECK(pinta_lib_debug_write(core, &gc.body, max_depth - 1, writer));
    }
    else
    {
        PINTA_CHECK(pinta_json_write_property_name(core, writer, prototype_literal));
        PINTA_CHECK(pinta_json_write_skipped(core, writer));
        
        PINTA_CHECK(pinta_json_write_property_name(core, writer, body_literal));
        PINTA_CHECK(pinta_json_write_skipped(core, writer));
    }

    PINTA_CHECK(pinta_json_write_end(core, writer));

PINTA_EXIT:
    PINTA_GC_EXIT(core);
    return exception;
}