#include "pinta_tests.h"

void pinta_test_assert_string_equal(wchar *expected, PintaReference *actual, char *name)
{
    wchar *data = pinta_string_ref_get_data(actual);

    sput_fail_if(wcscmp(expected, data) != 0, name);
}

PINTA_TEST_BEGIN(integer_to_string_tests, 3)
{
    PintaReference *value;
    PintaReference *string;

    value = PINTA_GC_LOCAL(0);
    string = PINTA_GC_LOCAL(1);

    PINTA_CHECK(pinta_lib_integer_alloc_value(core, 0, value));
    PINTA_CHECK(pinta_lib_integer_to_string(core, value, string));
    pinta_test_assert_string_equal(L"0", string, "to string 0");

    PINTA_CHECK(pinta_lib_integer_alloc_value(core, 1, value));
    PINTA_CHECK(pinta_lib_integer_to_string(core, value, string));
    pinta_test_assert_string_equal(L"1", string, "to string 1");

    PINTA_CHECK(pinta_lib_integer_alloc_value(core, -1, value));
    PINTA_CHECK(pinta_lib_integer_to_string(core, value, string));
    pinta_test_assert_string_equal(L"-1", string, "to string -1");

    PINTA_CHECK(pinta_lib_integer_alloc_value(core, 9, value));
    PINTA_CHECK(pinta_lib_integer_to_string(core, value, string));
    pinta_test_assert_string_equal(L"9", string, "to string 9");

    PINTA_CHECK(pinta_lib_integer_alloc_value(core, -9, value));
    PINTA_CHECK(pinta_lib_integer_to_string(core, value, string));
    pinta_test_assert_string_equal(L"-9", string, "to string -9");

    PINTA_CHECK(pinta_lib_integer_alloc_value(core, 10, value));
    PINTA_CHECK(pinta_lib_integer_to_string(core, value, string));
    pinta_test_assert_string_equal(L"10", string, "to string 10");

    PINTA_CHECK(pinta_lib_integer_alloc_value(core, -10, value));
    PINTA_CHECK(pinta_lib_integer_to_string(core, value, string));
    pinta_test_assert_string_equal(L"-10", string, "to string -10");

    PINTA_CHECK(pinta_lib_integer_alloc_value(core, 99, value));
    PINTA_CHECK(pinta_lib_integer_to_string(core, value, string));
    pinta_test_assert_string_equal(L"99", string, "to string 99");

    PINTA_CHECK(pinta_lib_integer_alloc_value(core, -99, value));
    PINTA_CHECK(pinta_lib_integer_to_string(core, value, string));
    pinta_test_assert_string_equal(L"-99", string, "to string -99");

    PINTA_CHECK(pinta_lib_integer_alloc_value(core, 100, value));
    PINTA_CHECK(pinta_lib_integer_to_string(core, value, string));
    pinta_test_assert_string_equal(L"100", string, "to string 100");

    PINTA_CHECK(pinta_lib_integer_alloc_value(core, -100, value));
    PINTA_CHECK(pinta_lib_integer_to_string(core, value, string));
    pinta_test_assert_string_equal(L"-100", string, "to string -100");

    PINTA_CHECK(pinta_lib_integer_alloc_value(core, 999, value));
    PINTA_CHECK(pinta_lib_integer_to_string(core, value, string));
    pinta_test_assert_string_equal(L"999", string, "to string 999");

    PINTA_CHECK(pinta_lib_integer_alloc_value(core, -999, value));
    PINTA_CHECK(pinta_lib_integer_to_string(core, value, string));
    pinta_test_assert_string_equal(L"-999", string, "to string -999");

    PINTA_CHECK(pinta_lib_integer_alloc_value(core, 1000, value));
    PINTA_CHECK(pinta_lib_integer_to_string(core, value, string));
    pinta_test_assert_string_equal(L"1000", string, "to string 1000");

    PINTA_CHECK(pinta_lib_integer_alloc_value(core, -1000, value));
    PINTA_CHECK(pinta_lib_integer_to_string(core, value, string));
    pinta_test_assert_string_equal(L"-1000", string, "to string -1000");

    PINTA_CHECK(pinta_lib_integer_alloc_value(core, 9999, value));
    PINTA_CHECK(pinta_lib_integer_to_string(core, value, string));
    pinta_test_assert_string_equal(L"9999", string, "to string 9999");

    PINTA_CHECK(pinta_lib_integer_alloc_value(core, -9999, value));
    PINTA_CHECK(pinta_lib_integer_to_string(core, value, string));
    pinta_test_assert_string_equal(L"-9999", string, "to string -9999");

    PINTA_CHECK(pinta_lib_integer_alloc_value(core, 10000, value));
    PINTA_CHECK(pinta_lib_integer_to_string(core, value, string));
    pinta_test_assert_string_equal(L"10000", string, "to string 10000");

    PINTA_CHECK(pinta_lib_integer_alloc_value(core, -10000, value));
    PINTA_CHECK(pinta_lib_integer_to_string(core, value, string));
    pinta_test_assert_string_equal(L"-10000", string, "to string -10000");

    PINTA_CHECK(pinta_lib_integer_alloc_value(core, 99999, value));
    PINTA_CHECK(pinta_lib_integer_to_string(core, value, string));
    pinta_test_assert_string_equal(L"99999", string, "to string 99999");

    PINTA_CHECK(pinta_lib_integer_alloc_value(core, -99999, value));
    PINTA_CHECK(pinta_lib_integer_to_string(core, value, string));
    pinta_test_assert_string_equal(L"-99999", string, "to string -99999");

    PINTA_CHECK(pinta_lib_integer_alloc_value(core, 100000, value));
    PINTA_CHECK(pinta_lib_integer_to_string(core, value, string));
    pinta_test_assert_string_equal(L"100000", string, "to string 100000");

    PINTA_CHECK(pinta_lib_integer_alloc_value(core, -100000, value));
    PINTA_CHECK(pinta_lib_integer_to_string(core, value, string));
    pinta_test_assert_string_equal(L"-100000", string, "to string -100000");

    PINTA_CHECK(pinta_lib_integer_alloc_value(core, 999999, value));
    PINTA_CHECK(pinta_lib_integer_to_string(core, value, string));
    pinta_test_assert_string_equal(L"999999", string, "to string 999999");

    PINTA_CHECK(pinta_lib_integer_alloc_value(core, -999999, value));
    PINTA_CHECK(pinta_lib_integer_to_string(core, value, string));
    pinta_test_assert_string_equal(L"-999999", string, "to string -999999");

    PINTA_CHECK(pinta_lib_integer_alloc_value(core, 1000000, value));
    PINTA_CHECK(pinta_lib_integer_to_string(core, value, string));
    pinta_test_assert_string_equal(L"1000000", string, "to string 1000000");

    PINTA_CHECK(pinta_lib_integer_alloc_value(core, -1000000, value));
    PINTA_CHECK(pinta_lib_integer_to_string(core, value, string));
    pinta_test_assert_string_equal(L"-1000000", string, "to string -1000000");

    PINTA_CHECK(pinta_lib_integer_alloc_value(core, 9999999, value));
    PINTA_CHECK(pinta_lib_integer_to_string(core, value, string));
    pinta_test_assert_string_equal(L"9999999", string, "to string 9999999");

    PINTA_CHECK(pinta_lib_integer_alloc_value(core, -9999999, value));
    PINTA_CHECK(pinta_lib_integer_to_string(core, value, string));
    pinta_test_assert_string_equal(L"-9999999", string, "to string -9999999");

    PINTA_CHECK(pinta_lib_integer_alloc_value(core, 10000000, value));
    PINTA_CHECK(pinta_lib_integer_to_string(core, value, string));
    pinta_test_assert_string_equal(L"10000000", string, "to string 10000000");

    PINTA_CHECK(pinta_lib_integer_alloc_value(core, -10000000, value));
    PINTA_CHECK(pinta_lib_integer_to_string(core, value, string));
    pinta_test_assert_string_equal(L"-10000000", string, "to string -10000000");

    PINTA_CHECK(pinta_lib_integer_alloc_value(core, 99999999, value));
    PINTA_CHECK(pinta_lib_integer_to_string(core, value, string));
    pinta_test_assert_string_equal(L"99999999", string, "to string 99999999");

    PINTA_CHECK(pinta_lib_integer_alloc_value(core, -99999999, value));
    PINTA_CHECK(pinta_lib_integer_to_string(core, value, string));
    pinta_test_assert_string_equal(L"-99999999", string, "to string -99999999");

    PINTA_CHECK(pinta_lib_integer_alloc_value(core, 100000000, value));
    PINTA_CHECK(pinta_lib_integer_to_string(core, value, string));
    pinta_test_assert_string_equal(L"100000000", string, "to string 100000000");

    PINTA_CHECK(pinta_lib_integer_alloc_value(core, -100000000, value));
    PINTA_CHECK(pinta_lib_integer_to_string(core, value, string));
    pinta_test_assert_string_equal(L"-100000000", string, "to string -100000000");

    PINTA_CHECK(pinta_lib_integer_alloc_value(core, 999999999, value));
    PINTA_CHECK(pinta_lib_integer_to_string(core, value, string));
    pinta_test_assert_string_equal(L"999999999", string, "to string 999999999");

    PINTA_CHECK(pinta_lib_integer_alloc_value(core, -999999999, value));
    PINTA_CHECK(pinta_lib_integer_to_string(core, value, string));
    pinta_test_assert_string_equal(L"-999999999", string, "to string -999999999");

    PINTA_CHECK(pinta_lib_integer_alloc_value(core, 1000000000, value));
    PINTA_CHECK(pinta_lib_integer_to_string(core, value, string));
    pinta_test_assert_string_equal(L"1000000000", string, "to string 1000000000");

    PINTA_CHECK(pinta_lib_integer_alloc_value(core, -1000000000, value));
    PINTA_CHECK(pinta_lib_integer_to_string(core, value, string));
    pinta_test_assert_string_equal(L"-1000000000", string, "to string -1000000000");

    PINTA_CHECK(pinta_lib_integer_alloc_value(core, 1410065407, value));
    PINTA_CHECK(pinta_lib_integer_to_string(core, value, string));
    pinta_test_assert_string_equal(L"1410065407", string, "to string 1410065407");

    PINTA_CHECK(pinta_lib_integer_alloc_value(core, -1410065407, value));
    PINTA_CHECK(pinta_lib_integer_to_string(core, value, string));
    pinta_test_assert_string_equal(L"-1410065407", string, "to string -1410065407");

    PINTA_CHECK(pinta_lib_integer_alloc_value(core, 1410065408, value));
    PINTA_CHECK(pinta_lib_integer_to_string(core, value, string));
    pinta_test_assert_string_equal(L"1410065408", string, "to string 1410065408");

    PINTA_CHECK(pinta_lib_integer_alloc_value(core, -1410065408, value));
    PINTA_CHECK(pinta_lib_integer_to_string(core, value, string));
    pinta_test_assert_string_equal(L"-1410065408", string, "to string -1410065408");

    PINTA_CHECK(pinta_lib_integer_alloc_value(core, 1215752191, value));
    PINTA_CHECK(pinta_lib_integer_to_string(core, value, string));
    pinta_test_assert_string_equal(L"1215752191", string, "to string 1215752191");

    PINTA_CHECK(pinta_lib_integer_alloc_value(core, -1215752191, value));
    PINTA_CHECK(pinta_lib_integer_to_string(core, value, string));
    pinta_test_assert_string_equal(L"-1215752191", string, "to string -1215752191");

    PINTA_CHECK(pinta_lib_integer_alloc_value(core, 628, value));
    PINTA_CHECK(pinta_lib_integer_to_string(core, value, string));
    pinta_test_assert_string_equal(L"628", string, "to string 628");

    PINTA_CHECK(pinta_lib_integer_alloc_value(core, -628, value));
    PINTA_CHECK(pinta_lib_integer_to_string(core, value, string));
    pinta_test_assert_string_equal(L"-628", string, "to string -628");

    PINTA_CHECK(pinta_lib_integer_alloc_value(core, 2012345678, value));
    PINTA_CHECK(pinta_lib_integer_to_string(core, value, string));
    pinta_test_assert_string_equal(L"2012345678", string, "to string 2012345678");

    PINTA_CHECK(pinta_lib_integer_alloc_value(core, -2012345678, value));
    PINTA_CHECK(pinta_lib_integer_to_string(core, value, string));
    pinta_test_assert_string_equal(L"-2012345678", string, "to string -2012345678");
}
PINTA_TEST_END(PINTA_OK)

void pinta_tests_integer()
{
    sput_enter_suite("Integer tests");
    sput_run_test(integer_to_string_tests);
    sput_leave_suite();
}
