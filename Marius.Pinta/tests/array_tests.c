#include "pinta_tests.h"

PINTA_TEST_BEGIN(array_data_should_move_in_compaction, 1)
{
    PintaReference *value;

    value = PINTA_GC_LOCAL(0);

    PINTA_CHECK(pinta_lib_string_alloc(core, 131, value));
    PINTA_CHECK(pinta_lib_array_alloc(core, 10, value));

    pinta_core_gc(core, 1);

    sput_fail_if(pinta_array_ref_get_data(value) != (PintaHeapObject**)(void*)&value->reference[1], "Array data was moved");
}
PINTA_TEST_END(PINTA_OK)

PINTA_TEST_BEGIN(array_elements_are_gc_roots, 3)
{
    u32 alive_count;
    PintaReference *array;
    PintaReference *value;
    PintaReference *value1;

    array = PINTA_GC_LOCAL(0);
    value = PINTA_GC_LOCAL(1);
    value1 = PINTA_GC_LOCAL(2);

    PINTA_CHECK(pinta_lib_array_alloc(core, 10, array));
    PINTA_CHECK(pinta_lib_string_alloc(core, 41, value));

    pinta_lib_array_set_item(core, array, 5, value);

    PINTA_CHECK(pinta_lib_array_alloc(core, 2, value));
    PINTA_CHECK(pinta_lib_decimal_alloc(core, value1));

    pinta_lib_array_set_item(core, array, 1, value);
    pinta_lib_array_set_item(core, value, 0, value1);

    alive_count = pinta_debug_get_alive_count(core->heap);
    sput_fail_if(alive_count != 4, "Correct #alive objects before gc");

    pinta_core_gc(core, 1);

    alive_count = pinta_debug_get_alive_count(core->heap);
    sput_fail_if(alive_count != 4, "Correct #alive objects after gc");
}
PINTA_TEST_END(PINTA_OK)

void pinta_tests_array()
{
    sput_enter_suite("Array tests");
    sput_run_test(array_data_should_move_in_compaction);
    sput_run_test(array_elements_are_gc_roots);
    sput_leave_suite();
}
