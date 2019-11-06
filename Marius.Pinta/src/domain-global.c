#include "pinta.h"

/* ACCESSORS */

PintaModuleDomain *pinta_domain_global_get_domain(PintaHeapObject *global)
{
    pinta_assert(global != NULL);
    pinta_assert(global->block_kind == PINTA_KIND_DOMAIN_GLOBAL);

    return global->data.domain_global.global_domain;
}

u32 pinta_domain_global_get_token(PintaHeapObject *global)
{
    pinta_assert(global != NULL);
    pinta_assert(global->block_kind == PINTA_KIND_DOMAIN_GLOBAL);

    return global->data.domain_global.global_token;
}

void pinta_domain_global_set_domain(PintaHeapObject *global, PintaModuleDomain *domain)
{
    pinta_assert(global != NULL);
    pinta_assert(global->block_kind == PINTA_KIND_DOMAIN_GLOBAL);

    global->data.domain_global.global_domain = domain;
}

void pinta_domain_global_set_token(PintaHeapObject *global, u32 token)
{
    pinta_assert(global != NULL);
    pinta_assert(global->block_kind == PINTA_KIND_DOMAIN_GLOBAL);

    global->data.domain_global.global_token = token;
}

PintaModuleDomain *pinta_domain_global_ref_get_domain(PintaReference *global)
{
    pinta_assert(global != NULL);

    return pinta_domain_global_get_domain(global->reference);
}

u32 pinta_domain_global_ref_get_token(PintaReference *global)
{
    pinta_assert(global != NULL);

    return pinta_domain_global_get_token(global->reference);
}

void pinta_domain_global_ref_set_domain(PintaReference *global, PintaModuleDomain *domain)
{
    pinta_assert(global != NULL);

    pinta_domain_global_set_domain(global->reference, domain);
}

void pinta_domain_global_ref_set_token(PintaReference *global, u32 token)
{
    pinta_assert(global != NULL);

    pinta_domain_global_set_token(global->reference, token);
}

/* global OBJECT */

void pinta_domain_global_init_type(PintaType *type)
{
    type->is_string = 0;

    /* Nothing to walk or relocate - we do not have any references*/

    type->to_integer = pinta_lib_domain_global_to_integer;
    type->to_integer_value = pinta_lib_domain_global_to_integer_value;
    type->to_decimal = pinta_lib_domain_global_to_decimal;
    type->to_string = pinta_lib_domain_global_to_string;
    type->to_numeric = pinta_lib_domain_global_to_numeric;
    type->to_bool = pinta_lib_domain_global_to_bool;
    type->to_zero = pinta_lib_domain_global_to_zero;

    type->get_member = pinta_lib_domain_global_get_member;
    type->set_member = pinta_lib_domain_global_set_member;

    type->get_item = pinta_lib_domain_global_get_item;
    type->set_item = pinta_lib_domain_global_set_item;

    type->get_length = pinta_lib_domain_global_get_length;
    type->get_char = pinta_lib_domain_global_get_char;

#if PINTA_DEBUG
    type->debug_write = pinta_lib_domain_global_debug_write;
#endif
}

PintaHeapObject *pinta_domain_global_alloc(PintaCore *core, PintaModuleDomain *domain, u32 token)
{
    PintaHeapObject *result;

    result = pinta_core_alloc(core, PINTA_KIND_DOMAIN_GLOBAL, PINTA_FLAG_NONE, 1);
    if (result == NULL)
        return NULL;

    pinta_domain_global_set_domain(result, domain);
    pinta_domain_global_set_token(result, token);
    return result;
}

PintaException pinta_lib_domain_global_alloc(PintaCore *core, PintaModuleDomain *domain, u32 token, PintaReference *result)
{
    result->reference = pinta_domain_global_alloc(core, domain, token);
    if (result->reference == NULL)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_OUT_OF_MEMORY);

    return PINTA_OK;
}

PintaException pinta_lib_domain_global_get_value(PintaCore *core, PintaReference *value, PintaReference *result)
{
    PintaModuleDomain *domain;
    u32 token;

    pinta_assert(core != NULL);
    pinta_assert(value != NULL);
    pinta_assert(result != NULL);

    domain = pinta_domain_global_ref_get_domain(value);
    token = pinta_domain_global_ref_get_token(value);

    return pinta_lib_array_get_item(core, &domain->globals, token, result);
}

PintaException pinta_lib_domain_global_to_integer(PintaCore *core, PintaReference *value, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    PintaReference global;
    PintaType *type;

    pinta_assert(core != NULL);
    pinta_assert(value != NULL);
    pinta_assert(result != NULL);

    PINTA_GC_ENTER(core, global);

    PINTA_CHECK(pinta_lib_domain_global_get_value(core, value, &global));
    type = pinta_core_get_type(core, &global);

    PINTA_CHECK(type->to_integer(core, &global, result));

    PINTA_GC_RETURN(core);
}

PintaException pinta_lib_domain_global_to_integer_value(PintaCore *core, PintaReference *value, i32 *result)
{
    PintaException exception = PINTA_OK;
    PintaReference global;
    PintaType *type;

    pinta_assert(core != NULL);
    pinta_assert(value != NULL);
    pinta_assert(result != NULL);

    PINTA_GC_ENTER(core, global);

    PINTA_CHECK(pinta_lib_domain_global_get_value(core, value, &global));
    type = pinta_core_get_type(core, &global);

    PINTA_CHECK(type->to_integer_value(core, &global, result));

    PINTA_GC_RETURN(core);
}

PintaException pinta_lib_domain_global_to_decimal(PintaCore *core, PintaReference *value, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    PintaReference global;
    PintaType *type;

    pinta_assert(core != NULL);
    pinta_assert(value != NULL);
    pinta_assert(result != NULL);

    PINTA_GC_ENTER(core, global);

    PINTA_CHECK(pinta_lib_domain_global_get_value(core, value, &global));
    type = pinta_core_get_type(core, &global);

    PINTA_CHECK(type->to_decimal(core, &global, result));

    PINTA_GC_RETURN(core);
}

PintaException pinta_lib_domain_global_to_string(PintaCore *core, PintaReference *value, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    PintaReference global;
    PintaType *type;

    pinta_assert(core != NULL);
    pinta_assert(value != NULL);
    pinta_assert(result != NULL);

    PINTA_GC_ENTER(core, global);

    PINTA_CHECK(pinta_lib_domain_global_get_value(core, value, &global));
    type = pinta_core_get_type(core, &global);

    PINTA_CHECK(type->to_string(core, &global, result));

    PINTA_GC_RETURN(core);
}

PintaException pinta_lib_domain_global_to_numeric(PintaCore *core, PintaReference *value, PintaReference *result) 
{
    PintaException exception = PINTA_OK;
    PintaReference global;
    PintaType *type;

    pinta_assert(core != NULL);
    pinta_assert(value != NULL);
    pinta_assert(result != NULL);

    PINTA_GC_ENTER(core, global);

    PINTA_CHECK(pinta_lib_domain_global_get_value(core, value, &global));
    type = pinta_core_get_type(core, &global);

    PINTA_CHECK(type->to_numeric(core, &global, result));

    PINTA_GC_RETURN(core);
}

PintaException pinta_lib_domain_global_to_bool(PintaCore *core, PintaReference *value, u8 *result)
{
    PintaException exception = PINTA_OK;
    PintaReference global;
    PintaType *type;

    pinta_assert(core != NULL);
    pinta_assert(value != NULL);
    pinta_assert(result != NULL);

    PINTA_GC_ENTER(core, global);

    PINTA_CHECK(pinta_lib_domain_global_get_value(core, value, &global));
    type = pinta_core_get_type(core, &global);

    PINTA_CHECK(type->to_bool(core, &global, result));

    PINTA_GC_RETURN(core);
}

PintaException pinta_lib_domain_global_to_zero(PintaCore *core, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    PintaReference global;
    PintaType *type;

    pinta_assert(core != NULL);
    pinta_assert(result != NULL);

    PINTA_GC_ENTER(core, global);

    PINTA_CHECK(pinta_lib_domain_global_get_value(core, result, &global));
    type = pinta_core_get_type(core, &global);

    result->reference = global.reference;
    PINTA_CHECK(type->to_zero(core, result));

    PINTA_GC_RETURN(core);
}

PintaException pinta_lib_domain_global_get_member(PintaCore *core, PintaReference *value, PintaReference *name, u8 *is_accessor, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    PintaReference global;
    PintaType *type;

    pinta_assert(core != NULL);
    pinta_assert(value != NULL);
    pinta_assert(result != NULL);

    PINTA_GC_ENTER(core, global);

    PINTA_CHECK(pinta_lib_domain_global_get_value(core, value, &global));
    type = pinta_core_get_type(core, &global);

    PINTA_CHECK(type->get_member(core, &global, name, is_accessor, result));

    PINTA_GC_RETURN(core);
}

PintaException pinta_lib_domain_global_set_member(PintaCore *core, PintaReference *object, PintaReference *name, PintaReference *value, u8 *is_accessor, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    PintaReference global;
    PintaType *type;

    pinta_assert(core != NULL);
    pinta_assert(object != NULL);
    pinta_assert(result != NULL);

    PINTA_GC_ENTER(core, global);

    PINTA_CHECK(pinta_lib_domain_global_get_value(core, object, &global));
    type = pinta_core_get_type(core, &global);

    PINTA_CHECK(type->set_member(core, &global, name, value, is_accessor, result));

    PINTA_GC_RETURN(core);
}

PintaException pinta_lib_domain_global_get_item(PintaCore *core, PintaReference *array, u32 index, PintaReference *value)
{
    PintaException exception = PINTA_OK;
    PintaReference global;
    PintaType *type;

    pinta_assert(core != NULL);
    pinta_assert(array != NULL);
    pinta_assert(value != NULL);

    PINTA_GC_ENTER(core, global);

    PINTA_CHECK(pinta_lib_domain_global_get_value(core, array, &global));
    type = pinta_core_get_type(core, &global);

    PINTA_CHECK(type->get_item(core, &global, index, value));

    PINTA_GC_RETURN(core);
}

PintaException pinta_lib_domain_global_set_item(PintaCore *core, PintaReference *array, u32 index, PintaReference *value)
{
    PintaException exception = PINTA_OK;
    PintaReference global;
    PintaType *type;

    pinta_assert(core != NULL);
    pinta_assert(array != NULL);
    pinta_assert(value != NULL);

    PINTA_GC_ENTER(core, global);

    PINTA_CHECK(pinta_lib_domain_global_get_value(core, array, &global));
    type = pinta_core_get_type(core, &global);

    PINTA_CHECK(type->set_item(core, &global, index, value));

    PINTA_GC_RETURN(core);
}

PintaException pinta_lib_domain_global_get_length(PintaCore *core, PintaReference *array, u32 *result)
{
    PintaException exception = PINTA_OK;
    PintaReference global;
    PintaType *type;

    pinta_assert(core != NULL);
    pinta_assert(array != NULL);

    PINTA_GC_ENTER(core, global);

    PINTA_CHECK(pinta_lib_domain_global_get_value(core, array, &global));
    type = pinta_core_get_type(core, &global);

    PINTA_CHECK(type->get_length(core, &global, result));

    PINTA_GC_RETURN(core);
}

PintaException pinta_lib_domain_global_get_char(PintaCore *core, PintaReference *array, u32 index, wchar *result)
{
    PintaException exception = PINTA_OK;
    PintaReference global;
    PintaType *type;

    pinta_assert(core != NULL);
    pinta_assert(array != NULL);

    PINTA_GC_ENTER(core, global);

    PINTA_CHECK(pinta_lib_domain_global_get_value(core, array, &global));
    type = pinta_core_get_type(core, &global);

    PINTA_CHECK(type->get_char(core, &global, index, result));

    PINTA_GC_RETURN(core);
}

PintaException pinta_lib_domain_global_debug_write(PintaCore *core, PintaReference *value, u32 max_depth, PintaJsonWriter *writer)
{
    PintaException exception = PINTA_OK;
    PintaReference global;
    PintaType *type;

    pinta_assert(core != NULL);
    pinta_assert(value != NULL);
    pinta_assert(writer != NULL);

    PINTA_GC_ENTER(core, global);

    PINTA_CHECK(pinta_lib_domain_global_get_value(core, value, &global));
    type = pinta_core_get_type(core, &global);

    PINTA_CHECK(type->debug_write(core, &global, max_depth, writer));

    PINTA_GC_RETURN(core);
}
