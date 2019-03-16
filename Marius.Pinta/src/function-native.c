#include "pinta.h"

static wchar null_literal[] = { 110, 117, 108, 108, 0 };
static wchar type_literal[] = { 116, 121, 112, 101, 0 };
static wchar native_function_literal[] = { 110, 97, 116, 105, 118, 101, 32, 102, 117, 110, 99, 116, 105, 111, 110, 0 };
static wchar tag_literal[] = { 116, 97, 103, 0 };
static wchar binding_literal[] = { 98, 105, 110, 100, 105, 110, 103, 0 };
static wchar closure_literal[] = { 99, 108, 111, 115, 117, 114, 101, 0 };
static wchar prototype_literal[] = { 112, 114, 111, 116, 111, 116, 121, 112, 101, 0 };
static wchar object_literal[] = { 111, 98, 106, 101, 99, 116, 0 };

/* FUNCTION native accessors */

PintaFunctionDelegate pinta_function_native_get_delegate(PintaHeapObject *function)
{
    pinta_assert(function != NULL);
    pinta_assert(function->block_kind == PINTA_KIND_FUNCTION_NATIVE);

    return function->data.function_native.function_delegate;
}

u32 pinta_function_native_get_tag(PintaHeapObject *function)
{
    pinta_assert(function != NULL);
    pinta_assert(function->block_kind == PINTA_KIND_FUNCTION_NATIVE);

    return function->data.function_native.function_tag;
}

PintaFunctionBody *pinta_function_native_get_body_value(PintaHeapObject *function)
{
    pinta_assert(function != NULL);
    pinta_assert(function->block_kind == PINTA_KIND_FUNCTION_NATIVE);
    pinta_assert(function->block_length > 1);

    return (PintaFunctionBody*)&function[1];
}

void pinta_function_native_get_body(PintaHeapObject *function, PintaReference *function_binding, PintaReference *function_closure, PintaReference *function_prototype, PintaReference *function_object)
{
    PintaFunctionBody *function_body;

    function_body = pinta_function_native_get_body_value(function);

    if (function_binding != NULL)
        function_binding->reference = pinta_function_body_get_binding(function_body);

    if (function_closure != NULL)
        function_closure->reference = pinta_function_body_get_closure(function_body);

    if (function_prototype != NULL)
        function_prototype->reference = pinta_function_body_get_prototype(function_body);

    if (function_object != NULL)
        function_object->reference = pinta_function_body_get_object(function_body);
}

void pinta_function_native_set_delegate(PintaHeapObject *function, PintaFunctionDelegate function_delegate)
{
    pinta_assert(function != NULL);
    pinta_assert(function->block_kind == PINTA_KIND_FUNCTION_NATIVE);

    function->data.function_native.function_delegate = function_delegate;
}

void pinta_function_native_set_tag(PintaHeapObject *function, u32 function_tag)
{
    pinta_assert(function != NULL);
    pinta_assert(function->block_kind == PINTA_KIND_FUNCTION_NATIVE);

    function->data.function_native.function_tag = function_tag;
}

void pinta_function_native_set_body(PintaHeapObject *function, PintaReference *function_binding, PintaReference *function_closure, PintaReference *function_prototype, PintaReference *function_object)
{
    PintaFunctionBody *function_body;

    pinta_assert(function != NULL);
    pinta_assert(function->block_kind == PINTA_KIND_FUNCTION_NATIVE);

    function_body = pinta_function_native_get_body_value(function);

    if (function_binding != NULL)
        pinta_function_body_set_binding(function_body, function_binding->reference);

    if (function_closure != NULL)
        pinta_function_body_set_closure(function_body, function_closure->reference);

    if (function_prototype != NULL)
        pinta_function_body_set_prototype(function_body, function_prototype->reference);

    if (function_object != NULL)
        pinta_function_body_set_object(function_body, function_object->reference);
}

PintaFunctionDelegate pinta_function_native_ref_get_delegate(PintaReference *function)
{
    pinta_assert(function != NULL);
    pinta_assert(function->reference != NULL);

    return pinta_function_native_get_delegate(function->reference);
}

u32 pinta_function_native_ref_get_tag(PintaReference *function)
{
    pinta_assert(function != NULL);
    pinta_assert(function->reference != NULL);

    return pinta_function_native_get_tag(function->reference);
}

void pinta_function_native_ref_get_body(PintaReference *function, PintaReference *function_binding, PintaReference *function_closure, PintaReference *function_prototype, PintaReference *function_object)
{
    pinta_assert(function != NULL);
    pinta_assert(function->reference != NULL);

    pinta_function_native_get_body(function->reference, function_binding, function_closure, function_prototype, function_object);
}

void pinta_function_native_ref_set_delegate(PintaReference *function, PintaFunctionDelegate function_delegate)
{
    pinta_assert(function != NULL);
    pinta_assert(function->reference != NULL);

    pinta_function_native_set_delegate(function->reference, function_delegate);
}

void pinta_function_native_ref_set_tag(PintaReference *function, u32 function_tag)
{
    pinta_assert(function != NULL);
    pinta_assert(function->reference != NULL);

    pinta_function_native_set_tag(function->reference, function_tag);
}

void pinta_function_native_ref_set_body(PintaReference *function, PintaReference *function_binding, PintaReference *function_closure, PintaReference *function_prototype, PintaReference *function_object)
{
    pinta_assert(function != NULL);
    pinta_assert(function->reference != NULL);

    pinta_function_native_set_body(function->reference, function_binding, function_closure, function_prototype, function_object);
}

/* FUNCTION */

void pinta_function_native_init_type(PintaType *type)
{
    type->is_string = 0;

    type->gc_walk = pinta_function_native_walk;
    type->gc_relocate = pinta_function_native_relocate;

    type->get_member = pinta_lib_function_native_get_member;
    type->set_member = pinta_lib_function_native_set_member;

#if PINTA_DEBUG
    type->debug_write = pinta_lib_function_native_debug_write;
#endif
}

u32 pinta_function_native_walk(PintaHeapObject *object, PintaHeapObjectWalkerState *state, PintaHeapObject **result)
{
    PintaFunctionBody *function_body;

    pinta_assert(object != NULL);
    pinta_assert(state != NULL);
    pinta_assert(result != NULL);
    pinta_assert(object->block_kind == PINTA_KIND_FUNCTION_NATIVE);

    *result = NULL;

    function_body = pinta_function_native_get_body_value(object);
    return pinta_function_body_walk(function_body, state, result);
}

void pinta_function_native_relocate(PintaHeapObject *object, PintaHeapReloc *reloc, u32 count)
{
    PintaFunctionBody *function_body;

    pinta_gc_relocate_validate(PINTA_KIND_FUNCTION_NATIVE, object, reloc, count);

    function_body = pinta_function_native_get_body_value(object);
    pinta_function_body_relocate(function_body, reloc, count);
}

PintaHeapObject *pinta_function_native_alloc_object(PintaCore *core, PintaFunctionDelegate function_delegate, u32 function_tag, PintaReference *function_binding, PintaReference *function_closure, PintaReference *function_object, PintaReference *function_prototype)
{
    PintaHeapObject *result;
    u32 block_length;

    pinta_assert(core != NULL);

    block_length = (sizeof(PintaHeapObject) + sizeof(PintaFunctionBody) + sizeof(PintaHeapObject) - 1) / sizeof(PintaHeapObject);

    result = pinta_core_alloc(core, PINTA_KIND_FUNCTION_NATIVE, PINTA_FLAG_NONE, block_length);
    if (result == NULL)
        return NULL;

    if (function_binding == NULL)
        function_binding = &core->null;

    if (function_closure == NULL)
        function_closure = &core->null;

    if (function_prototype == NULL)
        function_prototype = &core->null;

    if (function_object == NULL)
        function_object = &core->null;

    pinta_function_native_set_delegate(result, function_delegate);
    pinta_function_native_set_tag(result, function_tag);
    pinta_function_native_set_body(result, function_binding, function_closure, function_prototype, function_object);

    return result;
}

PintaException pinta_lib_function_native_alloc(PintaCore *core, PintaFunctionDelegate function_delegate, u32 function_tag, PintaReference *function_binding, PintaReference *function_closure, PintaReference *function_object, PintaReference *function_prototype, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    PintaHeapObject *value;

    pinta_assert(core != NULL);
    pinta_assert(result != NULL);

    if (function_closure != NULL && function_closure->reference != NULL && function_closure->reference->block_kind != PINTA_KIND_ARRAY)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    value = pinta_function_native_alloc_object(core, function_delegate, function_tag, function_binding, function_closure, function_object, function_prototype);
    if (value == NULL)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_MEMORY);

    result->reference = value;

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_function_native_get_member(PintaCore *core, PintaReference *object, PintaReference *name, u8 *is_accessor, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    PintaType *type;
    wchar *name_data;
    u32 name_length;
    PintaReference function_object;

    pinta_assert(core != NULL);
    pinta_assert(object != NULL);
    pinta_assert(name != NULL);
    pinta_assert(result != NULL);

    if (object->reference == NULL)
        return (PINTA_EXCEPTION_NULL_REFERENCE);

    if (object->reference->block_kind != PINTA_KIND_FUNCTION_NATIVE)
        return (PINTA_EXCEPTION_TYPE_MISMATCH);

    PINTA_GC_ENTER(core, function_object);

    if (name->reference == NULL)
        PINTA_CHECK(pinta_lib_string_alloc_value(core, null_literal, PINTA_LITERAL_LENGTH(null_literal), name));

    type = pinta_core_get_type(core, name);
    PINTA_CHECK(type->to_string(core, name, name));
    PINTA_CHECK(pinta_lib_string_to_string(core, name, name));

    name_data = pinta_string_ref_get_data(name);
    name_length = pinta_string_ref_get_length(name);

    if (name_length == PINTA_LITERAL_LENGTH(prototype_literal) && memcmp(prototype_literal, name_data, sizeof(wchar) * name_length) == 0)
    {
        if (is_accessor)
            *is_accessor = 0;

        PINTA_CHECK(pinta_lib_function_native_get_body(core, object, NULL, NULL, &function_object, NULL));
        if (function_object.reference == NULL)
        {
            PINTA_CHECK(pinta_lib_object_alloc(core, NULL, NULL, &function_object));
            PINTA_CHECK(pinta_lib_function_native_set_body(core, object, NULL, NULL, &function_object, NULL));
        }

        result->reference = function_object.reference;

        PINTA_RETURN();
    }

    PINTA_CHECK(pinta_lib_function_native_get_body(core, object, NULL, NULL, NULL, &function_object));
    if (function_object.reference == NULL)
    {
        PINTA_CHECK(pinta_lib_object_alloc(core, NULL, NULL, &function_object));
        PINTA_CHECK(pinta_lib_function_native_set_body(core, object, NULL, NULL, NULL, &function_object));
    }

    PINTA_CHECK(pinta_lib_object_get_member(core, &function_object, name, is_accessor, result));

    PINTA_GC_RETURN(core);
}

PintaException pinta_lib_function_native_set_member(PintaCore *core, PintaReference *object, PintaReference *name, PintaReference *value, u8 *is_accessor, PintaReference *setter)
{
    PintaException exception = PINTA_OK;
    PintaType *type;
    wchar *name_data;
    u32 name_length;
    PintaReference function_object;

    pinta_assert(core != NULL);
    pinta_assert(object != NULL);
    pinta_assert(name != NULL);
    pinta_assert(value != NULL);

    PINTA_GC_ENTER(core, function_object);

    if (object->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (object->reference->block_kind != PINTA_KIND_FUNCTION_NATIVE)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    if (name->reference == NULL)
        PINTA_CHECK(pinta_lib_string_alloc_value(core, null_literal, PINTA_LITERAL_LENGTH(null_literal), name));

    type = pinta_core_get_type(core, name);
    PINTA_CHECK(type->to_string(core, name, name));
    PINTA_CHECK(pinta_lib_string_to_string(core, name, name));

    name_data = pinta_string_ref_get_data(name);
    name_length = pinta_string_ref_get_length(name);

    if (name_length == PINTA_LITERAL_LENGTH(prototype_literal) && memcmp(prototype_literal, name_data, sizeof(wchar) * name_length) == 0)
    {
        if (is_accessor)
            *is_accessor = 0;
        PINTA_THROW(PINTA_EXCEPTION_INVALID_OPERATION);
    }

    PINTA_CHECK(pinta_lib_function_native_get_body(core, object, NULL, NULL, NULL, &function_object));
    if (function_object.reference == NULL)
    {
        PINTA_CHECK(pinta_lib_object_alloc(core, NULL, NULL, &function_object));
        PINTA_CHECK(pinta_lib_function_native_set_body(core, object, NULL, NULL, NULL, &function_object));
    }

    PINTA_CHECK(pinta_lib_object_set_member(core, &function_object, name, value, is_accessor, setter));

    PINTA_GC_RETURN(core);
}

PintaException pinta_lib_function_native_get_body(PintaCore *core, PintaReference *function, PintaReference *function_binding, PintaReference *function_closure, PintaReference *function_prototype, PintaReference *function_object)
{
    pinta_assert(core != NULL);
    pinta_assert(function != NULL);

    if (function->reference == NULL)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_NULL_REFERENCE);

    if (function->reference->block_kind != PINTA_KIND_FUNCTION_NATIVE)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_TYPE_MISMATCH);

    pinta_function_native_ref_get_body(function, function_binding, function_closure, function_prototype, function_object);

    return PINTA_OK;
}

PintaException pinta_lib_function_native_set_body(PintaCore *core, PintaReference *function, PintaReference *function_binding, PintaReference *function_closure, PintaReference *function_prototype, PintaReference *function_object)
{
    pinta_assert(core != NULL);
    pinta_assert(function != NULL);

    if (function->reference == NULL)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_NULL_REFERENCE);

    if (function->reference->block_kind != PINTA_KIND_FUNCTION_NATIVE)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_TYPE_MISMATCH);

    pinta_function_native_ref_set_body(function, function_binding, function_closure, function_prototype, function_object);

    return PINTA_OK;
}

PintaException pinta_lib_function_native_debug_write(PintaCore *core, PintaReference *value, u32 max_depth, PintaJsonWriter *writer)
{
    PintaException exception = PINTA_OK;
    u32 tag;

    struct
    {
        PintaReference function_binding;
        PintaReference function_closure;
        PintaReference function_prototype;
        PintaReference function_object;
    } gc;

    PINTA_GC_ENTER(core, gc);

    tag = pinta_function_native_ref_get_tag(value);

    PINTA_CHECK(pinta_json_write_object_start(core, writer));

    PINTA_CHECK(pinta_json_write_property_string(core, writer, type_literal, native_function_literal));
    PINTA_CHECK(pinta_json_write_property_u32(core, writer, tag_literal, tag));

    if (max_depth > 0)
    {
        pinta_function_native_ref_get_body(value, &gc.function_binding, &gc.function_closure, &gc.function_prototype, &gc.function_object);

        PINTA_CHECK(pinta_json_write_property_name(core, writer, binding_literal));
        PINTA_CHECK(pinta_lib_debug_write(core, &gc.function_binding, max_depth - 1, writer));

        PINTA_CHECK(pinta_json_write_property_name(core, writer, closure_literal));
        PINTA_CHECK(pinta_lib_debug_write(core, &gc.function_closure, max_depth - 1, writer));

        PINTA_CHECK(pinta_json_write_property_name(core, writer, prototype_literal));
        PINTA_CHECK(pinta_lib_debug_write(core, &gc.function_prototype, max_depth - 1, writer));

        PINTA_CHECK(pinta_json_write_property_name(core, writer, object_literal));
        PINTA_CHECK(pinta_lib_debug_write(core, &gc.function_object, max_depth - 1, writer));
    }
    else
    {
        PINTA_CHECK(pinta_json_write_property_name(core, writer, binding_literal));
        PINTA_CHECK(pinta_json_write_skipped(core, writer));

        PINTA_CHECK(pinta_json_write_property_name(core, writer, closure_literal));
        PINTA_CHECK(pinta_json_write_skipped(core, writer));

        PINTA_CHECK(pinta_json_write_property_name(core, writer, prototype_literal));
        PINTA_CHECK(pinta_json_write_skipped(core, writer));

        PINTA_CHECK(pinta_json_write_property_name(core, writer, object_literal));
        PINTA_CHECK(pinta_json_write_skipped(core, writer));
    }

    PINTA_CHECK(pinta_json_write_end(core, writer));

PINTA_EXIT:
    PINTA_GC_EXIT(core);
    return exception;
}
