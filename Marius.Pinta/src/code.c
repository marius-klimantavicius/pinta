#include "pinta.h"

#if PINTA_DEBUG
void pinta_debug_assert_code(PintaThread *thread)
{
    u8 found = 0;
    PintaModuleDomain *current_domain;
    PintaThread *current_thread;

    pinta_assert(thread != NULL);
    pinta_assert(thread->core != NULL);
    pinta_assert(thread->core->heap != NULL);
    pinta_assert(thread->core->threads != NULL);

    pinta_assert(thread->frame->is_final_frame || thread->domain != NULL);
    pinta_assert(thread->domain == NULL || thread->domain->module != NULL);

    if (thread->core->domains && thread->domain)
    {
        for (current_domain = thread->core->domains; current_domain; current_domain = current_domain->next)
        {
            if (current_domain == thread->domain)
            {
                found = 1;
                break;
            }
        }

        pinta_assert(found && "Core contains current module domain");
    }

    found = 0;
    for (current_thread = thread->core->threads; current_thread; current_thread = current_thread->next)
    {
        if (current_thread == thread)
        {
            found = 1;
            break;
        }
    }

    pinta_assert(found && "Core contains current thread");

    pinta_assert(thread->frame != NULL);
    pinta_assert(thread->frame->is_final_frame || thread->code_pointer != NULL);

    pinta_assert(thread->frame->is_final_frame || thread->code_pointer >= thread->frame->code_start);
    pinta_assert(thread->frame->is_final_frame || thread->code_pointer < thread->frame->code_end);
}
#endif /* PINTA_DEBUG */

/* helper/internal function */

PintaException pinta_code_to_string(PintaThread *thread, PintaReference *value, PintaReference *result)
{
    PintaCore *core;
    PintaType *type;

    pinta_assert(thread != NULL);
    pinta_assert(value != NULL);
    pinta_assert(result != NULL);

    core = thread->core;
    type = pinta_core_get_type(core, value);
    return type->to_string(core, value, result);
}

PintaException pinta_code_string_concat(PintaThread *thread, PintaReference *left, PintaReference *right, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    PintaCore *core;

    pinta_assert(left != NULL);
    pinta_assert(right != NULL);
    pinta_assert(result != NULL);
    pinta_assert(left != right);      // because this is internal and expected to be called with different references
    pinta_assert(left != result);
    pinta_assert(right != result);

    core = thread->core;

    PINTA_CHECK(pinta_code_to_string(thread, left, left));
    PINTA_CHECK(pinta_code_to_string(thread, right, right));

    if (left->reference == NULL)
    {
        result->reference = right->reference;
        PINTA_RETURN();
    }

    if (right->reference == NULL)
    {
        result->reference = left->reference;
        PINTA_RETURN();
    }

    if ((left->reference->block_kind == PINTA_KIND_STRING || left->reference->block_kind == PINTA_KIND_CHAR)
        && (right->reference->block_kind == PINTA_KIND_STRING || right->reference->block_kind == PINTA_KIND_CHAR))
    {
        wchar *left_data, *right_data, *string_data;
        u32 left_length, right_length;
        wchar left_value, right_value;

        left_data = NULL;
        right_data = NULL;

        if (left->reference->block_kind == PINTA_KIND_STRING)
        {
            left_length = pinta_string_ref_get_length(left);
            if (left_length == 0)
            {
                result->reference = right->reference;
                PINTA_RETURN();
            }
        }
        else
        {
            left_value = pinta_char_ref_get_value(left);
            left_length = pinta_char_ref_get_length(left);
            left_data = &left_value;
        }

        if (right->reference->block_kind == PINTA_KIND_STRING)
        {
            right_length = pinta_string_ref_get_length(right);
            if (right_length == 0)
            {
                result->reference = left->reference;
                PINTA_RETURN();
            }
        }
        else
        {
            right_value = pinta_char_ref_get_value(right);
            right_length = pinta_char_ref_get_length(right);
            right_data = &right_value;
        }

        PINTA_CHECK(pinta_lib_string_alloc(core, left_length + right_length, result));

        string_data = pinta_string_get_data(result->reference);
        if (!left_data)
            left_data = pinta_string_ref_get_data(left);
        if (!right_data)
            right_data = pinta_string_ref_get_data(right);

        memmove(string_data, left_data, sizeof(wchar) * left_length);
        memmove(string_data + left_length, right_data, sizeof(wchar) * right_length);
    }
    else
    {
        PINTA_CHECK(pinta_lib_multistring_alloc_value(core, left, left));
        PINTA_CHECK(pinta_lib_multistring_append(core, left, right, result));
    }

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_code_common_arithmetic(PintaThread *thread, u8 code, PintaReference *left, PintaReference *right, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    PintaCore *core;
    PintaType *left_type;
    PintaType *right_type;

    pinta_assert(left != NULL);
    pinta_assert(right != NULL);
    pinta_assert(result != NULL);
    pinta_assert(left != right);      // because this is internal and expected to be called with different references
    pinta_assert(left != result);
    pinta_assert(right != result);

    if (left->reference == NULL && right->reference == NULL)
    {
        PINTA_CHECK(pinta_lib_stack_push_null(thread));
        PINTA_RETURN();
    }

    core = thread->core;

    left_type = pinta_core_get_type(core, left);
    right_type = pinta_core_get_type(core, right);

    if (left->reference == NULL)
        PINTA_CHECK(right_type->to_zero(core, left));

    if (right->reference == NULL)
        PINTA_CHECK(left_type->to_zero(core, right));

    PINTA_CHECK(left_type->to_numeric(core, left, left));
    PINTA_CHECK(right_type->to_numeric(core, right, right));

    if (code == PINTA_CODE_REMAINDER || (left->reference->block_kind == PINTA_KIND_INTEGER && right->reference->block_kind == PINTA_KIND_INTEGER))
    {
        if (left->reference->block_kind == PINTA_KIND_DECIMAL)
            PINTA_CHECK(pinta_lib_decimal_to_int32(core, left, left));

        if (right->reference->block_kind == PINTA_KIND_DECIMAL)
            PINTA_CHECK(pinta_lib_decimal_to_int32(core, right, right));

        if (left->reference->block_kind != PINTA_KIND_INTEGER || right->reference->block_kind != PINTA_KIND_INTEGER)
            PINTA_THROW(PINTA_EXCEPTION_NOT_REACHABLE);

        switch (code)
        {
        case PINTA_CODE_ADD:
            PINTA_CHECK(pinta_lib_integer_alloc_value(core, pinta_integer_ref_get_value(left) + pinta_integer_ref_get_value(right), result));
            break;
        case PINTA_CODE_SUBTRACT:
            PINTA_CHECK(pinta_lib_integer_alloc_value(core, pinta_integer_ref_get_value(left) - pinta_integer_ref_get_value(right), result));
            break;
        case PINTA_CODE_MULTIPLY:
            PINTA_CHECK(pinta_lib_integer_alloc_value(core, pinta_integer_ref_get_value(left) * pinta_integer_ref_get_value(right), result));
            break;
        case PINTA_CODE_DIVIDE:
            if (pinta_integer_ref_get_value(right) == 0)
                PINTA_THROW(PINTA_EXCEPTION_DIVISION_BY_ZERO);
            PINTA_CHECK(pinta_lib_integer_alloc_value(core, pinta_integer_ref_get_value(left) / pinta_integer_ref_get_value(right), result));
            break;
        case PINTA_CODE_REMAINDER:
            if (pinta_integer_ref_get_value(right) == 0)
                PINTA_THROW(PINTA_EXCEPTION_DIVISION_BY_ZERO);
            PINTA_CHECK(pinta_lib_integer_alloc_value(core, pinta_integer_ref_get_value(left) % pinta_integer_ref_get_value(right), result));
            break;
        default:
            PINTA_THROW(PINTA_EXCEPTION_NOT_REACHABLE);
            break;
        }
    }
    else
    {
        if (left->reference->block_kind == PINTA_KIND_INTEGER)
            PINTA_CHECK(pinta_lib_decimal_from_int32(core, left, left));

        if (right->reference->block_kind == PINTA_KIND_INTEGER)
            PINTA_CHECK(pinta_lib_decimal_from_int32(core, right, right));

        if (left->reference->block_kind != PINTA_KIND_DECIMAL || right->reference->block_kind != PINTA_KIND_DECIMAL)
            PINTA_THROW(PINTA_EXCEPTION_NOT_REACHABLE);

        switch (code)
        {
        case PINTA_CODE_ADD:
            PINTA_CHECK(pinta_lib_decimal_add(core, left, right, result));
            break;
        case PINTA_CODE_SUBTRACT:
            PINTA_CHECK(pinta_lib_decimal_subtract(core, left, right, result));
            break;
        case PINTA_CODE_MULTIPLY:
            PINTA_CHECK(pinta_lib_decimal_multiply(core, left, right, result));
            break;
        case PINTA_CODE_DIVIDE:
            PINTA_CHECK(pinta_lib_decimal_divide(core, left, right, result));
            break;
        default:
            PINTA_THROW(PINTA_EXCEPTION_NOT_REACHABLE);
            break;
        }
    }

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_code_to_bool(PintaThread *thread, PintaReference *value, u8 *result)
{
    PintaCore *core;
    PintaType *type;

    pinta_assert(thread != NULL);
    pinta_assert(value != NULL);
    pinta_assert(result != NULL);

    core = thread->core;

    type = pinta_core_get_type(core, value);
    return type->to_bool(core, value, result);
}

PintaException pinta_code_next(PintaThread *thread, u8 *result)
{
    PintaException exception = PINTA_OK;
    PintaStackFrame *frame;
    u8 *code;

    pinta_debug_assert_code(thread);
    pinta_assert(result != NULL);

    code = thread->code_next_pointer;
    if (code == NULL)
        PINTA_THROW(PINTA_EXCEPTION_INVALID_OPERATION);

    frame = thread->frame;
    if (frame == NULL)
        PINTA_THROW(PINTA_EXCEPTION_INVALID_OPERATION);

    if (code < frame->code_start || code >= frame->code_end)
        PINTA_THROW(PINTA_EXCEPTION_INVALID_MODULE);

    *result = *code;
    thread->code_next_pointer = &code[1];

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_code_integer(PintaThread *thread, i32 *result)
{
    PintaException exception = PINTA_OK;
    PintaStackFrame *frame;
    u8 *code;
    i32 value = 0;
    PintaReader reader;

    pinta_debug_assert_code(thread);
    pinta_assert(result != NULL);

    code = thread->code_next_pointer;
    if (code == NULL)
        PINTA_THROW(PINTA_EXCEPTION_INVALID_OPERATION);

    frame = thread->frame;
    if (frame == NULL)
        PINTA_THROW(PINTA_EXCEPTION_INVALID_OPERATION);

    reader.start = frame->code_start;
    reader.end = frame->code_end;
    reader.current = code;

    PINTA_CHECK(pinta_binary_read_sleb128(&reader, NULL, &value));

    *result = value;
    thread->code_next_pointer = reader.current;

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_code_token(PintaThread *thread, u32 *result)
{
    PintaException exception = PINTA_OK;
    PintaStackFrame *frame;
    u8 *code;
    u32 value = 0;
    PintaReader reader;

    pinta_debug_assert_code(thread);
    pinta_assert(result != NULL);

    code = thread->code_next_pointer;
    if (code == NULL)
        PINTA_THROW(PINTA_EXCEPTION_INVALID_OPERATION);

    frame = thread->frame;
    if (frame == NULL)
        PINTA_THROW(PINTA_EXCEPTION_INVALID_OPERATION);

    reader.start = frame->code_start;
    reader.end = frame->code_end;
    reader.current = code;

    PINTA_CHECK(pinta_binary_read_uleb128(&reader, NULL, &value));

    *result = value;
    thread->code_next_pointer = reader.current;

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_code_get_string(PintaThread *thread, u32 token, PintaReference *result)
{
    PintaCore *core;
    PintaModuleDomain *domain;

    pinta_assert(token != PINTA_CODE_TOKEN_EMPTY);
    pinta_debug_assert_code(thread);

    core = thread->core;
    domain = thread->domain;

    return pinta_lib_module_get_string(core, domain, token, result);
}

PintaException pinta_code_global_accessor(PintaCore *core, PintaReference *object, PintaReference *name, u32 token, u8 is_set, PintaReference *value)
{
    PintaException exception = PINTA_OK;
    PintaThread *thread;
    PintaModuleDomain *domain;

    thread = core->threads;

    pinta_assert(token != PINTA_CODE_TOKEN_EMPTY);
    pinta_debug_assert_code(thread);

    PINTA_UNUSED(object);
    PINTA_UNUSED(name);

    domain = thread->domain;

    if (domain->globals.reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_INVALID_OPERATION);

    if (is_set)
        PINTA_CHECK(pinta_lib_array_set_item(core, &domain->globals, token, value));
    else
        PINTA_CHECK(pinta_lib_array_get_item(core, &domain->globals, token, value));

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_code_get_global(PintaThread *thread, PintaReference **result)
{
    PintaException exception = PINTA_OK;
    PintaCore *core;
    PintaModuleDomain *domain;

    core = thread->core;

    domain = thread->domain;
    if (domain->global_object.reference == NULL)
        PINTA_CHECK(pinta_lib_global_object_alloc(core, domain, &domain->global_object));

    *result = &domain->global_object;

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

/* code functions */

PintaException pinta_code_nop(PintaThread *thread)
{
    pinta_debug_assert_code(thread);
    return PINTA_OK;
}

PintaException pinta_code_load_null(PintaThread *thread)
{
    pinta_debug_assert_code(thread);

    return pinta_lib_stack_push_null(thread);
}

PintaException pinta_code_load_integer(PintaThread *thread, i32 value)
{
    PintaException exception = PINTA_OK;
    PintaCore *core;
    PintaHeapObject *result;
    pinta_debug_assert_code(thread);

    core = thread->core;

    result = pinta_integer_alloc_object_value(core, value);
    if (result == NULL)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_MEMORY);
    PINTA_CHECK(pinta_stack_push(thread, result));

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_code_load_integer_zero(PintaThread *thread)
{
    return pinta_code_load_integer(thread, 0);
}

PintaException pinta_code_load_decimal_zero(PintaThread *thread)
{
    PintaException exception = PINTA_OK;
    PintaCore *core;
    PintaHeapObject *value;

    pinta_debug_assert_code(thread);

    core = thread->core;

    value = pinta_decimal_alloc_object(core);
    if (value == NULL)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_MEMORY);
    pinta_decimal_set_zero(value);

    PINTA_CHECK(pinta_stack_push(thread, value));

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_code_load_integer_one(PintaThread *thread)
{
    return pinta_code_load_integer(thread, 1);
}

PintaException pinta_code_load_decimal_one(PintaThread *thread)
{
    PintaException exception = PINTA_OK;
    PintaCore *core;
    decimal decimal_value;
    PintaHeapObject *value;

    pinta_debug_assert_code(thread);

    core = thread->core;

    value = pinta_decimal_alloc_object(core);
    if (value == NULL)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_MEMORY);

    decimal_value = decimal_from32(1);
    pinta_decimal_set_value(value, decimal_value);

    PINTA_CHECK(pinta_stack_push(thread, value));

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_code_load_string(PintaThread *thread, u32 token)
{
    PintaException exception = PINTA_OK;
    PintaCore *core;
    PintaReference result;

    pinta_assert(token != PINTA_CODE_TOKEN_EMPTY);
    pinta_debug_assert_code(thread);

    core = thread->core;
    PINTA_GC_ENTER(core, result);

    PINTA_CHECK(pinta_code_get_string(thread, token, &result));
    PINTA_CHECK(pinta_lib_stack_push(thread, &result));

    PINTA_GC_RETURN(core);
}

PintaException pinta_code_add(PintaThread *thread)
{
    PintaException exception = PINTA_OK;
    PintaCore *core;
    u32 is_left_string = 0;
    u32 is_right_string = 0;
    PintaType *left_type;
    PintaType *right_type;
    struct
    {
        PintaReference left;
        PintaReference right;
        PintaReference result;
    } gc;

    pinta_debug_assert_code(thread);

    core = thread->core;
    PINTA_GC_ENTER(core, gc);

    PINTA_CHECK(pinta_lib_stack_pop(thread, &gc.right));
    PINTA_CHECK(pinta_lib_stack_pop(thread, &gc.left));

    if (gc.left.reference == NULL && gc.right.reference == NULL)
    {
        PINTA_CHECK(pinta_lib_stack_push_null(thread));
        PINTA_RETURN();
    }

    left_type = pinta_core_get_type(core, &gc.left);
    right_type = pinta_core_get_type(core, &gc.right);

    /*
    Add - if any strings then concat, otherwise normal arithmetic
    */
    is_left_string = left_type->is_string;
    is_right_string = right_type->is_string;

    if (is_left_string || is_right_string)
        PINTA_CHECK(pinta_code_string_concat(thread, &gc.left, &gc.right, &gc.result));
    else
        PINTA_CHECK(pinta_code_common_arithmetic(thread, PINTA_CODE_ADD, &gc.left, &gc.right, &gc.result));

    PINTA_CHECK(pinta_lib_stack_push(thread, &gc.result));

    PINTA_GC_RETURN(core);
}

PintaException pinta_code_arithmetic(PintaThread *thread, u8 code)
{
    PintaException exception = PINTA_OK;
    PintaCore *core;
    struct
    {
        PintaReference left;
        PintaReference right;
        PintaReference result;
    } gc;

    pinta_debug_assert_code(thread);

    pinta_assert(code == PINTA_CODE_ADD
        || code == PINTA_CODE_SUBTRACT
        || code == PINTA_CODE_MULTIPLY
        || code == PINTA_CODE_DIVIDE
        || code == PINTA_CODE_REMAINDER);

    core = thread->core;
    PINTA_GC_ENTER(core, gc);

    PINTA_CHECK(pinta_lib_stack_pop(thread, &gc.right));
    PINTA_CHECK(pinta_lib_stack_pop(thread, &gc.left));

    PINTA_CHECK(pinta_code_common_arithmetic(thread, code, &gc.left, &gc.right, &gc.result));
    PINTA_CHECK(pinta_lib_stack_push(thread, &gc.result));

    PINTA_GC_RETURN(core);
}

PintaException pinta_code_exclusive_or(PintaThread *thread)
{
    PintaException exception = PINTA_OK;
    PintaCore *core;
    u8 left_value = 0;
    u8 right_value = 0;
    u8 result_value = 0;
    struct
    {
        PintaReference left;
        PintaReference right;
        PintaReference result;
    } gc;

    pinta_debug_assert_code(thread);

    core = thread->core;
    PINTA_GC_ENTER(core, gc);

    PINTA_CHECK(pinta_lib_stack_pop(thread, &gc.right));
    PINTA_CHECK(pinta_lib_stack_pop(thread, &gc.left));

    if (gc.left.reference == NULL && gc.right.reference == NULL)
    {
        PINTA_CHECK(pinta_lib_stack_push_null(thread));
        PINTA_RETURN();
    }

    if (gc.left.reference == NULL)
    {
        PINTA_CHECK(pinta_lib_stack_push(thread, &gc.right));
        PINTA_RETURN();
    }

    if (gc.right.reference == NULL)
    {
        PINTA_CHECK(pinta_lib_stack_push(thread, &gc.left));
        PINTA_RETURN();
    }

    PINTA_CHECK(pinta_code_to_bool(thread, &gc.left, &left_value));
    PINTA_CHECK(pinta_code_to_bool(thread, &gc.right, &right_value));

    if (left_value ^ right_value)
        result_value = 1;

    PINTA_CHECK(pinta_lib_integer_alloc_value(core, result_value, &gc.result));
    PINTA_CHECK(pinta_lib_stack_push(thread, &gc.result));

    PINTA_GC_RETURN(core);
}

PintaException pinta_code_bitwise(PintaThread *thread, u8 code)
{
    PintaException exception = PINTA_OK;
    PintaCore *core;
    i32 left_value = 0;
    i32 right_value = 0;
    i32 result_value = 0;
    PintaType *left_type;
    PintaType *right_type;
    struct
    {
        PintaReference left;
        PintaReference right;
        PintaReference result;
    } gc;

    pinta_debug_assert_code(thread);

    pinta_assert(code == PINTA_CODE_BITWISE_AND
        || code == PINTA_CODE_BITWISE_OR
        || code == PINTA_CODE_BITWISE_EXCLUSIVE_OR);

    core = thread->core;
    PINTA_GC_ENTER(core, gc);

    PINTA_CHECK(pinta_lib_stack_pop(thread, &gc.right));
    PINTA_CHECK(pinta_lib_stack_pop(thread, &gc.left));

    if (gc.left.reference == NULL && gc.right.reference == NULL)
    {
        PINTA_CHECK(pinta_lib_stack_push_null(thread));
        PINTA_RETURN();
    }

    left_type = pinta_core_get_type(core, &gc.left);
    right_type = pinta_core_get_type(core, &gc.right);

    PINTA_CHECK(left_type->to_integer_value(core, &gc.left, &left_value));
    PINTA_CHECK(right_type->to_integer_value(core, &gc.right, &right_value));

    switch (code)
    {
    case PINTA_CODE_BITWISE_AND:
        result_value = left_value & right_value;
        break;
    case PINTA_CODE_BITWISE_OR:
        result_value = left_value | right_value;
        break;
    case PINTA_CODE_BITWISE_EXCLUSIVE_OR:
        result_value = left_value ^ right_value;
        break;
    default:
        PINTA_THROW(PINTA_EXCEPTION_NOT_REACHABLE);
    }

    PINTA_CHECK(pinta_lib_integer_alloc_value(core, result_value, &gc.result));
    PINTA_CHECK(pinta_lib_stack_push(thread, &gc.result));

    PINTA_GC_RETURN(core);
}

PintaException pinta_code_unary(PintaThread *thread, u8 code)
{
    PintaException exception = PINTA_OK;
    PintaCore *core;
    i32 compare_result = 0;
    i32 int_value;
    u8 bool_value = 0;
    PintaType *value_type;
    struct
    {
        PintaReference value;
        PintaReference result;
    } gc;

    pinta_debug_assert_code(thread);

    pinta_assert((code == PINTA_CODE_NOT)
        || (code == PINTA_CODE_BITWISE_NOT)
        || (code == PINTA_CODE_NEGATE)
        || (code == PINTA_CODE_COMPARE_NULL)
        || (code == PINTA_CODE_CONVERT_INTEGER)
        || (code == PINTA_CODE_CONVERT_DECIMAL)
        || (code == PINTA_CODE_CONVERT_STRING)
    );

    core = thread->core;
    PINTA_GC_ENTER(core, gc);

    PINTA_CHECK(pinta_lib_stack_pop(thread, &gc.value));

    value_type = pinta_core_get_type(core, &gc.value);

    switch (code)
    {
    case PINTA_CODE_NOT:

        PINTA_CHECK(pinta_code_to_bool(thread, &gc.value, &bool_value));

        if (bool_value)
            PINTA_CHECK(pinta_lib_integer_alloc_value(core, 0, &gc.result));
        else
            PINTA_CHECK(pinta_lib_integer_alloc_value(core, 1, &gc.result));

        PINTA_CHECK(pinta_lib_stack_push(thread, &gc.result));
        break;

    case PINTA_CODE_BITWISE_NOT:

        PINTA_CHECK(value_type->to_integer_value(core, &gc.value, &int_value));
        PINTA_CHECK(pinta_lib_integer_alloc_value(core, ~int_value, &gc.result));
        PINTA_CHECK(pinta_lib_stack_push(thread, &gc.result));
        break;

    case PINTA_CODE_NEGATE:

        PINTA_CHECK(value_type->to_numeric(core, &gc.value, &gc.value));

        if (gc.value.reference->block_kind == PINTA_KIND_INTEGER)
            PINTA_CHECK(pinta_lib_integer_alloc_value(core, -pinta_integer_ref_get_value(&gc.value), &gc.result));
        else if (gc.value.reference->block_kind == PINTA_KIND_DECIMAL)
            PINTA_CHECK(pinta_lib_decimal_negate(core, &gc.value, &gc.result));
        else
            PINTA_THROW(PINTA_EXCEPTION_ENGINE);

        PINTA_CHECK(pinta_lib_stack_push(thread, &gc.result));
        break;

    case PINTA_CODE_COMPARE_NULL:

        if (gc.value.reference == NULL)
            compare_result = 1;
        else
            compare_result = 0;

        PINTA_CHECK(pinta_lib_integer_alloc_value(core, compare_result, &gc.result));
        PINTA_CHECK(pinta_lib_stack_push(thread, &gc.result));
        break;

    case PINTA_CODE_CONVERT_INTEGER:

        PINTA_CHECK(value_type->to_integer(core, &gc.value, &gc.result));
        PINTA_CHECK(pinta_lib_stack_push(thread, &gc.result));
        break;

    case PINTA_CODE_CONVERT_DECIMAL:

        PINTA_CHECK(value_type->to_decimal(core, &gc.value, &gc.result));
        PINTA_CHECK(pinta_lib_stack_push(thread, &gc.result));
        break;

    case PINTA_CODE_CONVERT_STRING:

        PINTA_CHECK(value_type->to_string(core, &gc.value, &gc.result));
        PINTA_CHECK(pinta_lib_stack_push(thread, &gc.result));
        break;
    }

    PINTA_GC_RETURN(core);
}

PintaException pinta_code_compare(PintaThread *thread, u8 code)
{
    PintaException exception = PINTA_OK;
    PintaCore *core;
    i32 compare_result = 0;
    u32 is_left_string = 0;
    u32 is_right_string = 0;
    PintaType *left_type;
    PintaType *right_type;
    struct
    {
        PintaReference left;
        PintaReference right;
        PintaReference result;
    } gc;

    pinta_debug_assert_code(thread);

    pinta_assert((code == PINTA_CODE_COMPARE_EQUAL)
        || (code == PINTA_CODE_COMPARE_LESS_THAN)
        || (code == PINTA_CODE_COMPARE_MORE_THAN));

    core = thread->core;
    PINTA_GC_ENTER(core, gc);

    PINTA_CHECK(pinta_lib_stack_pop(thread, &gc.right));
    PINTA_CHECK(pinta_lib_stack_pop(thread, &gc.left));

    if (gc.right.reference == NULL && gc.left.reference == NULL)
    {
        if (code == PINTA_CODE_COMPARE_EQUAL)
            compare_result = 1;
        else
            compare_result = 0;

        PINTA_CHECK(pinta_lib_integer_alloc_value(core, compare_result, &gc.result));
        PINTA_CHECK(pinta_lib_stack_push(thread, &gc.result));
        PINTA_RETURN();
    }

    if (gc.right.reference == NULL || gc.left.reference == NULL)
    {
        if (code == PINTA_CODE_COMPARE_EQUAL)
        {
            compare_result = 0;
        }
        else if (code == PINTA_CODE_COMPARE_LESS_THAN)
        {
            if (gc.left.reference == NULL)
                compare_result = 1;
            else
                compare_result = 0;
        }
        else if (code == PINTA_CODE_COMPARE_MORE_THAN)
        {
            if (gc.right.reference == NULL)
                compare_result = 1;
            else
                compare_result = 0;
        }
        else
        {
            PINTA_THROW(PINTA_EXCEPTION_NOT_REACHABLE);
        }

        PINTA_CHECK(pinta_lib_integer_alloc_value(core, compare_result, &gc.result));
        PINTA_CHECK(pinta_lib_stack_push(thread, &gc.result));
        PINTA_RETURN();
    }

    if (gc.left.reference == gc.right.reference)
    {
        if (code == PINTA_CODE_COMPARE_EQUAL)
            compare_result = 1;
        else
            compare_result = 0;

        PINTA_CHECK(pinta_lib_integer_alloc_value(core, compare_result, &gc.result));
        PINTA_CHECK(pinta_lib_stack_push(thread, &gc.result));
        PINTA_RETURN();
    }

    left_type = pinta_core_get_type(core, &gc.left);
    right_type = pinta_core_get_type(core, &gc.right);

    is_left_string = left_type->is_string;
    is_right_string = right_type->is_string;

    if (gc.left.reference->block_kind == PINTA_KIND_ARRAY || gc.right.reference->block_kind == PINTA_KIND_ARRAY || gc.left.reference->block_kind == PINTA_KIND_WEAK || gc.right.reference->block_kind == PINTA_KIND_WEAK)
    {
        // we have already checked for reference equality
        // array/weak is not equal to anything other than itself
        compare_result = 0;
    }
    else if (is_left_string && is_right_string)
    {
        u32 left_length = 0;
        u32 right_length = 0;
        PINTA_CHECK(left_type->get_length(core, &gc.left, &left_length));
        PINTA_CHECK(right_type->get_length(core, &gc.right, &right_length));

        if (gc.left.reference->block_kind == PINTA_KIND_CHAR || gc.right.reference->block_kind == PINTA_KIND_CHAR)
        {
            wchar left_value = PINTA_CHAR('\0');
            wchar right_value = PINTA_CHAR('\0');

            if (left_length > 0)
                PINTA_CHECK(left_type->get_char(core, &gc.left, 0, &left_value));

            if (right_length > 0)
                PINTA_CHECK(right_type->get_char(core, &gc.right, 0, &right_value));

            if (left_length == right_length || left_value != right_value)
            {
                compare_result = (i32)left_value - (i32)right_value;
            }
            else
            {
                if (left_length < right_length)
                    compare_result = -1;
                else
                    compare_result = 1;
            }
        }
        else
        {
            PINTA_CHECK(pinta_lib_string_to_string(core, &gc.left, &gc.left));
            PINTA_CHECK(pinta_lib_string_to_string(core, &gc.right, &gc.right));

            if (left_length < right_length)
            {
                compare_result = pinta_string_compare(pinta_string_ref_get_data(&gc.left), pinta_string_ref_get_data(&gc.right), left_length);
                if (compare_result == 0)
                    compare_result = -1;
            }
            else if (left_length > right_length)
            {
                compare_result = pinta_string_compare(pinta_string_ref_get_data(&gc.left), pinta_string_ref_get_data(&gc.right), right_length);
                if (compare_result == 0)
                    compare_result = 1;
            }
            else
            {
                compare_result = pinta_string_compare(pinta_string_ref_get_data(&gc.left), pinta_string_ref_get_data(&gc.right), left_length);
            }
        }

        switch (code)
        {
        case PINTA_CODE_COMPARE_EQUAL:
            if (compare_result == 0)
                compare_result = 1;
            else
                compare_result = 0;
            break;
        case PINTA_CODE_COMPARE_LESS_THAN:
            if (compare_result < 0)
                compare_result = 1;
            else
                compare_result = 0;
            break;
        case PINTA_CODE_COMPARE_MORE_THAN:
            if (compare_result > 0)
                compare_result = 1;
            else
                compare_result = 0;
            break;
        default:
            PINTA_THROW(PINTA_EXCEPTION_NOT_REACHABLE);
            break;
        }

        PINTA_CHECK(pinta_lib_integer_alloc_value(core, compare_result, &gc.result));
        PINTA_CHECK(pinta_lib_stack_push(thread, &gc.result));
        PINTA_RETURN();
    }
    else if (gc.left.reference->block_kind == PINTA_KIND_INTEGER && gc.right.reference->block_kind == PINTA_KIND_INTEGER)
    {
        i32 left_value = pinta_integer_ref_get_value(&gc.left);
        i32 right_value = pinta_integer_ref_get_value(&gc.right);

        switch (code)
        {
        case PINTA_CODE_COMPARE_EQUAL:
            if (left_value == right_value)
                compare_result = 1;
            else
                compare_result = 0;
            break;
        case PINTA_CODE_COMPARE_LESS_THAN:
            if (left_value < right_value)
                compare_result = 1;
            else
                compare_result = 0;
            break;
        case PINTA_CODE_COMPARE_MORE_THAN:
            if (left_value > right_value)
                compare_result = 1;
            else
                compare_result = 0;
            break;
        default:
            PINTA_THROW(PINTA_EXCEPTION_NOT_REACHABLE);
            break;
        }
    }
    else
    {
        PINTA_CHECK(left_type->to_decimal(core, &gc.left, &gc.left));
        PINTA_CHECK(right_type->to_decimal(core, &gc.right, &gc.right));

        PINTA_CHECK(pinta_lib_decimal_compare(core, &gc.left, &gc.right, &gc.result));

        switch (code)
        {
        case PINTA_CODE_COMPARE_EQUAL:
            if (decimal_is_zero(pinta_decimal_ref_get_value(&gc.result)) == 1)
                compare_result = 1;
            else
                compare_result = 0;
        case PINTA_CODE_COMPARE_LESS_THAN:
            if (decimal_is_negative(pinta_decimal_ref_get_value(&gc.result)) == 1)
                compare_result = 1;
            else
                compare_result = 0;
            break;
        case PINTA_CODE_COMPARE_MORE_THAN:
            if (decimal_is_positive(pinta_decimal_ref_get_value(&gc.result)) == 1)
                compare_result = 1;
            else
                compare_result = 0;
            break;
        default:
            PINTA_THROW(PINTA_EXCEPTION_NOT_REACHABLE);
            break;
        }
    }

    PINTA_CHECK(pinta_lib_integer_alloc_value(core, compare_result, &gc.result));
    PINTA_CHECK(pinta_lib_stack_push(thread, &gc.result));

    PINTA_GC_RETURN(core);
}

PintaException pinta_code_new_array(PintaThread *thread)
{
    PintaException exception = PINTA_OK;
    PintaCore *core;
    i32 array_length;
    struct
    {
        PintaReference length;
        PintaReference result;
    } gc;

    pinta_debug_assert_code(thread);

    core = thread->core;
    PINTA_GC_ENTER(core, gc);

    PINTA_CHECK(pinta_lib_stack_pop(thread, &gc.length));

    if (gc.length.reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (gc.length.reference->block_kind != PINTA_KIND_INTEGER)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    array_length = pinta_integer_ref_get_value(&gc.length);

    if (array_length < 0)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

    PINTA_CHECK(pinta_lib_array_alloc(core, (u32)array_length, &gc.result));
    PINTA_CHECK(pinta_lib_stack_push(thread, &gc.result));

    PINTA_GC_RETURN(core);
}

PintaException pinta_code_concat(PintaThread *thread)
{
    PintaException exception = PINTA_OK;
    PintaCore *core;
    struct
    {
        PintaReference left;
        PintaReference right;
        PintaReference result;
    } gc;

    pinta_debug_assert_code(thread);

    core = thread->core;
    PINTA_GC_ENTER(core, gc);

    PINTA_CHECK(pinta_lib_stack_pop(thread, &gc.right));
    PINTA_CHECK(pinta_lib_stack_pop(thread, &gc.left));

    if (gc.left.reference == NULL && gc.right.reference == NULL)
    {
        PINTA_CHECK(pinta_lib_stack_push_null(thread));
        PINTA_RETURN();
    }

    PINTA_CHECK(pinta_code_string_concat(thread, &gc.left, &gc.right, &gc.result));
    PINTA_CHECK(pinta_lib_stack_push(thread, &gc.result));

    PINTA_GC_RETURN(core);
}

PintaException pinta_code_substring(PintaThread *thread)
{
    PintaException exception = PINTA_OK;
    PintaCore *core;
    wchar *string_data;

    i32 length_value;
    i32 offset_value;

    struct
    {
        PintaReference offset;
        PintaReference length;
        PintaReference string;
        PintaReference result;
    } gc;

    pinta_debug_assert_code(thread);

    core = thread->core;
    PINTA_GC_ENTER(core, gc);

    PINTA_CHECK(pinta_lib_stack_pop(thread, &gc.string));
    PINTA_CHECK(pinta_lib_stack_pop(thread, &gc.length));
    PINTA_CHECK(pinta_lib_stack_pop(thread, &gc.offset));

    if (gc.string.reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (gc.length.reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (gc.offset.reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (gc.string.reference->block_kind != PINTA_KIND_STRING
        && gc.string.reference->block_kind != PINTA_KIND_SUBSTRING
        && gc.string.reference->block_kind != PINTA_KIND_MULTISTRING
        && gc.string.reference->block_kind != PINTA_KIND_CHAR)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    if (gc.length.reference->block_kind != PINTA_KIND_DECIMAL && gc.length.reference->block_kind != PINTA_KIND_INTEGER)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    if (gc.offset.reference->block_kind != PINTA_KIND_DECIMAL && gc.offset.reference->block_kind != PINTA_KIND_INTEGER)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    if (gc.length.reference->block_kind == PINTA_KIND_DECIMAL)
        PINTA_CHECK(pinta_lib_decimal_to_int32(core, &gc.length, &gc.length));

    if (gc.offset.reference->block_kind == PINTA_KIND_DECIMAL)
        PINTA_CHECK(pinta_lib_decimal_to_int32(core, &gc.offset, &gc.offset));

    length_value = pinta_integer_ref_get_value(&gc.length);
    offset_value = pinta_integer_ref_get_value(&gc.offset);

    if (length_value < 0)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

    if (offset_value < 0)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

    PINTA_CHECK(pinta_lib_string_to_string(core, &gc.string, &gc.string));

    //PINTA_CHECK(pinta_lib_substring_alloc(core, string, (u16)offset_value, (u16)length_value, result));

    string_data = pinta_string_ref_get_data(&gc.string) + offset_value;
    PINTA_CHECK(pinta_lib_string_alloc_copy(core, string_data, (u32)length_value, &gc.result));
    PINTA_CHECK(pinta_lib_stack_push(thread, &gc.result));

    PINTA_GC_RETURN(core);
}

PintaException pinta_code_jump(PintaThread *thread, i32 offset)
{
    PintaException exception = PINTA_OK;
    PintaStackFrame *frame;
    u8 *code_pointer;

    pinta_debug_assert_code(thread);

    frame = thread->frame;
    code_pointer = thread->code_pointer + offset;
    if (code_pointer < frame->code_start || code_pointer > frame->code_end)
        PINTA_THROW(PINTA_EXCEPTION_INVALID_OPERATION);

    thread->code_next_pointer = code_pointer;

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_code_jump_conditional(PintaThread *thread, u8 code, i32 offset)
{
    PintaException exception = PINTA_OK;
    PintaCore *core;
    PintaStackFrame *frame;
    PintaType *value_type;
    u8 *code_pointer;
    u8 isZero = 0;
    PintaReference value;

    pinta_debug_assert_code(thread);

    pinta_assert(code == PINTA_CODE_JUMP_ZERO || code == PINTA_CODE_JUMP_NOT_ZERO);

    frame = thread->frame;
    code_pointer = thread->code_pointer + offset;
    if (code_pointer < frame->code_start || code_pointer > frame->code_end)
        return (PINTA_EXCEPTION_INVALID_MODULE);

    core = thread->core;
    PINTA_GC_ENTER(core, value);

    PINTA_CHECK(pinta_lib_stack_pop(thread, &value));

    value_type = pinta_core_get_type(core, &value);
    PINTA_CHECK(value_type->to_bool(core, &value, &isZero));

    isZero = !isZero;

    if (code == PINTA_CODE_JUMP_ZERO && isZero)
        thread->code_next_pointer = code_pointer;
    else if (code == PINTA_CODE_JUMP_NOT_ZERO && !isZero)
        thread->code_next_pointer = code_pointer;

    PINTA_GC_RETURN(core);
}

PintaException pinta_code_call(PintaThread *thread, u32 token, u32 arguments_count)
{
    PintaException exception = PINTA_OK;
    PintaCore *core;
    u32 locals_length = 0;
    u32 arguments_array_length = arguments_count;
    PintaCodeFunction function_value, *function = &function_value;
    PintaStackFrame *frame;
    PintaReference *stack;
    struct
    {
        PintaReference arguments;
        PintaReference locals;
        PintaReference value;
    } gc;

    pinta_assert(token != PINTA_CODE_TOKEN_EMPTY);
    pinta_assert(arguments_count != PINTA_CODE_TOKEN_EMPTY);
    pinta_debug_assert_code(thread);

    core = thread->core;
    PINTA_GC_ENTER(core, gc);

    pinta_debug_raise_before_call(thread, token, arguments_count);

    PINTA_CHECK(pinta_module_get_function(thread->domain, token, function));
    locals_length = function->locals_count;

    if (function->arguments_count > arguments_array_length && function->arguments_count != PINTA_CODE_TOKEN_EMPTY)
        arguments_array_length = function->arguments_count;

    PINTA_CHECK(pinta_lib_array_alloc(core, arguments_array_length, &gc.arguments));
    PINTA_CHECK(pinta_lib_array_alloc(core, locals_length, &gc.locals));

    PINTA_CHECK(pinta_lib_stack_get_reference(thread, arguments_count, &stack));
    pinta_array_ref_copy(&gc.arguments, 0, stack, arguments_count);
    PINTA_CHECK(pinta_lib_stack_discard(thread, arguments_count));

    PINTA_CHECK(pinta_lib_frame_push(thread));
    frame = thread->frame;

    frame->function_this.reference = NULL;
    frame->function_closure.reference = NULL;
    frame->function_arguments.reference = gc.arguments.reference;
    frame->function_locals.reference = gc.locals.reference;

    frame->code_start = function->code_start;
    frame->code_end = function->code_end;
    frame->return_address = thread->code_next_pointer;

    thread->code_next_pointer = function->code_start;

    pinta_debug_raise_after_call(thread);

    PINTA_GC_RETURN(core);
}

PintaException pinta_code_call_internal(PintaThread *thread, u32 token, u32 arguments_count)
{
    PintaException exception = PINTA_OK;
    PintaCore *core;
    PintaCoreInternalFunction function;
    PintaReference *stack;
    struct
    {
        PintaReference arguments;
        PintaReference value;
        PintaReference return_value;
    } gc;


    pinta_assert(token != PINTA_CODE_TOKEN_EMPTY);
    pinta_assert(arguments_count != PINTA_CODE_TOKEN_EMPTY);
    pinta_debug_assert_code(thread);

    core = thread->core;
    if (core->internal_functions == NULL)
        return (PINTA_EXCEPTION_INVALID_OPERATION);

    if (core->internal_functions_length <= token)
        return (PINTA_EXCEPTION_INVALID_OPERATION);

    PINTA_GC_ENTER(core, gc);

    pinta_debug_raise_before_call_internal(thread, token, arguments_count);

    function = core->internal_functions[token];

    PINTA_CHECK(pinta_lib_array_alloc(core, arguments_count, &gc.arguments));

    PINTA_CHECK(pinta_lib_stack_get_reference(thread, arguments_count, &stack));
    pinta_array_ref_copy(&gc.arguments, 0, stack, arguments_count);
    PINTA_CHECK(pinta_lib_stack_discard(thread, arguments_count));

    gc.value.reference = NULL;
    gc.return_value.reference = NULL;

    PINTA_CHECK(function(core, &gc.arguments, &gc.return_value));

    PINTA_CHECK(pinta_lib_stack_push(thread, &gc.return_value));

    pinta_debug_raise_after_call_internal(thread);

    PINTA_GC_RETURN(core);
}

PintaException pinta_code_return(PintaThread *thread)
{
    PintaException exception = PINTA_OK;
    PintaCore *core;
    u8 *return_address;
    u8 discard_result;
    u8 is_final_frame;
    PintaModuleDomain *return_domain;
    PintaReference value;
    PintaFrameTransform transform_result;

    pinta_debug_assert_code(thread);

    core = thread->core;
    PINTA_GC_ENTER(core, value);

    pinta_debug_raise_before_return(thread);

    return_address = thread->frame->return_address;
    discard_result = thread->frame->discard_result;
    transform_result = thread->frame->transform_result;
    is_final_frame = thread->frame->is_final_frame;
    return_domain = thread->frame->return_domain;

    PINTA_CHECK(pinta_lib_stack_pop(thread, &value));
    if (transform_result)
        PINTA_CHECK(transform_result(thread, &value));

    PINTA_CHECK(pinta_lib_frame_pop(thread));

    if (return_domain != NULL)
        thread->domain = return_domain;

    if (!is_final_frame && (thread->frame == NULL || thread->frame->is_final_frame))
        is_final_frame = 1;

    if (is_final_frame)
    {
        thread->code_result = PINTA_OK;
        thread->code_finished = 1;
        PINTA_RETURN();
    }

    if (!discard_result)
        PINTA_CHECK(pinta_lib_stack_push(thread, &value));

    if (thread->frame == NULL)
        PINTA_THROW(PINTA_EXCEPTION_INVALID_OPERATION);

    thread->code_next_pointer = return_address;

    pinta_debug_raise_after_return(thread);

    PINTA_GC_RETURN(core);
}

PintaException pinta_code_store_local(PintaThread *thread, u32 index)
{
    PintaException exception = PINTA_OK;
    PintaCore *core;
    PintaStackFrame *frame;
    PintaReference value;

    pinta_assert(index != PINTA_CODE_TOKEN_EMPTY);
    pinta_debug_assert_code(thread);

    frame = thread->frame;
    if (frame->function_locals.reference == NULL)
        return (PINTA_EXCEPTION_INVALID_OPERATION);

    core = thread->core;
    PINTA_GC_ENTER(core, value);

    PINTA_CHECK(pinta_lib_stack_pop(thread, &value));
    PINTA_CHECK(pinta_lib_array_set_item(core, &frame->function_locals, index, &value));

    PINTA_GC_RETURN(core);
}

PintaException pinta_code_store_global(PintaThread *thread, u32 index)
{
    PintaException exception = PINTA_OK;
    PintaCore *core;
    PintaModuleDomain *domain;
    PintaReference value;

    pinta_assert(index != PINTA_CODE_TOKEN_EMPTY);
    pinta_debug_assert_code(thread);

    domain = thread->domain;
    if (domain->globals.reference == NULL)
        return (PINTA_EXCEPTION_INVALID_OPERATION);

    core = thread->core;
    PINTA_GC_ENTER(core, value);

    PINTA_CHECK(pinta_lib_stack_pop(thread, &value));
    PINTA_CHECK(pinta_lib_array_set_item(core, &domain->globals, index, &value));

    PINTA_GC_RETURN(core);
}

PintaException pinta_code_store_argument(PintaThread *thread, u32 index)
{
    PintaException exception = PINTA_OK;
    PintaCore *core;
    PintaStackFrame *frame;
    PintaReference value;

    pinta_assert(index != PINTA_CODE_TOKEN_EMPTY);
    pinta_debug_assert_code(thread);

    frame = thread->frame;
    if (frame->function_arguments.reference == NULL)
        return (PINTA_EXCEPTION_INVALID_OPERATION);

    core = thread->core;
    PINTA_GC_ENTER(core, value);

    PINTA_CHECK(pinta_lib_stack_pop(thread, &value));
    PINTA_CHECK(pinta_lib_array_set_item(core, &frame->function_arguments, index, &value));

    PINTA_GC_RETURN(core);
}

PintaException pinta_code_store_item(PintaThread *thread)
{
    PintaException exception = PINTA_OK;
    PintaCore *core;
    i32 index_value = 0;
    PintaType *type;
    struct
    {
        PintaReference array;
        PintaReference index;
        PintaReference value;
    } gc;

    pinta_debug_assert_code(thread);

    core = thread->core;
    PINTA_GC_ENTER(core, gc);

    PINTA_CHECK(pinta_lib_stack_pop(thread, &gc.array));
    PINTA_CHECK(pinta_lib_stack_pop(thread, &gc.index));
    PINTA_CHECK(pinta_lib_stack_pop(thread, &gc.value));

    if (gc.array.reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (gc.index.reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (gc.index.reference->block_kind != PINTA_KIND_INTEGER && gc.index.reference->block_kind != PINTA_KIND_DECIMAL)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    if (gc.index.reference->block_kind == PINTA_KIND_DECIMAL)
        PINTA_CHECK(pinta_lib_decimal_to_int32(core, &gc.index, &gc.index));

    index_value = pinta_integer_ref_get_value(&gc.index);
    if (index_value < 0)
        PINTA_THROW(PINTA_EXCEPTION_INDEX_OUT_OF_BOUNDS);

    type = pinta_core_get_type(core, &gc.array);
    PINTA_CHECK(type->set_item(core, &gc.array, (u32)index_value, &gc.value));

    PINTA_GC_RETURN(core);
}

PintaException pinta_code_load_local(PintaThread *thread, u32 index)
{
    PintaException exception = PINTA_OK;
    PintaCore *core;
    PintaStackFrame *frame;
    PintaReference value;

    pinta_assert(index != PINTA_CODE_TOKEN_EMPTY);
    pinta_debug_assert_code(thread);

    frame = thread->frame;
    if (frame->function_locals.reference == NULL)
        return (PINTA_EXCEPTION_INVALID_OPERATION);

    core = thread->core;
    PINTA_GC_ENTER(core, value);

    PINTA_CHECK(pinta_lib_array_get_item(core, &frame->function_locals, index, &value));
    PINTA_CHECK(pinta_lib_stack_push(thread, &value));

    PINTA_GC_RETURN(core);
}

PintaException pinta_code_load_global(PintaThread *thread, u32 index)
{
    PintaException exception = PINTA_OK;
    PintaCore *core;
    PintaModuleDomain *domain;
    PintaReference value;

    pinta_assert(index != PINTA_CODE_TOKEN_EMPTY);
    pinta_debug_assert_code(thread);

    domain = thread->domain;
    if (domain->globals.reference == NULL)
        return (PINTA_EXCEPTION_INVALID_OPERATION);

    core = thread->core;
    PINTA_GC_ENTER(core, value);

    PINTA_CHECK(pinta_lib_array_get_item(core, &domain->globals, index, &value));
    PINTA_CHECK(pinta_lib_stack_push(thread, &value));

    PINTA_GC_RETURN(core);
}

PintaException pinta_code_load_argument(PintaThread *thread, u32 index)
{
    PintaException exception = PINTA_OK;
    PintaCore *core;
    PintaStackFrame *frame;
    PintaReference value;

    pinta_assert(index != PINTA_CODE_TOKEN_EMPTY);
    pinta_debug_assert_code(thread);

    frame = thread->frame;
    if (frame->function_arguments.reference == NULL)
        return (PINTA_EXCEPTION_INVALID_OPERATION);

    core = thread->core;
    PINTA_GC_ENTER(core, value);

    PINTA_CHECK(pinta_lib_array_get_item(core, &frame->function_arguments, index, &value));
    PINTA_CHECK(pinta_lib_stack_push(thread, &value));

    PINTA_GC_RETURN(core);
}

PintaException pinta_code_load_item(PintaThread *thread)
{
    PintaException exception = PINTA_OK;
    PintaCore *core;
    i32 index_value = 0;
    PintaType *type;
    struct
    {
        PintaReference array;
        PintaReference index;
        PintaReference value;
    } gc;

    pinta_debug_assert_code(thread);

    core = thread->core;
    PINTA_GC_ENTER(core, gc);

    PINTA_CHECK(pinta_lib_stack_pop(thread, &gc.array));
    PINTA_CHECK(pinta_lib_stack_pop(thread, &gc.index));

    if (gc.array.reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (gc.index.reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (gc.index.reference->block_kind != PINTA_KIND_INTEGER && gc.index.reference->block_kind != PINTA_KIND_DECIMAL)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    if (gc.index.reference->block_kind == PINTA_KIND_DECIMAL)
        PINTA_CHECK(pinta_lib_decimal_to_int32(core, &gc.index, &gc.index));

    index_value = pinta_integer_ref_get_value(&gc.index);
    if (index_value < 0)
        PINTA_THROW(PINTA_EXCEPTION_INDEX_OUT_OF_BOUNDS);

    type = pinta_core_get_type(core, &gc.array);
    PINTA_CHECK(type->get_item(core, &gc.array, (u32)index_value, &gc.value));
    PINTA_CHECK(pinta_lib_stack_push(thread, &gc.value));

    PINTA_GC_RETURN(core);

}

PintaException pinta_code_duplicate(PintaThread *thread, u32 count)
{
    pinta_debug_assert_code(thread);

    return pinta_lib_stack_duplicate(thread, count);
}

PintaException pinta_code_pop(PintaThread *thread)
{
    pinta_debug_assert_code(thread);

    return pinta_lib_stack_discard(thread, 1);
}

PintaException pinta_code_get_length(PintaThread *thread)
{
    PintaException exception = PINTA_OK;
    PintaCore *core;
    PintaType *type;
    u32 length = 0;
    struct
    {
        PintaReference value;
        PintaReference result;
    } gc;

    pinta_debug_assert_code(thread);

    core = thread->core;
    PINTA_GC_ENTER(core, gc);

    PINTA_CHECK(pinta_lib_stack_pop(thread, &gc.value));

    if (gc.value.reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    type = pinta_core_get_type(core, &gc.value);

    PINTA_CHECK(type->get_length(core, &gc.value, &length));
    PINTA_CHECK(pinta_lib_integer_alloc_value(core, (i32)length, &gc.result));
    PINTA_CHECK(pinta_lib_stack_push(thread, &gc.result));

    PINTA_GC_RETURN(core);
}

PintaException pinta_code_exit(PintaThread *thread)
{
    pinta_debug_assert_code(thread);

    thread->code_finished = 1;
    thread->code_result = PINTA_OK;

    return PINTA_OK;
}

PintaException pinta_code_load_blob(PintaThread *thread, u32 token)
{
    PintaException exception = PINTA_OK;
    PintaCore *core;
    PintaCodeBlob blob_value, *blob = &blob_value;
    PintaHeapObject *result;

    pinta_assert(token != PINTA_CODE_TOKEN_EMPTY);
    pinta_debug_assert_code(thread);

    core = thread->core;

    PINTA_CHECK(pinta_module_get_blob(thread->domain, token, blob));

    result = pinta_blob_alloc_object(core, blob->blob_length);
    if (result == NULL)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_MEMORY);
    if (pinta_module_is_compressed_data(thread->domain))
    {
        u8 *data = pinta_blob_get_data(result);
        pinta_decompress(blob->blob_data, blob->blob_length, data);
    }
    else
    {
        PINTA_CHECK(pinta_blob_write_data(result, 0, blob->blob_data, blob->blob_length));
    }

    PINTA_CHECK(pinta_stack_push(thread, result));

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_code_error(PintaThread *thread)
{
    PintaException exception = PINTA_OK;
    PintaCore *core;
    PintaReference value;

    pinta_debug_assert_code(thread);

    core = thread->core;
    PINTA_GC_ENTER(core, value);

    PINTA_CHECK(pinta_lib_stack_pop(thread, &value));

    if (value.reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (value.reference->block_kind != PINTA_KIND_INTEGER && value.reference->block_kind != PINTA_KIND_DECIMAL)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    if (value.reference->block_kind == PINTA_KIND_DECIMAL)
        PINTA_CHECK(pinta_lib_decimal_to_int32(core, &value, &value));

    thread->code_finished = 1;
    thread->code_result = (u32)pinta_integer_ref_get_value(&value);

    PINTA_GC_RETURN(core);
}

PintaException pinta_code_new_function(PintaThread *thread, u32 token)
{
    PintaException exception = PINTA_OK;
    PintaCore *core;
    PintaStackFrame *frame;
    struct
    {
        PintaReference function;
        PintaReference closure;
    } gc;

    pinta_assert(token != PINTA_CODE_TOKEN_EMPTY);
    pinta_debug_assert_code(thread);

    core = thread->core;
    PINTA_GC_ENTER(core, gc);

    frame = thread->frame;

    PINTA_CHECK(pinta_lib_function_managed_alloc_closure(core, &frame->function_closure, &frame->function_arguments, &frame->function_locals, &gc.closure));
    PINTA_CHECK(pinta_lib_function_managed_alloc(core, thread->domain, token, NULL, &gc.closure, NULL, NULL, &gc.function));
    PINTA_CHECK(pinta_lib_stack_push(thread, &gc.function));

    PINTA_GC_RETURN(core);
}

PintaException pinta_code_new_object(PintaThread *thread)
{
    PintaException exception = PINTA_OK;
    PintaCore *core;
    PintaReference value;

    pinta_debug_assert_code(thread);

    core = thread->core;
    PINTA_GC_ENTER(core, value);

    PINTA_CHECK(pinta_lib_object_alloc(core, NULL, NULL, &value));
    PINTA_CHECK(pinta_lib_stack_push(thread, &value));

    PINTA_GC_RETURN(core);
}

PintaException pinta_code_fill_arguments(PintaThread *thread, PintaCodeFunction *function, PintaReference *function_binding, u8 bind_this, PintaReference *stack_arguments, u32 stack_arguments_count, PintaReference *function_this, PintaReference *arguments)
{
    PintaException exception = PINTA_OK;
    PintaCore *core;
    u32 binding_array_length;
    u32 arguments_array_length;

    core = thread->core;

    binding_array_length = 0;
    if (function_binding->reference != NULL)
    {
        if (function_binding->reference->block_kind != PINTA_KIND_ARRAY)
            PINTA_THROW(PINTA_EXCEPTION_ENGINE);

        binding_array_length = pinta_array_ref_get_length(function_binding);
    }

    if (binding_array_length > 0)
    {
        if (bind_this)
            PINTA_CHECK(pinta_lib_array_get_item(core, function_binding, 0, function_this));

        binding_array_length = binding_array_length - 1;
    }

    arguments_array_length = stack_arguments_count + binding_array_length;
    if (function != NULL && function->arguments_count > arguments_array_length && function->arguments_count != PINTA_CODE_TOKEN_EMPTY)
        arguments_array_length = function->arguments_count;

    PINTA_CHECK(pinta_lib_array_alloc(core, arguments_array_length, arguments));
    if (binding_array_length > 0)
        pinta_array_ref_copy_array(arguments, 0, function_binding, 1, binding_array_length);

    pinta_array_ref_copy(arguments, binding_array_length, stack_arguments, stack_arguments_count);

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_code_invoke(PintaThread *thread, u32 arguments_count, u8 has_this)
{
    PintaException exception = PINTA_OK;
    PintaReference *stack;
    u32 offset;

    pinta_assert(arguments_count != PINTA_CODE_TOKEN_EMPTY);
    pinta_debug_assert_code(thread);

    pinta_debug_raise_before_invoke(thread, arguments_count, has_this);

    offset = arguments_count + 1;
    if (has_this)
        offset = offset + 1;

    PINTA_CHECK(pinta_lib_stack_get_reference(thread, offset, &stack));

    if (has_this)
        stack = &stack[1]; // TODO: implement something like stack iterator, this assumes that stack grows up and it might break if I decided to go with stack that grows down

    if (stack->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (stack->reference->block_kind == PINTA_KIND_FUNCTION_MANAGED)
        PINTA_CHECK(pinta_code_invoke_function_managed(thread, arguments_count, has_this));
    else if (stack->reference->block_kind == PINTA_KIND_FUNCTION_NATIVE)
        PINTA_CHECK(pinta_code_invoke_function_native(thread, arguments_count, has_this));
    else
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    pinta_debug_raise_after_invoke(thread);

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_code_invoke_function_managed(PintaThread *thread, u32 arguments_count, u8 has_this)
{
    PintaException exception = PINTA_OK;
    PintaCore *core;
    PintaReference *stack;
    PintaStackFrame *frame;
    u32 offset;
    PintaCodeFunction code_function_value, *code_function = &code_function_value;
    PintaModuleDomain *function_domain;
    u32 function_token;
    struct
    {
        PintaReference function_reference;
        PintaReference function_this;
        PintaReference function_binding;
        PintaReference function_closure;
        PintaReference function_arguments;
        PintaReference function_locals;
        PintaReference return_value;
    } gc;

    pinta_assert(arguments_count != PINTA_CODE_TOKEN_EMPTY);
    pinta_debug_assert_code(thread);

    core = thread->core;
    PINTA_GC_ENTER(core, gc);

    offset = arguments_count + 1;
    if (has_this)
        offset = offset + 1;

    PINTA_CHECK(pinta_lib_stack_get_reference(thread, offset, &stack));

    if (has_this)
    {
        gc.function_this.reference = stack->reference;
        stack = &stack[1]; // TODO: implement something like stack iterator, this assumes that stack grows up and it might break if I decided to go with stack that grows down
    }
    else
    {
        gc.function_this.reference = NULL;
    }

    gc.function_reference.reference = stack->reference;
    stack = &stack[1];

    if (gc.function_reference.reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (gc.function_reference.reference->block_kind != PINTA_KIND_FUNCTION_MANAGED)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    function_domain = pinta_function_managed_ref_get_domain(&gc.function_reference);
    if (function_domain == NULL)
        PINTA_THROW(PINTA_EXCEPTION_ENGINE);

    function_token = pinta_function_managed_ref_get_token(&gc.function_reference);
    if (function_token == PINTA_CODE_TOKEN_EMPTY)
        PINTA_THROW(PINTA_EXCEPTION_ENGINE);

    PINTA_CHECK(pinta_module_get_function(function_domain, function_token, code_function));

    PINTA_CHECK(pinta_lib_function_managed_get_body(core, &gc.function_reference, &gc.function_binding, &gc.function_closure, NULL, NULL));

    PINTA_CHECK(pinta_code_fill_arguments(thread, code_function, &gc.function_binding, 1, stack, arguments_count, &gc.function_this, &gc.function_arguments));
    PINTA_CHECK(pinta_lib_stack_discard(thread, offset));

    PINTA_CHECK(pinta_lib_array_alloc(core, code_function->locals_count, &gc.function_locals));
    PINTA_CHECK(pinta_lib_frame_push(thread));

    frame = thread->frame;

    frame->return_domain = thread->domain;
    frame->discard_result = 0;
    frame->function_this.reference = gc.function_this.reference;
    frame->function_closure.reference = gc.function_closure.reference;
    frame->function_arguments.reference = gc.function_arguments.reference;
    frame->function_locals.reference = gc.function_locals.reference;

    frame->code_start = code_function->code_start;
    frame->code_end = code_function->code_end;
    frame->return_address = thread->code_next_pointer;

    thread->domain = function_domain;
    thread->code_next_pointer = code_function->code_start;
    thread->code_pointer = code_function->code_start;

    PINTA_GC_RETURN(core);
}

PintaException pinta_code_invoke_function_native(PintaThread *thread, u32 arguments_count, u8 has_this)
{
    PintaException exception = PINTA_OK;
    PintaCore *core;
    PintaReference *stack;
    u32 offset;
    PintaFunctionDelegate function_delegate;
    u8 discard_return_value = 0;
    struct
    {
        PintaReference function_reference;
        PintaReference function_this;
        PintaReference function_binding;
        PintaReference function_arguments;
        PintaReference return_value;
    } gc;

    pinta_assert(arguments_count != PINTA_CODE_TOKEN_EMPTY);
    pinta_debug_assert_code(thread);

    core = thread->core;
    PINTA_GC_ENTER(core, gc);

    offset = arguments_count + 1;
    if (has_this)
        offset = offset + 1;

    PINTA_CHECK(pinta_lib_stack_get_reference(thread, offset, &stack));

    if (has_this)
    {
        gc.function_this.reference = stack->reference;
        stack = &stack[1]; // TODO: implement something like stack iterator, this assumes that stack grows up and it might break if I decided to go with stack that grows down
    }
    else
    {
        gc.function_this.reference = NULL;
    }

    gc.function_reference.reference = stack->reference;
    stack = &stack[1];

    if (gc.function_reference.reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (gc.function_reference.reference->block_kind != PINTA_KIND_FUNCTION_NATIVE)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    function_delegate = pinta_function_native_ref_get_delegate(&gc.function_reference);
    if (function_delegate == NULL)
        PINTA_THROW(PINTA_EXCEPTION_INVALID_OPERATION);

    PINTA_CHECK(pinta_lib_function_native_get_body(core, &gc.function_reference, &gc.function_binding, NULL, NULL, NULL));

    PINTA_CHECK(pinta_code_fill_arguments(thread, NULL, &gc.function_binding, 1, stack, arguments_count, &gc.function_this, &gc.function_arguments));
    PINTA_CHECK(pinta_lib_stack_discard(thread, offset));

    PINTA_CHECK(function_delegate(thread->core, thread, &gc.function_reference, &gc.function_this, &gc.function_arguments, &discard_return_value, &gc.return_value));

    if (!discard_return_value)
        PINTA_CHECK(pinta_lib_stack_push(thread, &gc.return_value));

    PINTA_GC_RETURN(core);
}

PintaException pinta_code_load_closure(PintaThread *thread, u32 closure_index, u32 value_index)
{
    PintaException exception = PINTA_OK;
    PintaCore *core;
    PintaStackFrame *frame;
    struct
    {
        PintaReference array;
        PintaReference value;
    } gc;

    pinta_assert(closure_index != PINTA_CODE_TOKEN_EMPTY);
    pinta_assert(value_index != PINTA_CODE_TOKEN_EMPTY);

    pinta_debug_assert_code(thread);

    frame = thread->frame;
    if (frame->function_closure.reference == NULL)
        return (PINTA_EXCEPTION_INVALID_OPERATION);

    core = thread->core;
    PINTA_GC_ENTER(core, gc);

    PINTA_CHECK(pinta_lib_array_get_item(core, &frame->function_closure, closure_index, &gc.array));
    PINTA_CHECK(pinta_lib_array_get_item(core, &gc.array, value_index, &gc.value));
    PINTA_CHECK(pinta_lib_stack_push(thread, &gc.value));

    PINTA_GC_RETURN(core);
}

PintaException pinta_code_store_closure(PintaThread *thread, u32 closure_index, u32 value_index)
{
    PintaException exception = PINTA_OK;
    PintaCore *core;
    PintaStackFrame *frame;
    struct
    {
        PintaReference array;
        PintaReference value;
    } gc;

    pinta_assert(closure_index != PINTA_CODE_TOKEN_EMPTY);
    pinta_assert(value_index != PINTA_CODE_TOKEN_EMPTY);

    pinta_debug_assert_code(thread);

    frame = thread->frame;
    if (frame->function_closure.reference == NULL)
        return (PINTA_EXCEPTION_INVALID_OPERATION);

    core = thread->core;
    PINTA_GC_ENTER(core, gc);

    PINTA_CHECK(pinta_lib_stack_pop(thread, &gc.value));
    PINTA_CHECK(pinta_lib_array_get_item(core, &frame->function_closure, closure_index, &gc.array));
    PINTA_CHECK(pinta_lib_array_set_item(core, &gc.array, value_index, &gc.value));

    PINTA_GC_RETURN(core);
}

PintaException pinta_code_load_this(PintaThread *thread)
{
    PintaException exception = PINTA_OK;
    PintaReference *function_this;

    function_this = &thread->frame->function_this;
    if (function_this->reference == NULL)
        PINTA_CHECK(pinta_code_get_global(thread, &function_this));

    PINTA_CHECK(pinta_lib_stack_push(thread, function_this));
PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_code_load_member(PintaThread *thread)
{
    PintaException exception = PINTA_OK;
    PintaCore *core;
    PintaType *object_type;
    u8 is_accessor;
    struct
    {
        PintaReference object;
        PintaReference name;
        PintaReference value;
    } gc;

    pinta_debug_assert_code(thread);

    core = thread->core;
    PINTA_GC_ENTER(core, gc);

    PINTA_CHECK(pinta_lib_stack_pop(thread, &gc.name));
    PINTA_CHECK(pinta_lib_stack_pop(thread, &gc.object));

    object_type = pinta_core_get_type(core, &gc.object);
    if (object_type->get_member == NULL)
        PINTA_THROW(PINTA_EXCEPTION_INVALID_OPERATION);

    PINTA_CHECK(object_type->get_member(core, &gc.object, &gc.name, &is_accessor, &gc.value));
    if (is_accessor)
    {
        PINTA_CHECK(pinta_lib_stack_push(thread, &gc.object));
        PINTA_CHECK(pinta_lib_stack_push(thread, &gc.value));
        PINTA_CHECK(pinta_code_invoke(thread, 0, 1));
        PINTA_RETURN();
    }
    else
    {
        PINTA_CHECK(pinta_lib_stack_push(thread, &gc.value));
    }

    PINTA_GC_RETURN(core);
}

PintaException pinta_code_store_member(PintaThread *thread)
{
    PintaException exception = PINTA_OK;
    PintaCore *core;
    PintaType *object_type;
    u8 is_accessor;
    struct
    {
        PintaReference object;
        PintaReference name;
        PintaReference value;
        PintaReference setter;
    } gc;

    pinta_debug_assert_code(thread);

    core = thread->core;
    PINTA_GC_ENTER(core, gc);

    PINTA_CHECK(pinta_lib_stack_pop(thread, &gc.value));
    PINTA_CHECK(pinta_lib_stack_pop(thread, &gc.name));
    PINTA_CHECK(pinta_lib_stack_pop(thread, &gc.object));

    object_type = pinta_core_get_type(core, &gc.object);
    if (object_type->set_member == NULL)
        PINTA_THROW(PINTA_EXCEPTION_INVALID_OPERATION);

    PINTA_CHECK(object_type->set_member(core, &gc.object, &gc.name, &gc.value, &is_accessor, &gc.setter));
    if (is_accessor)
    {
        PINTA_CHECK(pinta_lib_stack_push(thread, &gc.object));
        PINTA_CHECK(pinta_lib_stack_push(thread, &gc.setter));
        PINTA_CHECK(pinta_lib_stack_push(thread, &gc.value));
        PINTA_CHECK(pinta_code_invoke(thread, 1, 1));
        PINTA_RETURN();
    }

    PINTA_GC_RETURN(core);
}

PintaException pinta_code_new(PintaThread *thread, u32 arguments_count)
{
    PintaException exception = PINTA_OK;
    PintaReference *stack;
    u32 offset;

    pinta_assert(arguments_count != PINTA_CODE_TOKEN_EMPTY);
    pinta_debug_assert_code(thread);

    offset = arguments_count + 1;

    PINTA_CHECK(pinta_lib_stack_get_reference(thread, offset, &stack));

    if (stack->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (stack->reference->block_kind == PINTA_KIND_FUNCTION_MANAGED)
        PINTA_CHECK(pinta_code_construct_function(thread, arguments_count));
    else
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_code_construct_function(PintaThread *thread, u32 arguments_count)
{
    PintaException exception = PINTA_OK;
    PintaCore *core;
    PintaReference *stack;
    PintaStackFrame *frame;
    u32 offset;
    PintaCodeFunction code_function_value, *code_function = &code_function_value;
    PintaModuleDomain *function_domain;
    u32 function_token;
    struct
    {
        PintaReference function_reference;
        PintaReference function_binding;
        PintaReference function_closure;
        PintaReference function_prototype;
        PintaReference function_arguments;
        PintaReference function_locals;
        PintaReference object_instance;
    } gc;

    pinta_assert(arguments_count != PINTA_CODE_TOKEN_EMPTY);
    pinta_debug_assert_code(thread);

    core = thread->core;
    PINTA_GC_ENTER(core, gc);

    offset = arguments_count + 1;

    PINTA_CHECK(pinta_lib_stack_get_reference(thread, offset, &stack));

    gc.function_reference.reference = stack->reference;
    stack = &stack[1];

    if (gc.function_reference.reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (gc.function_reference.reference->block_kind != PINTA_KIND_FUNCTION_MANAGED)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    function_domain = pinta_function_managed_ref_get_domain(&gc.function_reference);
    if (function_domain == NULL)
        PINTA_THROW(PINTA_EXCEPTION_ENGINE);

    function_token = pinta_function_managed_ref_get_token(&gc.function_reference);
    if (function_token == PINTA_CODE_TOKEN_EMPTY)
        PINTA_THROW(PINTA_EXCEPTION_ENGINE);

    PINTA_CHECK(pinta_module_get_function(function_domain, function_token, code_function));

    PINTA_CHECK(pinta_lib_function_managed_get_body(core, &gc.function_reference, &gc.function_binding, &gc.function_closure, &gc.function_prototype, NULL));

    PINTA_CHECK(pinta_code_fill_arguments(thread, code_function, &gc.function_binding, 0, stack, arguments_count, NULL, &gc.function_arguments));
    PINTA_CHECK(pinta_lib_stack_discard(thread, offset));

    PINTA_CHECK(pinta_lib_object_alloc(core, NULL, &gc.function_prototype, &gc.object_instance));
    PINTA_CHECK(pinta_lib_stack_push(thread, &gc.object_instance)); // pre-push created instance

    PINTA_CHECK(pinta_lib_array_alloc(core, code_function->locals_count, &gc.function_locals));
    PINTA_CHECK(pinta_lib_frame_push(thread));

    frame = thread->frame;

    frame->return_domain = thread->domain;
    frame->discard_result = 1;
    frame->function_this.reference = gc.object_instance.reference;
    frame->function_closure.reference = gc.function_closure.reference;
    frame->function_arguments.reference = gc.function_arguments.reference;
    frame->function_locals.reference = gc.function_locals.reference;

    frame->code_start = code_function->code_start;
    frame->code_end = code_function->code_end;
    frame->return_address = thread->code_next_pointer;

    thread->domain = function_domain;
    thread->code_next_pointer = code_function->code_start;

    PINTA_GC_RETURN(core);
}

PintaException pinta_code_match(PintaThread *thread, u8 match_flags)
{
    PintaException exception = PINTA_OK;
    PintaCore *core;
    PintaType *string_type, *pattern_type;
    u32 is_match = 0;
    u8 type, arguments_count;

    struct
    {
        PintaReference offset;
        PintaReference length;
        PintaReference string;
        PintaReference pattern;
        PintaReference result;
    } gc;

    pinta_debug_assert_code(thread);

    type = (u8)((match_flags & PINTA_CODE_MATCH_TYPE_MASK) >> PINTA_CODE_MATCH_TYPE_BITS);
    arguments_count = (u8)(match_flags & PINTA_CODE_MATCH_ARGUMENTS_MASK);

    if (type == PINTA_CODE_MATCH_TYPE_SIMPLE)
    {
        if (arguments_count > 4 || arguments_count < 2)
            return (PINTA_EXCEPTION_INVALID_OPCODE);
    }
    else if (type == PINTA_CODE_MATCH_TYPE_PAN)
    {
        if (arguments_count > 3 || arguments_count < 2)
            return (PINTA_EXCEPTION_INVALID_OPCODE);
    }
    else
    {
        return (PINTA_EXCEPTION_INVALID_OPCODE);
    }

    core = thread->core;
    PINTA_GC_ENTER(core, gc);

    if (type == PINTA_CODE_MATCH_TYPE_SIMPLE)
    {
        if (arguments_count == 4)
            PINTA_CHECK(pinta_lib_stack_pop(thread, &gc.length));

        if (arguments_count >= 3)
            PINTA_CHECK(pinta_lib_stack_pop(thread, &gc.offset));
    }
    else if (type == PINTA_CODE_MATCH_TYPE_PAN)
    {
        if (arguments_count == 3)
            PINTA_CHECK(pinta_lib_stack_pop(thread, &gc.length));
    }

    PINTA_CHECK(pinta_lib_stack_pop(thread, &gc.pattern));
    PINTA_CHECK(pinta_lib_stack_pop(thread, &gc.string));

    if (gc.pattern.reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (gc.string.reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    string_type = pinta_core_get_type(core, &gc.string);
    pattern_type = pinta_core_get_type(core, &gc.pattern);

    if (!string_type->is_string || !pattern_type->is_string)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    PINTA_CHECK(pinta_lib_string_to_string(core, &gc.string, &gc.string));
    PINTA_CHECK(pinta_lib_string_to_string(core, &gc.pattern, &gc.pattern));

    if (type == PINTA_CODE_MATCH_TYPE_SIMPLE)
        PINTA_CHECK(pinta_lib_pattern_simple(core, &gc.string, &gc.pattern, &gc.offset, &gc.length, &is_match));
    else if (type == PINTA_CODE_MATCH_TYPE_PAN)
        PINTA_CHECK(pinta_lib_pattern_pan(core, &gc.string, &gc.pattern, &gc.length, &is_match));
    else
        PINTA_THROW(PINTA_EXCEPTION_INVALID_OPCODE);

    PINTA_CHECK(pinta_lib_integer_alloc_value(core, is_match, &gc.result));
    PINTA_CHECK(pinta_lib_stack_push(thread, &gc.result));

    PINTA_GC_RETURN(core);
}

PintaException pinta_code_step(PintaThread *thread)
{
    PintaException exception = PINTA_OK;
    u8 code = PINTA_CODE_NOP, match_flags = 0;
    i32 offset = 0;
    i32 integer_value = 0;
    u32 token = 0;
    u32 arguments_count = 0;
    u32 index = 0;

    pinta_debug_assert_code(thread);
    pinta_debug_raise_step(thread);

    PINTA_CHECK(pinta_code_next(thread, &code));

    if (code == PINTA_CODE_BREAK)
        pinta_debug_raise_break(thread, &code);

    switch (code)
    {
    case PINTA_CODE_NOP:
        PINTA_CHECK(pinta_code_nop(thread));
        break;
    case PINTA_CODE_ADD:
        PINTA_CHECK(pinta_code_add(thread));
        break;
    case PINTA_CODE_SUBTRACT:
    case PINTA_CODE_MULTIPLY:
    case PINTA_CODE_DIVIDE:
    case PINTA_CODE_REMAINDER:
        PINTA_CHECK(pinta_code_arithmetic(thread, code));
        break;
    case PINTA_CODE_EXCLUSIVE_OR:
        PINTA_CHECK(pinta_code_exclusive_or(thread));
        break;
    case PINTA_CODE_BITWISE_AND:
    case PINTA_CODE_BITWISE_OR:
    case PINTA_CODE_BITWISE_EXCLUSIVE_OR:
        PINTA_CHECK(pinta_code_bitwise(thread, code));
        break;
    case PINTA_CODE_NOT:
    case PINTA_CODE_BITWISE_NOT:
    case PINTA_CODE_NEGATE:
    case PINTA_CODE_COMPARE_NULL:
    case PINTA_CODE_CONVERT_INTEGER:
    case PINTA_CODE_CONVERT_DECIMAL:
    case PINTA_CODE_CONVERT_STRING:
        PINTA_CHECK(pinta_code_unary(thread, code));
        break;
    case PINTA_CODE_COMPARE_EQUAL:
    case PINTA_CODE_COMPARE_LESS_THAN:
    case PINTA_CODE_COMPARE_MORE_THAN:
        PINTA_CHECK(pinta_code_compare(thread, code));
        break;
    case PINTA_CODE_NEW_ARRAY:
        PINTA_CHECK(pinta_code_new_array(thread));
        break;
    case PINTA_CODE_CONCAT:
        PINTA_CHECK(pinta_code_concat(thread));
        break;
    case PINTA_CODE_SUBSTRING:
        PINTA_CHECK(pinta_code_substring(thread));
        break;
    case PINTA_CODE_JUMP:
        PINTA_CHECK(pinta_code_integer(thread, &offset));
        PINTA_CHECK(pinta_code_jump(thread, offset));
        break;
    case PINTA_CODE_JUMP_ZERO:
    case PINTA_CODE_JUMP_NOT_ZERO:
        PINTA_CHECK(pinta_code_integer(thread, &offset));
        PINTA_CHECK(pinta_code_jump_conditional(thread, code, offset));
        break;
    case PINTA_CODE_CALL:
        PINTA_CHECK(pinta_code_token(thread, &token));
        PINTA_CHECK(pinta_code_token(thread, &arguments_count));
        PINTA_CHECK(pinta_code_call(thread, token, arguments_count));
        break;
    case PINTA_CODE_CALL_INTERNAL:
        PINTA_CHECK(pinta_code_token(thread, &token));
        PINTA_CHECK(pinta_code_token(thread, &arguments_count));
        PINTA_CHECK(pinta_code_call_internal(thread, token, arguments_count));
        break;
    case PINTA_CODE_RETURN:
        PINTA_CHECK(pinta_code_return(thread));
        break;
    case PINTA_CODE_LOAD_NULL:
        PINTA_CHECK(pinta_code_load_null(thread));
        break;
    case PINTA_CODE_LOAD_INTEGER_ZERO:
        PINTA_CHECK(pinta_code_load_integer_zero(thread));
        break;
    case PINTA_CODE_LOAD_DECIMAL_ZERO:
        PINTA_CHECK(pinta_code_load_decimal_zero(thread));
        break;
    case PINTA_CODE_LOAD_INTEGER_ONE:
        PINTA_CHECK(pinta_code_load_integer_one(thread));
        break;
    case PINTA_CODE_LOAD_DECIMAL_ONE:
        PINTA_CHECK(pinta_code_load_decimal_one(thread));
        break;
    case PINTA_CODE_LOAD_INTEGER:
        PINTA_CHECK(pinta_code_integer(thread, &integer_value));
        PINTA_CHECK(pinta_code_load_integer(thread, integer_value));
        break;
    case PINTA_CODE_LOAD_STRING:
        PINTA_CHECK(pinta_code_token(thread, &token));
        PINTA_CHECK(pinta_code_load_string(thread, token));
        break;
    case PINTA_CODE_STORE_LOCAL:
        PINTA_CHECK(pinta_code_token(thread, &index));
        PINTA_CHECK(pinta_code_store_local(thread, index));
        break;
    case PINTA_CODE_STORE_GLOBAL:
        PINTA_CHECK(pinta_code_token(thread, &index));
        PINTA_CHECK(pinta_code_store_global(thread, index));
        break;
    case PINTA_CODE_STORE_ARGUMENT:
        PINTA_CHECK(pinta_code_token(thread, &index));
        PINTA_CHECK(pinta_code_store_argument(thread, index));
        break;
    case PINTA_CODE_STORE_ITEM:
        PINTA_CHECK(pinta_code_store_item(thread));
        break;
    case PINTA_CODE_LOAD_LOCAL:
        PINTA_CHECK(pinta_code_token(thread, &index));
        PINTA_CHECK(pinta_code_load_local(thread, index));
        break;
    case PINTA_CODE_LOAD_GLOBAL:
        PINTA_CHECK(pinta_code_token(thread, &index));
        PINTA_CHECK(pinta_code_load_global(thread, index));
        break;
    case PINTA_CODE_LOAD_ARGUMENT:
        PINTA_CHECK(pinta_code_token(thread, &index));
        PINTA_CHECK(pinta_code_load_argument(thread, index));
        break;
    case PINTA_CODE_LOAD_ITEM:
        PINTA_CHECK(pinta_code_load_item(thread));
        break;
    case PINTA_CODE_DUPLICATE:
        PINTA_CHECK(pinta_code_duplicate(thread, 1));
        break;
    case PINTA_CODE_POP:
        PINTA_CHECK(pinta_code_pop(thread));
        break;
    case PINTA_CODE_EXIT:
        PINTA_CHECK(pinta_code_exit(thread));
        break;
    case PINTA_CODE_GET_LENGTH:
        PINTA_CHECK(pinta_code_get_length(thread));
        break;
    case PINTA_CODE_ERROR:
        PINTA_CHECK(pinta_code_error(thread));
        break;
    case PINTA_CODE_LOAD_BLOB:
        PINTA_CHECK(pinta_code_token(thread, &token));
        PINTA_CHECK(pinta_code_load_blob(thread, token));
        break;
    case PINTA_CODE_LOAD_CLOSURE:
        PINTA_CHECK(pinta_code_token(thread, &token));
        PINTA_CHECK(pinta_code_token(thread, &index));
        PINTA_CHECK(pinta_code_load_closure(thread, token, index));
        break;
    case PINTA_CODE_LOAD_MEMBER:
        PINTA_CHECK(pinta_code_load_member(thread));
        break;
    case PINTA_CODE_LOAD_THIS:
        PINTA_CHECK(pinta_code_load_this(thread));
        break;
    case PINTA_CODE_STORE_CLOSURE:
        PINTA_CHECK(pinta_code_token(thread, &token));
        PINTA_CHECK(pinta_code_token(thread, &index));
        PINTA_CHECK(pinta_code_store_closure(thread, token, index));
        break;
    case PINTA_CODE_STORE_MEMBER:
        PINTA_CHECK(pinta_code_store_member(thread));
        break;
    case PINTA_CODE_NEW:
        PINTA_CHECK(pinta_code_token(thread, &token));
        PINTA_CHECK(pinta_code_new(thread, token));
        break;
    case PINTA_CODE_NEW_FUNCTION:
        PINTA_CHECK(pinta_code_token(thread, &token));
        PINTA_CHECK(pinta_code_new_function(thread, token));
        break;
    case PINTA_CODE_NEW_OBJECT:
        PINTA_CHECK(pinta_code_new_object(thread));
        break;
    case PINTA_CODE_INVOKE:
        PINTA_CHECK(pinta_code_token(thread, &arguments_count));
        PINTA_CHECK(pinta_code_invoke(thread, arguments_count, 0));
        break;
    case PINTA_CODE_INVOKE_MEMBER:
        PINTA_CHECK(pinta_code_token(thread, &arguments_count));
        PINTA_CHECK(pinta_code_invoke(thread, arguments_count, 1));
        break;
    case PINTA_CODE_DUPLICATE_MULTIPLE:
        PINTA_CHECK(pinta_code_token(thread, &token));
        PINTA_CHECK(pinta_code_duplicate(thread, token));
        break;
    case PINTA_CODE_MATCH:
        PINTA_CHECK(pinta_code_next(thread, &match_flags));
        PINTA_CHECK(pinta_code_match(thread, match_flags));
        break;
    case PINTA_CODE_BREAK:
        exception = PINTA_OK;
        break;
    default:
        PINTA_THROW(PINTA_EXCEPTION_INVALID_OPCODE);
        break;
    }

PINTA_EXIT:

    pinta_debug_raise_exception(thread, exception);
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_code_execute(PintaCore *core)
{
    PintaException exception = PINTA_OK;
    PintaThread *thread;
    PintaDebugger *debugger;

    thread = core->threads;
    debugger = core->debugger;

    if (debugger == NULL)
    {
        while (!thread->code_finished)
        {
            PINTA_CHECK(pinta_code_step(thread));
            thread->code_pointer = thread->code_next_pointer;
        }
    }
    else
    {
        while (!thread->code_finished)
        {
            if (!thread->code_is_suspended)
            {
                PINTA_CHECK(pinta_code_step(thread));
                thread->code_pointer = thread->code_next_pointer;
            }

            pinta_debug_raise_tick(core);
        }
    }

PINTA_EXIT:

    pinta_debug_raise_exit(core, exception);

    return PINTA_EXCEPTION(exception);
}

PintaException pinta_code_execute_module(PintaCore *core, PintaModuleDomain *domain)
{
    PintaException exception = PINTA_OK;
    PintaModule *module;
    struct
    {
        PintaReference function;
    } gc;

    PINTA_GC_ENTER(core, gc);

    module = domain->module;
    PINTA_CHECK(pinta_lib_function_managed_alloc(core, domain, module->start_function_token, NULL, NULL, NULL, NULL, &gc.function));
    PINTA_CHECK(pinta_lib_stack_push(core->threads, &gc.function));
    PINTA_CHECK(pinta_code_invoke(core->threads, 0, 0));
    PINTA_CHECK(pinta_code_execute(core));

    PINTA_GC_RETURN(core);
}
