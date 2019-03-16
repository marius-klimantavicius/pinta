#include "pinta_tests.h"

PintaException pinta_test_object_const_get(PintaCore *core, PintaThread *thread, PintaReference *function, PintaReference *function_this, PintaReference *function_arguments, u8 *discard_return_value, PintaReference *return_value)
{
    PINTA_UNUSED(thread);
    PINTA_UNUSED(function);
    PINTA_UNUSED(function_this);
    PINTA_UNUSED(function_arguments);

    return pinta_lib_integer_alloc_value(core, -42, return_value);
}

PintaException pinta_test_property_native(PintaCore *core, PintaReference *object, PintaReference *name, u32 native_token, u8 is_set, PintaReference *value)
{
    PintaException exception = PINTA_OK;

    if (is_set)
        PINTA_THROW(PINTA_EXCEPTION_INVALID_OPERATION);

    PINTA_CHECK(pinta_lib_integer_alloc_value(core, -(i32)(native_token * 2), value));

PINTA_EXIT:
    return PINTA_OK;
}

PINTA_TEST_BEGIN(object_should_set_and_get_single_value, 4)
{
    u32 alive_count;
    PintaReference *object;
    PintaReference *name;
    PintaReference *value;
    PintaReference *setter;
    u8 is_accessor;

    object = PINTA_GC_LOCAL(0);
    name = PINTA_GC_LOCAL(1);
    value = PINTA_GC_LOCAL(2);
    setter = PINTA_GC_LOCAL(3);

    PINTA_CHECK(pinta_lib_object_alloc(core, NULL, NULL, object));
    PINTA_CHECK(pinta_lib_string_alloc_value(core, L"foo", 3, name));
    PINTA_CHECK(pinta_lib_integer_alloc_value(core, 42, value));
    PINTA_CHECK(pinta_lib_object_set_member(core, object, name, value, &is_accessor, setter));

    value->reference = NULL;
    name->reference = NULL;

    pinta_core_gc(core, 1);

    alive_count = pinta_debug_get_alive_count(core->heap);
    sput_fail_if(alive_count != 3, "Correct #alive objects after gc"); // integer is cached

    PINTA_CHECK(pinta_lib_string_alloc_value(core, L"foo", 3, name));
    PINTA_CHECK(pinta_lib_object_get_member(core, object, name, &is_accessor, value));

    sput_fail_if(value->reference == NULL, "Failed to get o.foo, returned NULL");
    sput_fail_if(value->reference->block_kind != PINTA_KIND_INTEGER, "o.foo is no longer integer");
    sput_fail_if(pinta_integer_ref_get_value(value) != 42, pinta_tests_message(L"o.foo changed value from 42 to %d", pinta_integer_ref_get_value(value)));
}
PINTA_TEST_END(PINTA_OK)

PINTA_TEST_BEGIN(object_should_define_and_get_property, 5)
{
    u32 alive_count;
    PintaReference *object;
    PintaReference *name;
    PintaReference *value;
    PintaReference *null;
    PintaReference *arguments;
    PintaFunctionDelegate function_delegate;
    u8 is_accessor;

    object = PINTA_GC_LOCAL(0);
    name = PINTA_GC_LOCAL(1);
    value = PINTA_GC_LOCAL(2);
    null = PINTA_GC_LOCAL(3);
    arguments = PINTA_GC_LOCAL(4);

    PINTA_CHECK(pinta_lib_object_alloc(core, NULL, NULL, object));
    PINTA_CHECK(pinta_lib_string_alloc_value(core, L"foo", 3, name));
    PINTA_CHECK(pinta_lib_function_native_alloc(core, pinta_test_object_const_get, 0, NULL, NULL, NULL, NULL, value));
    PINTA_CHECK(pinta_lib_object_define_property(core, object, name, 1, 1, 0, null, value, null));

    value->reference = NULL;
    name->reference = NULL;

    pinta_core_gc(core, 1);

    alive_count = pinta_debug_get_alive_count(core->heap);
    sput_fail_if(alive_count != 4, "Correct #alive objects after gc");

    PINTA_CHECK(pinta_lib_string_alloc_value(core, L"foo", 3, name));
    PINTA_CHECK(pinta_lib_object_get_member(core, object, name, &is_accessor, value));

    sput_fail_unless(is_accessor, "Defined accessor, but get member returned value");

    if (is_accessor)
    {
        PINTA_CHECK(pinta_lib_array_alloc(core, 0, arguments));

        sput_fail_if(value->reference == NULL, "accessor function is NULL");
        sput_fail_if(value->reference->block_kind != PINTA_KIND_FUNCTION_NATIVE, "Non native function returned");
        
        function_delegate = pinta_function_native_ref_get_delegate(value);
        PINTA_CHECK(function_delegate(core, NULL, value, object, arguments, NULL, value));
    }

    sput_fail_if(value->reference == NULL, "Failed to get o.foo, returned NULL");
    sput_fail_if(value->reference->block_kind != PINTA_KIND_INTEGER, "o.foo is no longer integer");
    sput_fail_if(pinta_integer_ref_get_value(value) != -42, pinta_tests_message(L"o.foo changed value from -42 to %d", pinta_integer_ref_get_value(value)));
}
PINTA_TEST_END(PINTA_OK);

PINTA_TEST_BEGIN(object_should_define_and_get_property_native, 5)
{
    PintaReference *object;
    PintaReference *name;
    PintaReference *value;
    PintaReference *null;
    PintaReference *arguments;
    u8 is_accessor;

    object = PINTA_GC_LOCAL(0);
    name = PINTA_GC_LOCAL(1);
    value = PINTA_GC_LOCAL(2);
    null = PINTA_GC_LOCAL(3);
    arguments = PINTA_GC_LOCAL(4);

    PINTA_CHECK(pinta_lib_object_alloc(core, NULL, NULL, object));
    PINTA_CHECK(pinta_lib_string_alloc_value(core, L"foo", 3, name));
    PINTA_CHECK(pinta_lib_object_define_property_native(core, object, name, 1, 1, pinta_test_property_native, 21));

    value->reference = NULL;
    name->reference = NULL;

    pinta_core_gc(core, 1);

    PINTA_CHECK(pinta_lib_string_alloc_value(core, L"foo", 3, name));
    PINTA_CHECK(pinta_lib_object_get_member(core, object, name, &is_accessor, value));

    sput_fail_if(is_accessor, "Defined native property, but get member returned accessor");

    sput_fail_if(value->reference == NULL, "Failed to get o.foo, returned NULL");
    sput_fail_if(value->reference->block_kind != PINTA_KIND_INTEGER, "o.foo is no longer integer");
    sput_fail_if(pinta_integer_ref_get_value(value) != -42, pinta_tests_message(L"o.foo changed value from -42 to %d", pinta_integer_ref_get_value(value)));
}
PINTA_TEST_END(PINTA_OK);

void pinta_tests_object()
{
    sput_enter_suite("Object tests");
    sput_run_test(object_should_set_and_get_single_value);
    sput_run_test(object_should_define_and_get_property);
    sput_run_test(object_should_define_and_get_property_native);
    sput_leave_suite();
}
