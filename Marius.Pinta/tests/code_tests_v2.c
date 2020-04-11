#include "pinta_tests.h"

#define PINTA_TEST_FILE(name)   (L"..\\Marius.Pinta.Test.Files\\" L##name)

PINTA_TEST_BEGIN(code_array_sum_v2, 1)
{
    PINTA_CHECK(pinta_test_execute_module(core, PINTA_TEST_FILE("array-sum-v2.pint"), NULL));
}
PINTA_TEST_END(PINTA_OK)

PINTA_TEST_BEGIN(code_load_const_v2, 2)
{
    PintaReference *name, *value;
    decimal decimal_value;
    decimal decimal_comparison;
    PintaModuleDomain *domain;

    name = PINTA_GC_LOCAL(0);
    value = PINTA_GC_LOCAL(1);

    PINTA_CHECK(pinta_test_execute_module(core, PINTA_TEST_FILE("load-const-v2.pint"), &domain));

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

PINTA_TEST_BEGIN(code_simple_substring_v2, 2)
{
    PintaReference *name, *value;
    PintaModuleDomain *domain;

    name = PINTA_GC_LOCAL(0);
    value = PINTA_GC_LOCAL(1);

    PINTA_CHECK(pinta_test_execute_module(core, PINTA_TEST_FILE("simple-substring-v2.pint"), &domain));

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

PINTA_TEST_BEGIN(code_internal_function_v2, 1)
{
    core->internal_functions = (PintaCoreInternalFunction*)pinta_memory_alloc(core->memory, 3 * sizeof(PintaCoreInternalFunction));
    core->internal_functions_length = 3;

    core->internal_functions[0] = pinta_test_internal_integer_eq_6;
    core->internal_functions[1] = pinta_test_internal_strings_eq_hi;
    core->internal_functions[2] = pinta_test_internal_decimal_eq_pi;

    PINTA_CHECK(pinta_test_execute_module(core, PINTA_TEST_FILE("simple-internal-function-v2.pint"), NULL));
}
PINTA_TEST_END(PINTA_OK)

PINTA_TEST_BEGIN(code_simple_script_v2, 2)
{
    PintaModuleDomain *domain;
    PintaReference *name = PINTA_GC_LOCAL(0);
    PintaReference *value = PINTA_GC_LOCAL(1);

    core->internal_functions = (PintaCoreInternalFunction*)pinta_memory_alloc(core->memory, 1 * sizeof(PintaCoreInternalFunction));
    core->internal_functions_length = 1;

    core->internal_functions[0] = pinta_test_internal_assert;

    PINTA_CHECK(pinta_lib_string_alloc_value(core, L"name", 4, name));
    PINTA_CHECK(pinta_lib_string_alloc_value(core, L"World", 5, value));

    PINTA_CHECK(pinta_test_load_module(core, PINTA_TEST_FILE("simple-script-v2.pint"), &domain));
    PINTA_CHECK(pinta_lib_module_set_global_value(core, domain, name, value));
    PINTA_CHECK(pinta_code_execute_module(core, domain));
}
PINTA_TEST_END(PINTA_OK)

PINTA_TEST_BEGIN(code_script_closure_v2, 2)
{
    PintaModuleDomain *domain;
    PintaReference *name = PINTA_GC_LOCAL(0);
    PintaReference *value = PINTA_GC_LOCAL(1);

    core->internal_functions = (PintaCoreInternalFunction*)pinta_memory_alloc(core->memory, 1 * sizeof(PintaCoreInternalFunction));
    core->internal_functions_length = 1;

    core->internal_functions[0] = pinta_test_internal_assert;

    PINTA_CHECK(pinta_lib_string_alloc_value(core, L"name", 4, name));
    PINTA_CHECK(pinta_lib_string_alloc_value(core, L"World", 5, value));

    PINTA_CHECK(pinta_test_load_module(core, PINTA_TEST_FILE("script-closure-v2.pint"), &domain));
    PINTA_CHECK(pinta_lib_module_set_global_value(core, domain, name, value));
    PINTA_CHECK(pinta_code_execute_module(core, domain));
}
PINTA_TEST_END(PINTA_OK)

PINTA_TEST_BEGIN(code_simple_for_loop_v2, 1)
{
    PintaModuleDomain *domain;

    core->internal_functions = (PintaCoreInternalFunction*)pinta_memory_alloc(core->memory, 1 * sizeof(PintaCoreInternalFunction));
    core->internal_functions_length = 1;

    core->internal_functions[0] = pinta_test_internal_assert;

    PINTA_CHECK(pinta_test_load_module(core, PINTA_TEST_FILE("simple-for-loop-v2.pint"), &domain));
    PINTA_CHECK(pinta_code_execute_module(core, domain));
}
PINTA_TEST_END(PINTA_OK)

PINTA_TEST_BEGIN(code_operations_v2, 1)
{
    PintaModuleDomain *domain;

    core->internal_functions = (PintaCoreInternalFunction*)pinta_memory_alloc(core->memory, 1 * sizeof(PintaCoreInternalFunction));
    core->internal_functions_length = 1;

    core->internal_functions[0] = pinta_test_internal_assert;

    PINTA_CHECK(pinta_test_load_module(core, PINTA_TEST_FILE("operations-script-v2.pint"), &domain));
    PINTA_CHECK(pinta_code_execute_module(core, domain));
}
PINTA_TEST_END(PINTA_OK)

PINTA_TEST_BEGIN(code_exit_on_global_return_v2, 1)
{
    PintaModuleDomain *domain;

    core->internal_functions = (PintaCoreInternalFunction*)pinta_memory_alloc(core->memory, 1 * sizeof(PintaCoreInternalFunction));
    core->internal_functions_length = 1;

    core->internal_functions[0] = pinta_test_internal_unreachable;

    PINTA_CHECK(pinta_test_load_module(core, PINTA_TEST_FILE("exit-on-global-return-v2.pint"), &domain));
    PINTA_CHECK(pinta_code_execute_module(core, domain));
}
PINTA_TEST_END(PINTA_OK)

PINTA_TEST_BEGIN(code_unicode_strings_are_correct_v2, 2)
{
    PintaModuleDomain *domain;
    PintaReference *value = PINTA_GC_LOCAL(0);
    PintaReference *name = PINTA_GC_LOCAL(1);

    wchar expected_word[] = { 260, 382, 117, 111, 108, 97, 115 };
    wchar expected_georgian[] = { 4314, 4304, 4318, 4304, 4320, 4304, 4313, 4317, 4305, 4321, 32, 4304, 4325, 32, 4309, 4312, 4316, 4315, 4308, 32, 4312, 4316, 4306, 4314, 4312, 4321, 4323, 4320, 4321 };
    wchar expected_all[] = { 260, 268, 280, 278, 302, 352, 370, 362, 261, 269, 281, 279, 303, 353, 371, 363 };

    core->internal_functions = (PintaCoreInternalFunction*)pinta_memory_alloc(core->memory, 1 * sizeof(PintaCoreInternalFunction));
    core->internal_functions_length = 1;

    core->internal_functions[0] = pinta_test_internal_unreachable;

    PINTA_CHECK(pinta_test_load_module(core, PINTA_TEST_FILE("unicode-string-v2.pint"), &domain));
    PINTA_CHECK(pinta_code_execute_module(core, domain));

    PINTA_CHECK(pinta_lib_string_alloc_value(core, L"WORD", 4, name));
    PINTA_CHECK(pinta_lib_module_get_global_value(core, domain, name, value));

    sput_fail_if(value->reference == NULL, "WORD - not null");
    sput_fail_if(value->reference->block_kind != PINTA_KIND_STRING, "WORD - String");

    sput_fail_if(pinta_string_ref_get_length(value) != 7, "WORD - Length 7");

    sput_fail_if(memcmp(pinta_string_ref_get_data(value), expected_word, sizeof(expected_word)) != 0, "WORD - Correct value");

    PINTA_CHECK(pinta_lib_string_alloc_value(core, L"GEORGIAN", 8, name));
    PINTA_CHECK(pinta_lib_module_get_global_value(core, domain, name, value));

    sput_fail_if(value->reference == NULL, "GEORGIAN - not null");
    sput_fail_if(value->reference->block_kind != PINTA_KIND_STRING, "GEORGIAN - String");

    sput_fail_if(memcmp(pinta_string_ref_get_data(value), expected_georgian, sizeof(expected_georgian)) != 0, "GEORGIAN - Correct value");

    PINTA_CHECK(pinta_lib_string_alloc_value(core, L"ALL", 3, name));
    PINTA_CHECK(pinta_lib_module_get_global_value(core, domain, name, value));

    sput_fail_if(value->reference == NULL, "ALL - not null");
    sput_fail_if(value->reference->block_kind != PINTA_KIND_STRING, "ALL - String");

    sput_fail_if(memcmp(pinta_string_ref_get_data(value), expected_all, sizeof(expected_all)) != 0, "ALL - Correct value");
}
PINTA_TEST_END(PINTA_OK)

PINTA_TEST_BEGIN(code_unicode_outline_v2, 2)
{
    PintaModuleDomain *domain;
    PintaReference *buffer = PINTA_GC_LOCAL(0);
    PintaReference *blob = PINTA_GC_LOCAL(1);

    core->internal_functions = (PintaCoreInternalFunction*)pinta_memory_alloc(core->memory, 1 * sizeof(PintaCoreInternalFunction));
    core->internal_functions_length = 1;

    core->internal_functions[0] = pinta_api_internal_out;

    PINTA_CHECK(pinta_test_load_module(core, PINTA_TEST_FILE("unicode-outline-v2.pint"), &domain));
    PINTA_CHECK(pinta_code_execute_module(core, domain));

    buffer->reference = core->output.reference;
    PINTA_CHECK(pinta_lib_buffer_get_blob(core, buffer, blob));

    sput_fail_if(blob->reference == NULL, "NULL blob");

    wchar *expected = L"Ąžuolų";
    sput_fail_if(pinta_buffer_ref_get_length(buffer) != wcslen(expected) * sizeof(wchar), "Correct length");
    sput_fail_if(memcmp(pinta_blob_ref_get_data(blob), expected, pinta_buffer_ref_get_length(buffer)) != 0, "Correct data");
}
PINTA_TEST_END(PINTA_OK)

PINTA_TEST_BEGIN(code_sub_buffer_outline_v2, 1)
{
    PintaModuleDomain *domain;

    core->internal_functions = (PintaCoreInternalFunction*)pinta_memory_alloc(core->memory, 2 * sizeof(PintaCoreInternalFunction));
    core->internal_functions_length = 2;

    core->internal_functions[0] = pinta_api_internal_out;
    core->internal_functions[1] = pinta_test_expect_buffer;

    PINTA_CHECK(pinta_test_load_module(core, PINTA_TEST_FILE("sub-buffer-v2.pint"), &domain));
    PINTA_CHECK(pinta_code_execute_module(core, domain));
}
PINTA_TEST_END(PINTA_OK)

PINTA_TEST_BEGIN(code_string_operations_with_globals_v2, 3)
{
    PintaModuleDomain *domain;
    PintaReference *a = PINTA_GC_LOCAL(0);
    PintaReference *b = PINTA_GC_LOCAL(1);
    PintaReference *name = PINTA_GC_LOCAL(2);

    core->internal_functions = (PintaCoreInternalFunction*)pinta_memory_alloc(core->memory, 1 * sizeof(PintaCoreInternalFunction));
    core->internal_functions_length = 1;

    core->internal_functions[0] = pinta_test_internal_assert;

    PINTA_CHECK(pinta_test_load_module(core, PINTA_TEST_FILE("string-operations-with-global-variables-v2.pint"), &domain));

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

PINTA_TEST_BEGIN(code_new_function_v2, 1)
{
    PintaModuleDomain *domain;
    core->internal_functions = (PintaCoreInternalFunction*)pinta_memory_alloc(core->memory, 1 * sizeof(PintaCoreInternalFunction));
    core->internal_functions_length = 1;

    core->internal_functions[0] = pinta_test_internal_assert;

    PINTA_CHECK(pinta_test_load_module(core, PINTA_TEST_FILE("new-function-v2.pint"), &domain));
    PINTA_CHECK(pinta_code_execute_module(core, domain));
}
PINTA_TEST_END(PINTA_OK)

PINTA_TEST_BEGIN(code_construct_v2, 1)
{
    PintaModuleDomain *domain;
    core->internal_functions = (PintaCoreInternalFunction*)pinta_memory_alloc(core->memory, 1 * sizeof(PintaCoreInternalFunction));
    core->internal_functions_length = 1;

    core->internal_functions[0] = pinta_test_internal_assert;

    PINTA_CHECK(pinta_test_load_module(core, PINTA_TEST_FILE("construct-v2.pint"), &domain));
    PINTA_CHECK(pinta_code_execute_module(core, domain));
}
PINTA_TEST_END(PINTA_OK)

PINTA_TEST_BEGIN(code_object_literal_v2, 1)
{
    PintaModuleDomain *domain;
    core->internal_functions = (PintaCoreInternalFunction*)pinta_memory_alloc(core->memory, 1 * sizeof(PintaCoreInternalFunction));
    core->internal_functions_length = 1;

    core->internal_functions[0] = pinta_test_internal_assert;

    PINTA_CHECK(pinta_test_load_module(core, PINTA_TEST_FILE("object-literal-v2.pint"), &domain));
    PINTA_CHECK(pinta_code_execute_module(core, domain));
}
PINTA_TEST_END(PINTA_OK)

PINTA_TEST_BEGIN(code_globals_properties_v2, 1)
{
    PintaModuleDomain *domain;
    core->internal_functions = (PintaCoreInternalFunction*)pinta_memory_alloc(core->memory, 1 * sizeof(PintaCoreInternalFunction));
    core->internal_functions_length = 1;

    core->internal_functions[0] = pinta_test_internal_assert;

    PINTA_CHECK(pinta_test_load_module(core, PINTA_TEST_FILE("globals-properties-v2.pint"), &domain));
    PINTA_CHECK(pinta_code_execute_module(core, domain));
}
PINTA_TEST_END(PINTA_OK)

PINTA_TEST_BEGIN(code_subtract_two_strings_v2, 1)
{
    PintaModuleDomain *domain;
    core->internal_functions = (PintaCoreInternalFunction*)pinta_memory_alloc(core->memory, 1 * sizeof(PintaCoreInternalFunction));
    core->internal_functions_length = 1;

    core->internal_functions[0] = pinta_test_internal_assert;

    PINTA_CHECK(pinta_test_load_module(core, PINTA_TEST_FILE("subtract-two-strings-v2.pint"), &domain));
    PINTA_CHECK(pinta_code_execute_module(core, domain));
}
PINTA_TEST_END(PINTA_OK)

PINTA_TEST_BEGIN(code_tail_call_simple_v2, 1)
{
    PintaModuleDomain *domain;
    core->internal_functions = (PintaCoreInternalFunction*)pinta_memory_alloc(core->memory, 1 * sizeof(PintaCoreInternalFunction));
    core->internal_functions_length = 1;

    core->internal_functions[0] = pinta_test_internal_assert;

    PINTA_CHECK(pinta_test_load_module(core, PINTA_TEST_FILE("tail-call-simple.pint"), &domain));
    PINTA_CHECK(pinta_code_execute_module(core, domain));
}
PINTA_TEST_END(PINTA_OK)

PINTA_TEST_BEGIN(code_tail_call_invoke_v2, 1)
{
    PintaModuleDomain *domain;
    core->internal_functions = (PintaCoreInternalFunction*)pinta_memory_alloc(core->memory, 1 * sizeof(PintaCoreInternalFunction));
    core->internal_functions_length = 1;

    core->internal_functions[0] = pinta_test_internal_assert;

    PINTA_CHECK(pinta_test_load_module(core, PINTA_TEST_FILE("tail-call-invoke.pint"), &domain));
    PINTA_CHECK(pinta_code_execute_module(core, domain));
}
PINTA_TEST_END(PINTA_OK)

PINTA_TEST_BEGIN(code_compare_numbers_v2, 1)
{
    PintaModuleDomain *domain;
    core->internal_functions = (PintaCoreInternalFunction*)pinta_memory_alloc(core->memory, 1 * sizeof(PintaCoreInternalFunction));
    core->internal_functions_length = 1;

    core->internal_functions[0] = pinta_test_internal_assert;

    PINTA_CHECK(pinta_test_load_module(core, PINTA_TEST_FILE("compare-numbers-v2.pint"), &domain));
    PINTA_CHECK(pinta_code_execute_module(core, domain));
}
PINTA_TEST_END(PINTA_OK)

void pinta_tests_code_v2()
{
    sput_enter_suite("Code tests V2");
    sput_run_test(code_array_sum_v2);
    sput_run_test(code_load_const_v2);
    sput_run_test(code_simple_substring_v2);
    sput_run_test(code_internal_function_v2);
    sput_run_test(code_simple_script_v2);
    sput_run_test(code_script_closure_v2);
    sput_run_test(code_simple_for_loop_v2);
    sput_run_test(code_operations_v2);
    sput_run_test(code_exit_on_global_return_v2);
    sput_run_test(code_unicode_strings_are_correct_v2);
    sput_run_test(code_unicode_outline_v2);
    sput_run_test(code_sub_buffer_outline_v2);
    sput_run_test(code_string_operations_with_globals_v2);
    sput_run_test(code_new_function_v2);
    sput_run_test(code_globals_properties_v2);
    sput_run_test(code_object_literal_v2);
    sput_run_test(code_subtract_two_strings_v2);
    sput_run_test(code_tail_call_simple_v2);
    sput_run_test(code_tail_call_invoke_v2);
    sput_run_test(code_compare_numbers_v2);
    sput_leave_suite();
}
