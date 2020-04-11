#include "pinta_tests.h"

#define PINTA_TEST_FILE(name)   (L"..\\Marius.Pinta.Test.Files\\" L##name)

PintaException pinta_test_load_module(PintaCore *core, wchar *filename, PintaModuleDomain **result)
{
    PintaException exception = PINTA_OK;
    void *domain;
    PintaApiString name;

    name.string_data = filename;
    name.string_length = wcslen(filename);
    name.string_encoding = PINTA_API_ENCODING_UTF16;

    domain = api->load_module(api, &name);
    if (domain == NULL)
        PINTA_THROW(PINTA_EXCEPTION_INVALID_MODULE);

    *result = (PintaModuleDomain *)domain;
PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_test_execute_module(PintaCore *core, wchar *filename, PintaModuleDomain **result)
{
    PintaException exception = PINTA_OK;
    PintaApiString name;
    void *domain;

    name.string_data = filename;
    name.string_length = wcslen(filename);
    name.string_encoding = PINTA_API_ENCODING_UTF16;

    domain = api->load_module(api, &name);
    if (domain == NULL)
        PINTA_THROW(PINTA_EXCEPTION_INVALID_MODULE);
    PINTA_CHECK(api->execute(api, domain));

    if (result)
        *result = domain;

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_test_compare_equal(PintaReference *left, PintaReference *right, i32 *compare_result)
{
    PintaException exception = PINTA_OK;
    u8 is_left_string = 0;
    u8 is_right_string = 0;
    u8 is_left_binary = 0;
    u8 is_right_binary = 0;
    struct
    {
        PintaReference result;
    } gc;

    pinta_debug_assert_code(core->threads);

    PINTA_GC_ENTER(core, gc);

    if (right->reference == NULL && left->reference == NULL)
    {
        *compare_result = 1;
        PINTA_RETURN();
    }

    if (right->reference == NULL || left->reference == NULL)
    {
        *compare_result = 0;
        PINTA_RETURN();
    }

    if (left->reference == right->reference)
    {
        *compare_result = 1;
        PINTA_RETURN();
    }

    switch (left->reference->block_kind)
    {
    case PINTA_KIND_STRING:
    case PINTA_KIND_SUBSTRING:
    case PINTA_KIND_MULTISTRING:
    case PINTA_KIND_CHAR:
        is_left_string = 1;
        break;
    case PINTA_KIND_BLOB:
    case PINTA_KIND_BUFFER:
        is_left_binary = 1;
        break;
    default:
        is_left_string = 0;
        is_left_binary = 0;
        break;
    }

    switch (right->reference->block_kind)
    {
    case PINTA_KIND_STRING:
    case PINTA_KIND_SUBSTRING:
    case PINTA_KIND_MULTISTRING:
    case PINTA_KIND_CHAR:
        is_right_string = 1;
        break;
    case PINTA_KIND_BLOB:
    case PINTA_KIND_BUFFER:
        is_right_binary = 1;
        break;
    default:
        is_right_string = 0;
        is_right_binary = 0;
        break;
    }

    if (left->reference->block_kind == PINTA_KIND_ARRAY || right->reference->block_kind == PINTA_KIND_ARRAY)
    {
        // we have already checked for reference equality
        // array is not equal to anything other than itself
        *compare_result = 0;
    }
    else if (is_left_string && is_right_string)
    {
        PINTA_CHECK(pinta_lib_string_to_string(core, left, left));
        PINTA_CHECK(pinta_lib_string_to_string(core, right, right));

        *compare_result = wcscmp(pinta_string_ref_get_data(left), pinta_string_ref_get_data(right));

        if (*compare_result == 0)
            *compare_result = 1;
        else
            *compare_result = 0;

        PINTA_RETURN();
    }
    else if (is_left_binary && is_right_binary)
    {
        u32 left_length;
        u32 right_length;
        u8 *left_data;
        u8 *right_data;

        if (left->reference->block_kind == PINTA_KIND_BUFFER)
        {
            left_length = pinta_buffer_ref_get_length(left);
            PINTA_CHECK(pinta_lib_buffer_get_blob(core, left, left));
        }
        else
        {
            left_length = pinta_blob_ref_get_length(left);
        }

        if (right->reference->block_kind == PINTA_KIND_BUFFER)
        {
            right_length = pinta_buffer_ref_get_length(right);
            PINTA_CHECK(pinta_lib_buffer_get_blob(core, right, right));
        }
        else
        {
            right_length = pinta_blob_ref_get_length(right);
        }

        if (left->reference == right->reference && left_length == right_length)
        {
            *compare_result = 1;
        }
        else
        {
            if (left_length == right_length)
            {
                left_data = pinta_blob_ref_get_data(left);
                right_data = pinta_blob_ref_get_data(right);

                *compare_result = memcmp(left_data, right_data, left_length);

                if (*compare_result == 0)
                    *compare_result = 1;
                else
                    *compare_result = 0;
            }
            else
            {
                *compare_result = 0;
            }
        }

        PINTA_RETURN();
    }
    else if (left->reference->block_kind == PINTA_KIND_INTEGER && right->reference->block_kind == PINTA_KIND_INTEGER)
    {
        i32 left_value = pinta_integer_ref_get_value(left);
        i32 right_value = pinta_integer_ref_get_value(right);

        if (left_value == right_value)
            *compare_result = 1;
        else
            *compare_result = 0;
    }
    else
    {
        switch (left->reference->block_kind)
        {
        case PINTA_KIND_INTEGER:
            PINTA_CHECK(pinta_lib_decimal_from_int32(core, left, left));
            break;
        case PINTA_KIND_DECIMAL:
            break;
        case PINTA_KIND_STRING:
        case PINTA_KIND_SUBSTRING:
        case PINTA_KIND_MULTISTRING:
        case PINTA_KIND_CHAR:
            PINTA_CHECK(pinta_lib_decimal_try_parse(core, left, NULL, left));
            break;
        case PINTA_KIND_ARRAY:
            PINTA_CHECK(pinta_lib_decimal_alloc_zero(core, left));
            break;
        default:
            PINTA_THROW(PINTA_EXCEPTION_ENGINE);
            break;
        }

        switch (right->reference->block_kind)
        {
        case PINTA_KIND_INTEGER:
            PINTA_CHECK(pinta_lib_decimal_from_int32(core, right, right));
            break;
        case PINTA_KIND_DECIMAL:
            break;
        case PINTA_KIND_STRING:
        case PINTA_KIND_SUBSTRING:
        case PINTA_KIND_MULTISTRING:
        case PINTA_KIND_CHAR:
            PINTA_CHECK(pinta_lib_decimal_try_parse(core, right, NULL, right));
            break;
        case PINTA_KIND_ARRAY:
            PINTA_CHECK(pinta_lib_decimal_alloc_zero(core, right));
            break;
        default:
            PINTA_THROW(PINTA_EXCEPTION_ENGINE);
            break;
        }

        PINTA_CHECK(pinta_lib_decimal_compare(core, left, right, &gc.result));

        if (decimal_is_zero(pinta_decimal_ref_get_value(&gc.result)) == 1)
            *compare_result = 1;
        else
            *compare_result = 0;
    }

    PINTA_GC_RETURN(core);
}

PintaException pinta_test_internal_print(PintaCore *core, PintaReference *arguments, PintaReference *return_value)
{
    PintaException exception = PINTA_OK;
    u32 length;
    u32 index;
    struct
    {
        PintaReference value;
    } gc;

    PINTA_UNUSED(return_value);

    pinta_assert(core != NULL);
    pinta_assert(arguments != NULL);
    pinta_assert(return_value != NULL);
    pinta_debug_assert_code(core->threads);

    PINTA_GC_ENTER(core, gc);

    if (arguments->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (arguments->reference->block_kind != PINTA_KIND_ARRAY)
        PINTA_THROW(PINTA_EXCEPTION_ENGINE);

    length = pinta_array_ref_get_length(arguments);
    if (length == 0)
        PINTA_RETURN();

    for (index = 0; index < length; index++)
    {
        PINTA_CHECK(pinta_lib_array_get_item(core, arguments, index, &gc.value));
        switch (gc.value.reference->block_kind)
        {
        case PINTA_KIND_INTEGER:
            PINTA_CHECK(pinta_lib_integer_to_string(core, &gc.value, &gc.value));
            break;
        case PINTA_KIND_DECIMAL:
            PINTA_CHECK(pinta_lib_decimal_to_string(core, &gc.value, &gc.value));
            break;
        default:
            break;
        }

#if PINTA_DEBUG
        PINTA_CHECK(pinta_core_print_reference(core, &gc.value));
#endif

    }

    PINTA_GC_RETURN(core);
}

PintaException pinta_test_internal_assert(PintaCore *core, PintaReference *arguments, PintaReference *return_value)
{
    PintaException exception = PINTA_OK;
    u32 length;
    i32 compare_result;
    u8 *data;
    size_t converted;
    struct
    {
        PintaReference actual;
        PintaReference expected;
        PintaReference message;
        PintaReference blob;
    } gc;

    PINTA_UNUSED(return_value);

    pinta_assert(core != NULL);
    pinta_assert(arguments != NULL);
    pinta_assert(return_value != NULL);
    pinta_debug_assert_code(core->threads);

    PINTA_GC_ENTER(core, gc);

    if (arguments->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (arguments->reference->block_kind != PINTA_KIND_ARRAY)
        PINTA_THROW(PINTA_EXCEPTION_ENGINE);

    length = pinta_array_ref_get_length(arguments);
    if (length != 2 && length != 3)
        PINTA_THROW(PINTA_EXCEPTION_INVALID_ARGUMENTS);

    PINTA_CHECK(pinta_lib_array_get_item(core, arguments, 0, &gc.expected));
    PINTA_CHECK(pinta_lib_array_get_item(core, arguments, 1, &gc.actual));

    if (length == 3)
        PINTA_CHECK(pinta_lib_array_get_item(core, arguments, 2, &gc.message));

    PINTA_CHECK(pinta_test_compare_equal(&gc.expected, &gc.actual, &compare_result));

    if (length == 2)
    {
        sput_fail_if(compare_result != 1, "assertEqual");
    }
    else
    {
        if (gc.message.reference == NULL)
            PINTA_CHECK(pinta_lib_string_alloc_value(core, L"assertEqual", 11, &gc.message));

        switch (gc.message.reference->block_kind)
        {
        case PINTA_KIND_INTEGER:
            PINTA_CHECK(pinta_lib_integer_to_string(core, &gc.message, &gc.message));
            break;
        case PINTA_KIND_DECIMAL:
            PINTA_CHECK(pinta_lib_decimal_to_string(core, &gc.message, &gc.message));
            break;
        case PINTA_KIND_STRING:
        case PINTA_KIND_SUBSTRING:
        case PINTA_KIND_MULTISTRING:
        case PINTA_KIND_CHAR:
            PINTA_CHECK(pinta_lib_string_to_string(core, &gc.message, &gc.message));
            break;
        case PINTA_KIND_ARRAY:
            PINTA_CHECK(pinta_lib_string_alloc_value(core, L"[array]", 7, &gc.message));
            break;
        default:
            PINTA_CHECK(pinta_lib_string_alloc_value(core, L"assertEqual", 11, &gc.message));
            break;
        }

        PINTA_CHECK(pinta_lib_blob_alloc(core, pinta_string_ref_get_length(&gc.message) * 2, &gc.blob));
        data = pinta_blob_ref_get_data(&gc.blob);
        wcstombs_s(&converted, (char*)data, pinta_blob_ref_get_length(&gc.blob), pinta_string_ref_get_data(&gc.message), pinta_blob_ref_get_length(&gc.blob));

        sput_fail_if(compare_result != 1, (char*)data);
    }
    PINTA_GC_RETURN(core);
}

PintaException pinta_test_internal_integer_eq_6(PintaCore *core, PintaReference *arguments, PintaReference *return_value)
{
    PintaException exception = PINTA_OK;
    u32 length;
    struct
    {
        PintaReference value;
    } gc;

    pinta_assert(arguments != NULL);
    pinta_assert(return_value != NULL);
    pinta_assert(arguments->reference != NULL);
    pinta_assert(arguments->reference->block_kind == PINTA_KIND_ARRAY);
    pinta_debug_assert_code(core->threads);

    PINTA_GC_ENTER(core, gc);

    length = pinta_array_ref_get_length(arguments);
    sput_fail_if(length != 1, "Integer 6 - single argument");

    PINTA_CHECK(pinta_lib_array_get_item(core, arguments, 0, &gc.value));

    sput_fail_if(gc.value.reference == NULL, "Integer 6 - not null");
    sput_fail_if(gc.value.reference->block_kind != PINTA_KIND_INTEGER, "Integer 6 - integer");
    sput_fail_if(pinta_integer_ref_get_value(&gc.value) != 6, "Integer 6 - correct value");

    return_value->reference = NULL;

    PINTA_GC_RETURN(core);
}

PintaException pinta_test_internal_strings_eq_hi(PintaCore *core, PintaReference *arguments, PintaReference *return_value)
{
    PintaException exception = PINTA_OK;
    u32 length;
    struct
    {
        PintaReference value;
    } gc;

    pinta_assert(arguments != NULL);
    pinta_assert(return_value != NULL);
    pinta_assert(arguments->reference != NULL);
    pinta_assert(arguments->reference->block_kind == PINTA_KIND_ARRAY);
    pinta_debug_assert_code(core->threads);

    PINTA_GC_ENTER(core, gc);

    length = pinta_array_ref_get_length(arguments);
    sput_fail_if(length != 2, "Strings hi - two arguments");

    PINTA_CHECK(pinta_lib_array_get_item(core, arguments, 0, &gc.value));

    sput_fail_if(gc.value.reference == NULL, "String hi[0] - not null");
    sput_fail_if(gc.value.reference->block_kind != PINTA_KIND_STRING, "String hi[0] - string");
    sput_fail_if(pinta_string_ref_get_length(&gc.value) != 11, "String hi[0] - correct length");
    sput_fail_if(memcmp(pinta_string_ref_get_data(&gc.value), L"Well, hello", 11 * sizeof(wchar)) != 0, "String hi[0] - correct value");

    PINTA_CHECK(pinta_lib_array_get_item(core, arguments, 1, &gc.value));

    sput_fail_if(gc.value.reference == NULL, "String hi[1] - not null");
    sput_fail_if(gc.value.reference->block_kind != PINTA_KIND_STRING, "String hi[1] - string");
    sput_fail_if(pinta_string_ref_get_length(&gc.value) != 9, "String hi[1] - correct length");
    sput_fail_if(memcmp(pinta_string_ref_get_data(&gc.value), L"Hey there", 9 * sizeof(wchar)) != 0, "String hi[1] - correct value");

    return_value->reference = NULL;

    PINTA_GC_RETURN(core);
}

PintaException pinta_test_internal_decimal_eq_pi(PintaCore *core, PintaReference *arguments, PintaReference *return_value)
{
    PintaException exception = PINTA_OK;
    u32 length;
    decimal pi100;
    decimal diff;
    struct
    {
        PintaReference value;
    } gc;

    pinta_assert(arguments != NULL);
    pinta_assert(return_value != NULL);
    pinta_assert(arguments->reference != NULL);
    pinta_assert(arguments->reference->block_kind == PINTA_KIND_ARRAY);
    pinta_debug_assert_code(core->threads);

    PINTA_GC_ENTER(core, gc);

    length = pinta_array_ref_get_length(arguments);
    sput_fail_if(length != 1, "Decimal 100pi - single argument");

    PINTA_CHECK(pinta_lib_array_get_item(core, arguments, 0, &gc.value));

    sput_fail_if(gc.value.reference == NULL, "Decimal 100pi - not null");
    sput_fail_if(gc.value.reference->block_kind != PINTA_KIND_DECIMAL, "Decimal 100pi - decimal");

    decimal_from_string(L"314.15", 6, &pi100);
    diff = decimal_subtract(pinta_decimal_ref_get_value(&gc.value), pi100);
    sput_fail_if(decimal_is_zero(diff) != 1, "Decimal 100pi - correct value");

    return_value->reference = NULL;

    PINTA_GC_RETURN(core);
}

PintaException pinta_test_internal_unreachable(PintaCore *core, PintaReference *arguments, PintaReference *return_value)
{
    PINTA_UNUSED(core);
    PINTA_UNUSED(arguments);
    PINTA_UNUSED(return_value);

    sput_fail_unless(0, "Unreachable");

    return PINTA_EXCEPTION(PINTA_EXCEPTION_NOT_REACHABLE);
}

PintaException pinta_test_set_global(PintaCore *core, PintaModuleDomain *domain, wchar *nameValue, wchar *valueValue)
{
    PintaException exception = PINTA_OK;
    struct
    {
        PintaReference name;
        PintaReference value;
    } gc;

    PINTA_GC_ENTER(core, gc);

    PINTA_CHECK(pinta_lib_string_alloc_value(core, nameValue, wcslen(nameValue), &gc.name));
    PINTA_CHECK(pinta_lib_string_alloc_value(core, valueValue, wcslen(valueValue), &gc.value));
    PINTA_CHECK(pinta_lib_module_set_global_value(core, domain, &gc.name, &gc.value));

    PINTA_GC_RETURN(core);
}

PintaException pinta_test_expect_buffer(PintaCore *core, PintaReference *arguments, PintaReference *return_value)
{
    PintaException exception = PINTA_OK;
    u32 arguments_length;
    u8 failed = 0;
    struct
    {
        PintaReference array;
        PintaReference buffer;
        PintaReference expected;
        PintaReference message;
    } gc;

    PINTA_GC_ENTER(core, gc);

    if (arguments->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (arguments->reference->block_kind != PINTA_KIND_ARRAY)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    gc.buffer.reference = core->output.reference;
    arguments_length = pinta_array_ref_get_length(arguments);

    if (arguments_length == 0)
        PINTA_RETURN();

    PINTA_CHECK(pinta_lib_array_get_item(core, arguments, 0, &gc.expected));

    if (arguments_length > 1)
        PINTA_CHECK(pinta_lib_array_get_item(core, arguments, 1, &gc.message));

    PINTA_CHECK(pinta_lib_array_alloc(core, arguments_length > 1 ? 3 : 2, &gc.array));

    PINTA_CHECK(pinta_lib_array_set_item(core, &gc.array, 0, &gc.expected));
    PINTA_CHECK(pinta_lib_array_set_item(core, &gc.array, 1, &gc.buffer));

    if (arguments_length > 1)
        PINTA_CHECK(pinta_lib_array_set_item(core, &gc.array, 2, &gc.message));

    PINTA_CHECK(pinta_test_internal_assert(core, &gc.array, &gc.message));

    PINTA_CHECK(pinta_lib_buffer_alloc(core, &gc.buffer));
    core->output.reference = gc.buffer.reference;

    PINTA_GC_RETURN(core);
}

PintaException pinta_test_global_function(PintaCore *core, PintaReference *function, PintaReference *function_this, PintaReference *function_arguments, PintaReference *return_value)
{
    return pinta_lib_property_table_alloc(core, 4, return_value);
}

PINTA_TEST_BEGIN(code_array_sum, 1)
{
    PINTA_CHECK(pinta_test_execute_module(core, PINTA_TEST_FILE("array-sum.pint"), NULL));
}
PINTA_TEST_END(PINTA_OK)

PINTA_TEST_BEGIN(code_load_const, 2)
{
    PintaReference *name, *value;
    decimal decimal_value;
    decimal decimal_comparison;
    PintaModuleDomain *domain;

    name = PINTA_GC_LOCAL(0);
    value = PINTA_GC_LOCAL(1);

    PINTA_CHECK(pinta_test_execute_module(core, PINTA_TEST_FILE("load-const.pint"), &domain));

    PINTA_CHECK(pinta_lib_string_alloc_value(core, L"gnull", 5, name));
    PINTA_CHECK(pinta_lib_module_get_global_value(core, domain, name, value));

    sput_fail_if(value->reference != NULL, "gnull - Null");

    PINTA_CHECK(pinta_lib_string_alloc_value(core, L"gizero", 6, name));
    PINTA_CHECK(pinta_lib_module_get_global_value(core, domain, name, value));

    sput_fail_if(value->reference == NULL, "gizero - Not null");
    sput_fail_if(value->reference->block_kind != PINTA_KIND_INTEGER, "gizero - Integer");
    sput_fail_if(pinta_integer_ref_get_value(value) != 0, "gizero - Value 0");

    PINTA_CHECK(pinta_lib_string_alloc_value(core, L"gione", 5, name));
    PINTA_CHECK(pinta_lib_module_get_global_value(core, domain, name, value));

    sput_fail_if(value->reference == NULL, "gione - Not null");
    sput_fail_if(value->reference->block_kind != PINTA_KIND_INTEGER, "gione - Integer");
    sput_fail_if(pinta_integer_ref_get_value(value) != 1, "gione - Value 1");

    PINTA_CHECK(pinta_lib_string_alloc_value(core, L"gimpi", 5, name));
    PINTA_CHECK(pinta_lib_module_get_global_value(core, domain, name, value));

    sput_fail_if(value->reference == NULL, "gimpi - Not null");
    sput_fail_if(value->reference->block_kind != PINTA_KIND_INTEGER, "gimpi - Integer");
    sput_fail_if(pinta_integer_ref_get_value(value) != -31415, "gimpi - Value -31415");

    PINTA_CHECK(pinta_lib_string_alloc_value(core, L"gdone", 5, name));
    PINTA_CHECK(pinta_lib_module_get_global_value(core, domain, name, value));

    decimal_value = decimal_from32(1);

    sput_fail_if(value->reference == NULL, "gdone - Not null");
    sput_fail_if(value->reference->block_kind != PINTA_KIND_DECIMAL, "gdone - Decimal");

    decimal_comparison = (decimal)decimal_compare(decimal_value, pinta_decimal_ref_get_value(value));
    sput_fail_if(decimal_is_zero(decimal_comparison) == 0, "gdone - Value 1");

    PINTA_CHECK(pinta_lib_string_alloc_value(core, L"gdzero", 6, name));
    PINTA_CHECK(pinta_lib_module_get_global_value(core, domain, name, value));

    sput_fail_if(value->reference == NULL, "gdzero - Not null");
    sput_fail_if(value->reference->block_kind != PINTA_KIND_DECIMAL, "gdzero - Decimal");
    sput_fail_if(decimal_is_zero(pinta_decimal_ref_get_value(value)) == 0, "gdzero - Value 0");

    PINTA_CHECK(pinta_lib_string_alloc_value(core, L"gstring1", 8, name));
    PINTA_CHECK(pinta_lib_module_get_global_value(core, domain, name, value));

    sput_fail_if(value->reference == NULL, "gstring1 - Not null");
    sput_fail_if(value->reference->block_kind != PINTA_KIND_STRING, "gstring1 - String");
    sput_fail_if(pinta_string_ref_get_length(value) != 1, "gstring1 - Length 1");
    sput_fail_if(memcmp("1", pinta_string_ref_get_data(value), pinta_string_ref_get_length(value) * sizeof(wchar)) != 0, "gstring1 - Value '1'");

    PINTA_CHECK(pinta_lib_string_alloc_value(core, L"gstringLong", 11, name));
    PINTA_CHECK(pinta_lib_module_get_global_value(core, domain, name, value));

    sput_fail_if(value->reference == NULL, "gstringLong - Not null");
    sput_fail_if(value->reference->block_kind != PINTA_KIND_STRING, "gstringLong - String");
    sput_fail_if(pinta_string_ref_get_length(value) != 324, "gstringLong - Length 324");
    sput_fail_if(memcmp(L"A very very very long string, that is extremely long. Well I am lying a bit. Well... A lot. This is not that long, but still good enough. I guess... I might need like a kilobyte string or even longer.\nMaybe I should add special function to read file? What I am talking (writing) about? There are no special functions yet ;))", pinta_string_ref_get_data(value), pinta_string_ref_get_length(value) * sizeof(wchar)) != 0, "gstringLong - Value 'A very very...'");
}
PINTA_TEST_END(PINTA_OK)

PINTA_TEST_BEGIN(code_simple_substring, 2)
{
    PintaReference *name, *value;
    PintaModuleDomain *domain;

    name = PINTA_GC_LOCAL(0);
    value = PINTA_GC_LOCAL(1);

    PINTA_CHECK(pinta_test_execute_module(core, PINTA_TEST_FILE("simple-substring.pint"), &domain));

    PINTA_CHECK(pinta_lib_string_alloc_value(core, L"empty", 5, name));
    PINTA_CHECK(pinta_lib_module_get_global_value(core, domain, name, value));

    sput_fail_if(value->reference == NULL, "empty - Not null");
    sput_fail_if(value->reference->block_kind != PINTA_KIND_STRING && value->reference->block_kind != PINTA_KIND_SUBSTRING, "empty - String or Substring");

    if (value->reference->block_kind == PINTA_KIND_STRING)
        sput_fail_if(pinta_string_ref_get_length(value) != 0, "empty - Length 0");
    else
        sput_fail_if(pinta_substring_ref_get_length(value) != 0, "empty - Length 0");

    PINTA_CHECK(pinta_lib_string_alloc_value(core, L"whole", 5, name));
    PINTA_CHECK(pinta_lib_module_get_global_value(core, domain, name, value));

    sput_fail_if(value->reference == NULL, "whole - Not null");
    sput_fail_if(value->reference->block_kind != PINTA_KIND_STRING && value->reference->block_kind != PINTA_KIND_SUBSTRING, "whole - String or Substring");

    if (value->reference->block_kind == PINTA_KIND_STRING)
    {
        sput_fail_if(pinta_string_ref_get_length(value) != 10, "whole - Length 10");
    }
    else
    {
        sput_fail_if(pinta_substring_ref_get_length(value) != 10, "whole - Length 10");
        PINTA_CHECK(pinta_lib_string_to_string(core, value, value));
    }

    sput_fail_if(memcmp(pinta_string_ref_get_data(value), L"qwertyuiop", 10 * sizeof(wchar)) != 0, "whole - Correct value");

    PINTA_CHECK(pinta_lib_string_alloc_value(core, L"fr", 2, name));
    PINTA_CHECK(pinta_lib_module_get_global_value(core, domain, name, value));

    sput_fail_if(value->reference == NULL, "fr - Not null");
    sput_fail_if(value->reference->block_kind != PINTA_KIND_STRING && value->reference->block_kind != PINTA_KIND_SUBSTRING, "fr - String or Substring");

    if (value->reference->block_kind == PINTA_KIND_STRING)
    {
        sput_fail_if(pinta_string_ref_get_length(value) != 3, "fr - Length 3");
    }
    else
    {
        sput_fail_if(pinta_substring_ref_get_length(value) != 3, "fr - Length 3");
        PINTA_CHECK(pinta_lib_string_to_string(core, value, value));
    }

    sput_fail_if(memcmp(pinta_string_ref_get_data(value), L"rty", 3 * sizeof(wchar)) != 0, "fr - Correct value");
}
PINTA_TEST_END(PINTA_OK)

PINTA_TEST_BEGIN(code_internal_function, 1)
{
    core->internal_functions = (PintaCoreInternalFunction*)pinta_memory_alloc(core->memory, 3 * sizeof(PintaCoreInternalFunction));
    core->internal_functions_length = 3;

    core->internal_functions[0] = pinta_test_internal_integer_eq_6;
    core->internal_functions[1] = pinta_test_internal_strings_eq_hi;
    core->internal_functions[2] = pinta_test_internal_decimal_eq_pi;

    PINTA_CHECK(pinta_test_execute_module(core, PINTA_TEST_FILE("simple-internal-function.pint"), NULL));
}
PINTA_TEST_END(PINTA_OK)

PINTA_TEST_BEGIN(code_simple_script, 2)
{
    PintaModuleDomain *domain;
    PintaReference *name = PINTA_GC_LOCAL(0);
    PintaReference *value = PINTA_GC_LOCAL(1);

    core->internal_functions = (PintaCoreInternalFunction*)pinta_memory_alloc(core->memory, 1 * sizeof(PintaCoreInternalFunction));
    core->internal_functions_length = 1;

    core->internal_functions[0] = pinta_test_internal_assert;

    PINTA_CHECK(pinta_lib_string_alloc_value(core, L"name", 4, name));
    PINTA_CHECK(pinta_lib_string_alloc_value(core, L"World", 5, value));

    PINTA_CHECK(pinta_test_load_module(core, PINTA_TEST_FILE("simple-script.pint"), &domain));
    PINTA_CHECK(pinta_lib_module_set_global_value(core, domain, name, value));
    PINTA_CHECK(pinta_code_execute_module(core, domain));
}
PINTA_TEST_END(PINTA_OK)

PINTA_TEST_BEGIN(code_simple_for_loop, 1)
{
    PintaModuleDomain *domain;
    core->internal_functions = (PintaCoreInternalFunction*)pinta_memory_alloc(core->memory, 1 * sizeof(PintaCoreInternalFunction));
    core->internal_functions_length = 1;

    core->internal_functions[0] = pinta_test_internal_assert;

    PINTA_CHECK(pinta_test_load_module(core, PINTA_TEST_FILE("simple-for-loop.pint"), &domain));
    PINTA_CHECK(pinta_code_execute_module(core, domain));
}
PINTA_TEST_END(PINTA_OK)

PINTA_TEST_BEGIN(code_operations, 1)
{
    PintaModuleDomain *domain;
    core->internal_functions = (PintaCoreInternalFunction*)pinta_memory_alloc(core->memory, 1 * sizeof(PintaCoreInternalFunction));
    core->internal_functions_length = 1;

    core->internal_functions[0] = pinta_test_internal_assert;

    PINTA_CHECK(pinta_test_load_module(core, PINTA_TEST_FILE("operations-script.pint"), &domain));
    PINTA_CHECK(pinta_code_execute_module(core, domain));
}
PINTA_TEST_END(PINTA_OK)

PINTA_TEST_BEGIN(code_exit_on_global_return, 1)
{
    PintaModuleDomain *domain;
    core->internal_functions = (PintaCoreInternalFunction*)pinta_memory_alloc(core->memory, 1 * sizeof(PintaCoreInternalFunction));
    core->internal_functions_length = 1;

    core->internal_functions[0] = pinta_test_internal_unreachable;

    PINTA_CHECK(pinta_test_load_module(core, PINTA_TEST_FILE("exit-on-global-return.pint"), &domain));
    PINTA_CHECK(pinta_code_execute_module(core, domain));
}
PINTA_TEST_END(PINTA_OK)

PINTA_TEST_BEGIN(code_unicode_strings_are_correct, 2)
{
    PintaModuleDomain *domain;
    PintaReference *value = PINTA_GC_LOCAL(0);
    PintaReference *name = PINTA_GC_LOCAL(1);

    wchar expected_word[] = { 260, 382, 117, 111, 108, 97, 115 };

    core->internal_functions = (PintaCoreInternalFunction*)pinta_memory_alloc(core->memory, 1 * sizeof(PintaCoreInternalFunction));
    core->internal_functions_length = 1;

    core->internal_functions[0] = pinta_test_internal_unreachable;

    PINTA_CHECK(pinta_test_load_module(core, PINTA_TEST_FILE("unicode-string.pint"), &domain));
    PINTA_CHECK(pinta_code_execute_module(core, domain));

    PINTA_CHECK(pinta_lib_string_alloc_value(core, L"WORD", 4, name));
    PINTA_CHECK(pinta_lib_module_get_global_value(core, domain, name, value));

    sput_fail_if(value->reference == NULL, "WORD - not null");
    sput_fail_if(value->reference->block_kind != PINTA_KIND_STRING, "WORD - String");

    sput_fail_if(pinta_string_ref_get_length(value) != 7, "WORD - Length 7");

    sput_fail_if(memcmp(pinta_string_ref_get_data(value), expected_word, sizeof(expected_word)) != 0, "WORD - Correct value");
}
PINTA_TEST_END(PINTA_OK)

PINTA_TEST_BEGIN(code_unicode_outline, 2)
{
    PintaModuleDomain *domain;
    PintaReference *blob = PINTA_GC_LOCAL(1);

    core->internal_functions = (PintaCoreInternalFunction*)pinta_memory_alloc(core->memory, 1 * sizeof(PintaCoreInternalFunction));
    core->internal_functions_length = 1;

    core->internal_functions[0] = pinta_api_internal_out;

    PINTA_CHECK(pinta_test_load_module(core, PINTA_TEST_FILE("unicode-outline.pint"), &domain));
    PINTA_CHECK(pinta_code_execute_module(core, domain));

    PINTA_CHECK(pinta_lib_buffer_get_blob(core, &core->output, blob));

    sput_fail_if(blob->reference == NULL, "NULL blob");

    wchar *expected = L"Ąžuolų";
    sput_fail_if(pinta_buffer_ref_get_length(&core->output) != wcslen(expected) * sizeof(wchar), "Correct length");
    sput_fail_if(memcmp(pinta_blob_ref_get_data(blob), expected, pinta_buffer_ref_get_length(&core->output)) != 0, "Correct data");
}
PINTA_TEST_END(PINTA_OK)

PINTA_TEST_BEGIN(code_sub_buffer_outline, 1)
{
    PintaModuleDomain *domain;

    core->internal_functions = (PintaCoreInternalFunction*)pinta_memory_alloc(core->memory, 2 * sizeof(PintaCoreInternalFunction));
    core->internal_functions_length = 2;

    core->internal_functions[0] = pinta_api_internal_out;
    core->internal_functions[1] = pinta_test_expect_buffer;

    PINTA_CHECK(pinta_test_load_module(core, PINTA_TEST_FILE("sub-buffer.pint"), &domain));
    PINTA_CHECK(pinta_code_execute_module(core, domain));
}
PINTA_TEST_END(PINTA_OK)

PINTA_TEST_BEGIN(code_string_operations_with_globals, 3)
{
    PintaModuleDomain *domain;
    PintaReference *a = PINTA_GC_LOCAL(0);
    PintaReference *b = PINTA_GC_LOCAL(1);
    PintaReference *name = PINTA_GC_LOCAL(2);

    core->internal_functions = (PintaCoreInternalFunction*)pinta_memory_alloc(core->memory, 1 * sizeof(PintaCoreInternalFunction));
    core->internal_functions_length = 1;

    core->internal_functions[0] = pinta_test_internal_assert;

    PINTA_CHECK(pinta_test_load_module(core, PINTA_TEST_FILE("string-operations-with-global-variables.pint"), &domain));

    PINTA_CHECK(pinta_lib_string_alloc_value(core, L"a", 1, name));
    PINTA_CHECK(pinta_lib_string_alloc_value(core, L"12.9", 4, a));
    PINTA_CHECK(pinta_lib_module_set_global_value(core, domain, name, a));

    PINTA_CHECK(pinta_lib_string_alloc_value(core, L"b", 1, name));
    PINTA_CHECK(pinta_lib_string_alloc_value(core, L"9.4", 3, b));
    PINTA_CHECK(pinta_lib_module_set_global_value(core, domain, name, b));

    a->reference = NULL;
    b->reference = NULL;
    name->reference = NULL;

    PINTA_CHECK(pinta_code_execute_module(core, domain));
}
PINTA_TEST_END(PINTA_OK)

PINTA_TEST_BEGIN(code_subtract_two_strings, 1)
{
    PintaModuleDomain *domain;
    core->internal_functions = (PintaCoreInternalFunction*)pinta_memory_alloc(core->memory, 1 * sizeof(PintaCoreInternalFunction));
    core->internal_functions_length = 1;

    core->internal_functions[0] = pinta_test_internal_assert;

    PINTA_CHECK(pinta_test_load_module(core, PINTA_TEST_FILE("subtract-two-strings.pint"), &domain));
    PINTA_CHECK(pinta_code_execute_module(core, domain));
}
PINTA_TEST_END(PINTA_OK)

PINTA_TEST_BEGIN(code_compare_numbers, 1)
{
    PintaModuleDomain *domain;
    core->internal_functions = (PintaCoreInternalFunction*)pinta_memory_alloc(core->memory, 1 * sizeof(PintaCoreInternalFunction));
    core->internal_functions_length = 1;

    core->internal_functions[0] = pinta_test_internal_assert;

    PINTA_CHECK(pinta_test_load_module(core, PINTA_TEST_FILE("compare-numbers.pint"), &domain));
    PINTA_CHECK(pinta_code_execute_module(core, domain));
}
PINTA_TEST_END(PINTA_OK)

void pinta_tests_code()
{
    sput_enter_suite("Code tests");
    sput_run_test(code_array_sum);
    sput_run_test(code_load_const);
    sput_run_test(code_simple_substring);
    sput_run_test(code_internal_function);
    sput_run_test(code_simple_script);
    sput_run_test(code_simple_for_loop);
    sput_run_test(code_operations);
    sput_run_test(code_exit_on_global_return);
    sput_run_test(code_unicode_strings_are_correct);
    sput_run_test(code_unicode_outline);
    sput_run_test(code_sub_buffer_outline);
    sput_run_test(code_string_operations_with_globals);
    sput_run_test(code_subtract_two_strings);
    sput_run_test(code_compare_numbers);
    sput_leave_suite();
}
