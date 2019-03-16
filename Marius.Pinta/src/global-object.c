#include "pinta.h"

static wchar type_literal[] = { 116, 121, 112, 101, 0 };
static wchar global_object_literal[] = { 103, 108, 111, 98, 97, 108, 32, 111, 98, 106, 101, 99, 116, 0 };
static wchar body_literal[] = { 98, 111, 100, 121, 0 };

/* ACCESSORS */

PintaModuleDomain *pinta_global_object_get_domain(PintaHeapObject *global)
{
    pinta_assert(global != NULL);
    pinta_assert(global->block_kind == PINTA_KIND_GLOBAL_OBJECT);

    return global->data.global_object.global_domain;
}

PintaHeapObject *pinta_global_object_get_body(PintaHeapObject *global)
{
    pinta_assert(global != NULL);
    pinta_assert(global->block_kind == PINTA_KIND_GLOBAL_OBJECT);

    return global->data.global_object.global_body;
}

void pinta_global_object_set_domain(PintaHeapObject *global, PintaModuleDomain *domain)
{
    pinta_assert(global != NULL);
    pinta_assert(global->block_kind == PINTA_KIND_GLOBAL_OBJECT);

    global->data.global_object.global_domain = domain;
}

void pinta_global_object_set_body(PintaHeapObject *global, PintaHeapObject *body)
{
    pinta_assert(global != NULL);
    pinta_assert(global->block_kind == PINTA_KIND_GLOBAL_OBJECT);

    global->data.global_object.global_body = body;
}

PintaModuleDomain *pinta_global_object_ref_get_domain(PintaReference *global)
{
    pinta_assert(global != NULL);

    return pinta_global_object_get_domain(global->reference);
}

PintaHeapObject *pinta_global_object_ref_get_body(PintaReference *global)
{
    pinta_assert(global != NULL);

    return pinta_global_object_get_body(global->reference);
}

void pinta_global_object_ref_set_domain(PintaReference *global, PintaModuleDomain *domain)
{
    pinta_assert(global != NULL);

    pinta_global_object_set_domain(global->reference, domain);
}

void pinta_global_object_ref_set_body(PintaReference *global, PintaReference *body)
{
    pinta_assert(global != NULL);
    pinta_assert(body != NULL);

    pinta_global_object_set_body(global->reference, body->reference);
}

void pinta_global_object_ref_set_body_null(PintaReference *global)
{
    pinta_assert(global != NULL);

    pinta_global_object_set_body(global->reference, NULL);
}

/* GLOBAL OBJECT */

void pinta_global_object_init_type(PintaType *type)
{
    type->is_string = 0;

    type->gc_walk = pinta_global_object_walk;
    type->gc_relocate = pinta_global_object_relocate;

    type->get_member = pinta_lib_global_object_get_member;
    type->set_member = pinta_lib_global_object_set_member;

#if PINTA_DEBUG
    type->debug_write = pinta_lib_global_object_debug_write;
#endif
}

u32 pinta_global_object_walk(PintaHeapObject *object, PintaHeapObjectWalkerState *state, PintaHeapObject **result)
{
    PintaHeapObject *item;

    switch (state->field)
    {
    case 0:
        item = pinta_global_object_get_body(object);
        if (item != NULL)
        {
            *result = item;
            state->field = 1;
            return 1;
        }
        //fall through
    }

    return 0;
}

void pinta_global_object_relocate(PintaHeapObject *object, PintaHeapReloc *reloc, u32 count)
{
    PintaHeapReloc *entry;
    PintaHeapObject *item;

    item = pinta_global_object_get_body(object);
    if (item != NULL)
    {
        entry = pinta_gc_relocate_find(item, reloc, count);
        if (entry != NULL)
            pinta_global_object_set_body(object, item - entry->offset);
    }
}

PintaHeapObject *pinta_global_object_alloc(PintaCore *core, PintaModuleDomain *domain)
{
    PintaHeapObject *result;

    result = pinta_core_alloc(core, PINTA_KIND_GLOBAL_OBJECT, PINTA_FLAG_NONE, 1);
    if (result == NULL)
        return NULL;

    pinta_global_object_set_domain(result, domain);
    pinta_global_object_set_body(result, NULL);
    return result;
}

PintaException pinta_global_object_native_property(PintaCore *core, PintaReference *object, PintaReference *name, u32 native_token, u8 is_set, PintaReference *value)
{
    PintaException exception = PINTA_OK;
    PintaModuleDomain *domain;

    PINTA_UNUSED(name);

    pinta_assert(core != NULL);
    pinta_assert(object != NULL);
    pinta_assert(name != NULL);
    pinta_assert(value != NULL);

    domain = pinta_global_object_ref_get_domain(object);

    if (!is_set)
        PINTA_CHECK(pinta_lib_array_get_item(core, &domain->globals, native_token, value));
    else
        PINTA_CHECK(pinta_lib_array_set_item(core, &domain->globals, native_token, value));

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_global_object_alloc(PintaCore *core, PintaModuleDomain *domain, PintaReference *result)
{
    result->reference = pinta_global_object_alloc(core, domain);
    if (result->reference == NULL)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_OUT_OF_MEMORY);

    return PINTA_OK;
}

PintaException pinta_lib_global_object_get_member(PintaCore *core, PintaReference *object, PintaReference *name, u8 *is_accessor, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    PintaPropertySlot slot;
    u32 global_token;
    PintaModuleDomain *domain;
    PintaPropertyNativeDelegate delegate;
    PintaReference body;

    pinta_assert(core != NULL);
    pinta_assert(object != NULL);
    pinta_assert(name != NULL);
    pinta_assert(result != NULL);

    if (object->reference == NULL)
        return (PINTA_EXCEPTION_NULL_REFERENCE);

    if (object->reference->block_kind != PINTA_KIND_GLOBAL_OBJECT)
        return (PINTA_EXCEPTION_TYPE_MISMATCH);

    PINTA_GC_ENTER(core, body);

    body.reference = pinta_global_object_ref_get_body(object);
    if (body.reference == NULL)
    {
        PINTA_CHECK(pinta_lib_property_table_alloc(core, 4, &body));
        pinta_global_object_ref_set_body(object, &body);
    }

    domain = pinta_global_object_ref_get_domain(object);

    PINTA_CHECK(pinta_lib_property_table_find(core, &body, name, &slot));
    if (!slot.is_found)
    {
        if (is_accessor)
            *is_accessor = 0;

        PINTA_CHECK(pinta_lib_module_get_global_token(core, domain, name, &global_token));
        if (global_token == PINTA_CODE_TOKEN_EMPTY)
        {
            result->reference = NULL;
        }
        else
        {
            PINTA_CHECK(pinta_lib_property_table_add_native(core, &body, name, &slot, 1, 1, pinta_global_object_native_property, global_token));
            pinta_global_object_ref_set_body(object, &body);

            PINTA_CHECK(pinta_global_object_native_property(core, object, name, global_token, 0, result));
        }
    }
    else
    {
        if (slot.is_native)
        {
            if (is_accessor)
                *is_accessor = 0;

            PINTA_CHECK(pinta_lib_property_table_get_native(core, &body, &slot, &delegate, &global_token));
            PINTA_CHECK(delegate(core, object, name, global_token, 0, result));
        }
        else if (slot.is_accessor)
        {
            if (is_accessor)
                *is_accessor = 1;

            PINTA_CHECK(pinta_lib_property_table_get_accessor(core, &body, &slot, result, NULL));
        }
        else
        {
            if (is_accessor)
                *is_accessor = 0;

            PINTA_CHECK(pinta_lib_property_table_get_value(core, &body, &slot, result));
        }
    }

    PINTA_GC_RETURN(core);
}

PintaException pinta_lib_global_object_set_member(PintaCore *core, PintaReference *object, PintaReference *name, PintaReference *value, u8 *is_accessor, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    PintaPropertySlot slot;
    u32 global_token;
    PintaModuleDomain *domain;
    PintaPropertyNativeDelegate delegate;
    PintaReference body;

    pinta_assert(core != NULL);
    pinta_assert(object != NULL);
    pinta_assert(name != NULL);
    pinta_assert(result != NULL);

    if (object->reference == NULL)
        return (PINTA_EXCEPTION_NULL_REFERENCE);

    if (object->reference->block_kind != PINTA_KIND_GLOBAL_OBJECT)
        return (PINTA_EXCEPTION_TYPE_MISMATCH);

    PINTA_GC_ENTER(core, body);

    body.reference = pinta_global_object_ref_get_body(object);
    if (body.reference == NULL)
    {
        PINTA_CHECK(pinta_lib_property_table_alloc(core, 4, &body));
        pinta_global_object_ref_set_body(object, &body);
    }

    domain = pinta_global_object_ref_get_domain(object);

    PINTA_CHECK(pinta_lib_property_table_find(core, &body, name, &slot));
    if (!slot.is_found)
    {
        if (is_accessor)
            *is_accessor = 0;

        PINTA_CHECK(pinta_lib_module_get_global_token(core, domain, name, &global_token));
        if (global_token == PINTA_CODE_TOKEN_EMPTY)
        {
            result->reference = NULL;
        }
        else
        {
            PINTA_CHECK(pinta_lib_property_table_add_native(core, &body, name, &slot, 1, 1, pinta_global_object_native_property, global_token));
            pinta_global_object_ref_set_body(object, &body);

            PINTA_CHECK(pinta_global_object_native_property(core, object, name, global_token, 1, value));
        }
    }
    else
    {
        if (slot.is_native)
        {
            if (is_accessor)
                *is_accessor = 0;

            PINTA_CHECK(pinta_lib_property_table_get_native(core, &body, &slot, &delegate, &global_token));
            PINTA_CHECK(delegate(core, object, name, global_token, 1, value));
        }
        else if (slot.is_accessor)
        {
            if (is_accessor)
                *is_accessor = 1;

            PINTA_CHECK(pinta_lib_property_table_get_accessor(core, &body, &slot, NULL, result));
        }
        else
        {
            if (is_accessor)
                *is_accessor = 0;

            PINTA_CHECK(pinta_lib_property_table_set_value(core, &body, &slot, value));
        }
    }

    PINTA_GC_RETURN(core);
}

PintaException pinta_lib_global_object_debug_write(PintaCore *core, PintaReference *value, u32 max_depth, PintaJsonWriter *writer)
{
    PintaException exception = PINTA_OK;

    PintaReference body;

    PINTA_GC_ENTER(core, body);

    PINTA_CHECK(pinta_json_write_object_start(core, writer));
    PINTA_CHECK(pinta_json_write_property_string(core, writer, type_literal, global_object_literal));

    body.reference = pinta_global_object_ref_get_body(value);

    PINTA_CHECK(pinta_json_write_property_name(core, writer, body_literal));
    if (max_depth > 0)
        PINTA_CHECK(pinta_lib_debug_write(core, &body, max_depth - 1, writer));
    else
        PINTA_CHECK(pinta_json_write_skipped(core, writer));

    PINTA_CHECK(pinta_json_write_end(core, writer));

PINTA_EXIT:
    PINTA_GC_EXIT(core);
    return exception;
}