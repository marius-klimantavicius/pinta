#include "pinta_tests.h"

#define PINTA_TEST_FILE(name)   (L"..\\Marius.Pinta.Test.Files\\" L##name)

PintaException pinta_test_call_managed(PintaCore *core, PintaThread *thread, PintaReference *function, PintaReference *function_this, PintaReference *function_arguments, u8 *discard_return_value, PintaReference *return_value)
{
    PintaException exception = PINTA_OK;
    
    pinta_assert(core != NULL);
    pinta_assert(function != NULL);
    pinta_assert(function_arguments != NULL);

    struct
    {
        PintaReference function;
    } gc;

    PINTA_GC_ENTER(core, gc);

    if (thread == NULL)
        PINTA_THROW(PINTA_EXCEPTION_INVALID_ARGUMENTS);

    PINTA_CHECK(pinta_lib_array_get_item(core, function_arguments, 0, &gc.function));
    PINTA_CHECK(pinta_lib_stack_push(thread, &gc.function));

    PINTA_CHECK(pinta_code_invoke(thread, 0, 0));

    if (discard_return_value)
        *discard_return_value = 1;

    PINTA_GC_RETURN(core);
}

PINTA_TEST_BEGIN(native_function_should_call_managed_function, 2)
{
    PintaModuleDomain *domain;
    PintaReference *name = PINTA_GC_LOCAL(0);
    PintaReference *value = PINTA_GC_LOCAL(1);

    core->internal_functions = (PintaCoreInternalFunction*)pinta_memory_alloc(core->memory, 1 * sizeof(PintaCoreInternalFunction));
    core->internal_functions_length = 1;

    core->internal_functions[0] = pinta_test_internal_assert;

    PINTA_CHECK(pinta_lib_string_alloc_value(core, L"nativeCaller", 12, name));
    PINTA_CHECK(pinta_lib_function_native_alloc(core, pinta_test_call_managed, 0, NULL, NULL, NULL, NULL, value));

    PINTA_CHECK(pinta_test_load_module(core, PINTA_TEST_FILE("native-call-managed-v2.pint"), &domain));
    PINTA_CHECK(pinta_lib_module_set_global_value(core, domain, name, value));
    PINTA_CHECK(pinta_code_execute_module(core, domain));
}
PINTA_TEST_END(PINTA_OK)

PINTA_TEST_BEGIN(require_module, 2)
{
    PintaModuleDomain *domain;
    PintaReference *name = PINTA_GC_LOCAL(0);
    PintaReference *value = PINTA_GC_LOCAL(1);

    core->internal_functions = (PintaCoreInternalFunction*)pinta_memory_alloc(core->memory, 1 * sizeof(PintaCoreInternalFunction));
    core->internal_functions_length = 1;

    core->internal_functions[0] = pinta_test_internal_assert;

    PINTA_CHECK(pinta_lib_string_alloc_value(core, L"require", 7, name));
    PINTA_CHECK(pinta_lib_function_native_alloc(core, pinta_code_rt_require, 0, NULL, NULL, NULL, NULL, value));

    PINTA_CHECK(pinta_test_load_module(core, PINTA_TEST_FILE("require-v2.pint"), &domain));
    PINTA_CHECK(pinta_lib_module_set_global_value(core, domain, name, value));
    PINTA_CHECK(pinta_code_execute_module(core, domain));
}
PINTA_TEST_END(PINTA_OK)

PINTA_TEST_BEGIN(require_module_should_return_replaced_exports, 2)
{
    PintaModuleDomain *domain;
    PintaReference *name = PINTA_GC_LOCAL(0);
    PintaReference *value = PINTA_GC_LOCAL(1);

    core->internal_functions = (PintaCoreInternalFunction*)pinta_memory_alloc(core->memory, 1 * sizeof(PintaCoreInternalFunction));
    core->internal_functions_length = 1;

    core->internal_functions[0] = pinta_test_internal_assert;

    PINTA_CHECK(pinta_lib_string_alloc_value(core, L"require", 7, name));
    PINTA_CHECK(pinta_lib_function_native_alloc(core, pinta_code_rt_require, 0, NULL, NULL, NULL, NULL, value));

    PINTA_CHECK(pinta_test_load_module(core, PINTA_TEST_FILE("require-exports-v2.pint"), &domain));
    PINTA_CHECK(pinta_lib_module_set_global_value(core, domain, name, value));
    PINTA_CHECK(pinta_code_execute_module(core, domain));
}
PINTA_TEST_END(PINTA_OK)

void pinta_tests_function()
{
    sput_enter_suite("Function tests");
    sput_run_test(native_function_should_call_managed_function);
    sput_run_test(require_module);
    sput_run_test(require_module_should_return_replaced_exports);
    sput_leave_suite();
}
