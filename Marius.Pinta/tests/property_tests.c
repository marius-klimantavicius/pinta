#include "pinta_tests.h"

PINTA_TEST_BEGIN(property_data_should_move_in_compaction, 3)
{
    PintaReference *value;
    PintaReference *table;
    PintaReference *key;

    PintaProperty *actual_table;
    PintaProperty *current;
    u32 actual_capacity;
    u32 index;
    u8 found;

    value = PINTA_GC_LOCAL(0);
    table = PINTA_GC_LOCAL(1);
    key = PINTA_GC_LOCAL(2);

    PINTA_CHECK(pinta_lib_string_alloc(core, 131, value));
    PINTA_CHECK(pinta_lib_string_alloc(core, 131, value));
    PINTA_CHECK(pinta_lib_string_alloc_value(core, L"member", 6, key));
    PINTA_CHECK(pinta_lib_property_table_alloc(core, 4, table));

    PINTA_CHECK(pinta_lib_property_table_add_value(core, table, key, NULL, 1, 1, 1, value));

    pinta_core_gc(core, 1);

    actual_table = pinta_property_table_ref_get_table(table);
    actual_capacity = pinta_property_table_ref_get_capacity(table);

    sput_fail_if(actual_capacity != 4, "Property table was resized");
    for (index = 0; index < actual_capacity; index++)
    {
        current = &actual_table[index];
        if (current->key != NULL)
        {
            sput_fail_if(current->key != key->reference, "Key was not moved");
            sput_fail_if(current->value.data.value != value->reference, "Value was not moved");

            found = 1;
        }
    }

    sput_fail_unless(found, "A valid entry was not found");
}
PINTA_TEST_END(PINTA_OK)

void pinta_tests_property()
{
    sput_enter_suite("Property tests");
    sput_run_test(property_data_should_move_in_compaction);
    sput_leave_suite();
}
