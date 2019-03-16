#include "pinta_tests.h"

#define PINTA_TEST_FILE(name)   (L"..\\Marius.Pinta.Test.Files\\" L##name)

PINTA_TEST_BEGIN(pattern_simple_tests, 2)
{
    PintaReference *name, *value;
    PintaModuleDomain *domain;
    i32 result_value;

    name = PINTA_GC_LOCAL(0);
    value = PINTA_GC_LOCAL(1);

    PINTA_CHECK(pinta_test_execute_module(core, PINTA_TEST_FILE("pattern-simple-v2.pint"), &domain));

    PINTA_CHECK(pinta_lib_string_alloc_value(core, L"result", 6, name));
    PINTA_CHECK(pinta_lib_module_get_global_value(core, domain, name, value));
   
    sput_fail_if(value->reference == NULL, "result - Not null");
    sput_fail_if(value->reference->block_kind != PINTA_KIND_INTEGER && value->reference->block_kind != PINTA_KIND_DECIMAL, "result - integer or decimal");

    if (value->reference->block_kind == PINTA_KIND_DECIMAL)
        PINTA_CHECK(pinta_lib_decimal_to_int32_value(core, value, &result_value));
    else
        result_value = pinta_integer_ref_get_value(value);

    sput_fail_if(result_value != 1, "result - Must be 1");
}
PINTA_TEST_END(PINTA_OK)

PINTA_TEST_BEGIN(pattern_pan_tests, 2)
{
    PintaReference *name, *value;
    PintaModuleDomain *domain;
    i32 result_value;

    name = PINTA_GC_LOCAL(0);
    value = PINTA_GC_LOCAL(1);

    PINTA_CHECK(pinta_test_execute_module(core, PINTA_TEST_FILE("pattern-pan-v2.pint"), &domain));

    PINTA_CHECK(pinta_lib_string_alloc_value(core, L"result", 6, name));
    PINTA_CHECK(pinta_lib_module_get_global_value(core, domain, name, value));
   
    sput_fail_if(value->reference == NULL, "result - Not null");
    sput_fail_if(value->reference->block_kind != PINTA_KIND_INTEGER && value->reference->block_kind != PINTA_KIND_DECIMAL, "result - integer or decimal");

    if (value->reference->block_kind == PINTA_KIND_DECIMAL)
        PINTA_CHECK(pinta_lib_decimal_to_int32_value(core, value, &result_value));
    else
        result_value = pinta_integer_ref_get_value(value);

    sput_fail_if(result_value != 1, "result - Must be 1");
}
PINTA_TEST_END(PINTA_OK)

void pinta_tests_pattern()
{
    sput_enter_suite("Pattern tests");
    sput_run_test(pattern_simple_tests);
    sput_run_test(pattern_pan_tests);
    sput_leave_suite();
}