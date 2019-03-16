#include "pinta_tests.h"

PINTA_TEST_BEGIN(weak_does_not_prevent_collection, 2)
{
    PintaReference *weak;
    PintaReference *string;

    weak = PINTA_GC_LOCAL(0);
    string = PINTA_GC_LOCAL(1);

    PINTA_CHECK(pinta_lib_weak_alloc(core, weak));
    PINTA_CHECK(pinta_lib_string_alloc_value(core, L"Some data to fill buffer", 24, string));

    PINTA_CHECK(pinta_lib_weak_set_target(core, weak, string));
    string->reference = NULL;

    pinta_core_gc(core, 0);

    PINTA_CHECK(pinta_lib_weak_get_target(core, weak, string));

    sput_fail_if(string->reference != NULL, "Weak reference collected");
}
PINTA_TEST_END(PINTA_OK)

PINTA_TEST_BEGIN(weak_survives_compaction, 3)
{
    PintaReference *weak;
    PintaReference *string;
    PintaReference *target;

    weak = PINTA_GC_LOCAL(0);
    string = PINTA_GC_LOCAL(1);
    target = PINTA_GC_LOCAL(2);

    PINTA_CHECK(pinta_lib_string_alloc_value(core, L"Some data to fill buffer", 24, string));
    PINTA_CHECK(pinta_lib_string_alloc_value(core, L"Some data to fill buffer", 24, string));
    PINTA_CHECK(pinta_lib_string_alloc_value(core, L"Some data to fill buffer", 24, string));
    PINTA_CHECK(pinta_lib_weak_alloc(core, weak));
    PINTA_CHECK(pinta_lib_weak_alloc(core, weak));
    PINTA_CHECK(pinta_lib_string_alloc_value(core, L"Some data to fill buffer", 24, string));
    PINTA_CHECK(pinta_lib_string_alloc_value(core, L"Some data to fill buffer", 24, string));

    PINTA_CHECK(pinta_lib_weak_set_target(core, weak, string));

    pinta_core_gc(core, 1);

    PINTA_CHECK(pinta_lib_weak_get_target(core, weak, target));

    sput_fail_if(string->reference != target->reference, "Weak survived compaction");
}
PINTA_TEST_END(PINTA_OK)


void pinta_tests_weak()
{
    sput_enter_suite("Weak tests");
    weak_does_not_prevent_collection();
    weak_survives_compaction();
    sput_leave_suite();
}
