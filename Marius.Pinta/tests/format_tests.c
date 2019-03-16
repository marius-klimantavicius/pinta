#include "pinta_tests.h"

void pinta_test_format_string(wchar *format_string, wchar *format_argument, wchar *expected)
{
    PintaException exception = PINTA_OK;
    wchar *actual_data;
    i32 test;
    struct
    {
        PintaReference format;
        PintaReference arguments;
        PintaReference item;
        PintaReference actual;
    } gc;

    PINTA_GC_ENTER(core, gc);

    PINTA_CHECK(pinta_lib_string_alloc_value(core, format_string, wcslen(format_string), &gc.format));
    PINTA_CHECK(pinta_lib_array_alloc(core, 1, &gc.arguments));
    PINTA_CHECK(pinta_lib_string_alloc_value(core, format_argument, wcslen(format_argument), &gc.item));
    PINTA_CHECK(pinta_lib_array_set_item(core, &gc.arguments, 0, &gc.item));

    PINTA_CHECK(pinta_lib_format(core, NULL, &gc.format, &gc.arguments, 0, &gc.actual));
    PINTA_CHECK(pinta_lib_string_to_string(core, &gc.actual, &gc.actual));

    actual_data = pinta_string_ref_get_data(&gc.actual);
    test = wcscmp(expected, actual_data);
    if (test != 0)
        sput_fail_if(test != 0, "failed format");

PINTA_EXIT:
    PINTA_GC_EXIT(core);
    if (exception != PINTA_OK)
        sput_fail_if(exception != PINTA_OK, "No exception");
}

void pinta_test_format(wchar *format_string, wchar *format_argument, wchar *expected)
{
    PintaException exception = PINTA_OK;
    wchar *actual_data;
    i32 test;
    struct
    {
        PintaReference format;
        PintaReference arguments;
        PintaReference item;
        PintaReference actual;
    } gc;
    pinta_assert(format_string != NULL);
    pinta_assert(format_argument != NULL);
    pinta_assert(expected != NULL);

    PINTA_GC_ENTER(core, gc);

    PINTA_CHECK(pinta_lib_string_alloc_value(core, format_string, wcslen(format_string), &gc.format));
    PINTA_CHECK(pinta_lib_array_alloc(core, 1, &gc.arguments));
    PINTA_CHECK(pinta_lib_string_alloc_value(core, format_argument, wcslen(format_argument), &gc.item));
    PINTA_CHECK(pinta_lib_array_set_item(core, &gc.arguments, 0, &gc.item));

    PINTA_CHECK(pinta_lib_format(core, NULL, &gc.format, &gc.arguments, 0, &gc.actual));
    PINTA_CHECK(pinta_lib_string_to_string(core, &gc.actual, &gc.actual));

    actual_data = pinta_string_ref_get_data(&gc.actual);
    test = wcscmp(expected, actual_data);
    sput_fail_if(test != 0, pinta_tests_message(L"failed format (%ls, %ls): expected - %ls, actual - %ls", format_string, format_argument, expected, actual_data));
    if (test != 0)
        test++;

PINTA_EXIT:
    PINTA_GC_EXIT(core);
    if (exception != PINTA_OK)
        sput_fail_if(exception != PINTA_OK, "No exception");
}

PINTA_TEST_BEGIN(format_multiple_arguments, 4)
{
    wchar *actual_data;
    wchar *expected;
    i32 test;
    PintaReference *format;
    PintaReference *arguments;
    PintaReference *item;
    PintaReference *actual;

    format = PINTA_GC_LOCAL(0);
    arguments = PINTA_GC_LOCAL(1);
    item = PINTA_GC_LOCAL(2);
    actual = PINTA_GC_LOCAL(3);

    PINTA_CHECK(pinta_lib_string_alloc_value(core, L"%s: %#.2f %*s", wcslen(L"%s: %#.2f %*s"), format));
    PINTA_CHECK(pinta_lib_array_alloc(core, 5, arguments));

    PINTA_CHECK(pinta_lib_string_alloc_value(core, L"Total", 5, item));
    PINTA_CHECK(pinta_lib_array_set_item(core, arguments, 1, item));

    PINTA_CHECK(pinta_lib_string_alloc_value(core, L"100.25", 6, item));
    PINTA_CHECK(pinta_lib_array_set_item(core, arguments, 2, item));

    PINTA_CHECK(pinta_lib_integer_alloc_value(core, -5, item));
    PINTA_CHECK(pinta_lib_array_set_item(core, arguments, 3, item));

    PINTA_CHECK(pinta_lib_string_alloc_value(core, L"Lt", 2, item));
    PINTA_CHECK(pinta_lib_array_set_item(core, arguments, 4, item));

    PINTA_CHECK(pinta_lib_format(core, NULL, format, arguments, 1, actual));
    PINTA_CHECK(pinta_lib_string_to_string(core, actual, actual));

    actual_data = pinta_string_ref_get_data(actual);
    expected = L"Total: 100.25    Lt";
    test = wcscmp(expected, actual_data);
    if (test != 0)
        sput_fail_if(test != 0, "format_multiple_arguments");

}
PINTA_TEST_END(PINTA_OK)

void pinta_test_format_strings()
{
    pinta_init_tests(30 * 1024, 1024);

    pinta_test_format(L"%.2g", L"25500", L"2.6e+04");
    pinta_test_format(L"%.3g", L"25500", L"2.55e+04");
    pinta_test_format(L"%.4g", L"25500", L"2.55e+04");
    pinta_test_format(L"%.5g", L"25500", L"25500");

    pinta_test_format(L"%f", L"0", L"0");
    pinta_test_format(L"%g", L"0", L"0");
    pinta_test_format(L"%e", L"0", L"0.000000e+000");

    pinta_test_format(L"%.0e", L"0", L"0e+000");

    pinta_test_format_string(L"%s", L"Hello world", L"Hello world");
    pinta_test_format_string(L"%-10s", L"Text", L"      Text");
    pinta_test_format_string(L"%10s", L"Text", L"Text      ");
    pinta_test_format_string(L"%-10.2s", L"Text", L"        Te");
    pinta_test_format_string(L"%10.2s", L"Text", L"Te        ");

    pinta_test_format_string(L"%#s", L"Hello world", L"Hello world");
    pinta_test_format_string(L"% -10s", L"Text", L"      Text");
    pinta_test_format_string(L"%+10s", L"Text", L"Text      ");
    pinta_test_format_string(L"%-10.2s", L"Text", L"        Te");
    pinta_test_format_string(L"%-010.2s", L"Text", L"00000000Te");

    pinta_test_format(L"%+#23.15e", L"78945612", L"+7.894561200000000e+007");
    pinta_test_format(L"%-#23.15e", L"78945612", L" 7.894561200000000e+007");
    pinta_test_format(L"%#23.15e", L"78945612", L"7.894561200000000e+007 ");
    pinta_test_format(L"%0#23.15e", L"78945612", L"7.894561200000000e+007 ");
    pinta_test_format(L"%#1.1g", L"78945612", L"8.e+07");
    pinta_test_format(L"%+8d", L"100", L"+100    ");
    pinta_test_format(L"%-5d", L"100", L"  100");
    pinta_test_format(L"%08.2f", L"-10.998", L"-11.00  ");
    pinta_test_format(L"%-08.2f", L"-10.998", L"-0011.00");

    pinta_test_format(L"%d", L"10.4", L"10");
    pinta_test_format(L"%d", L"10.9", L"10");
    pinta_test_format(L"%5d", L"10.9", L"10   ");
    pinta_test_format(L"%-5d", L"10.9", L"   10");
    pinta_test_format(L"%05d", L"10.9", L"10   ");
    pinta_test_format(L"%#d", L"10.9", L"10.");
    pinta_test_format(L"%+d", L"10.9", L"+10");
    pinta_test_format(L"%+05d", L"10.9", L"+10  ");
    pinta_test_format(L"% 05d", L"10.9", L" 10  ");
    pinta_test_format(L"%+#05d", L"10.9", L"+10. ");
    pinta_test_format(L"%+-5d", L"10.9", L"  +10");
    pinta_test_format(L"%+-#5d", L"10.9", L" +10.");

    pinta_test_format(L"%.2d", L"10.4", L"10.00");
    pinta_test_format(L"%.1d", L"10.9", L"10.0");
    pinta_test_format(L"%6.2d", L"10.9", L"10.00 ");
    pinta_test_format(L"%-5.1d", L"10.9", L" 10.0");
    pinta_test_format(L"%05.0d", L"10.9", L"10   ");
    pinta_test_format(L"%#.2d", L"10.9", L"10.00");
    pinta_test_format(L"%+.1d", L"10.9", L"+10.0");
    pinta_test_format(L"%+05.5d", L"10.9", L"+10.00000");
    pinta_test_format(L"% 06.1d", L"10.9", L" 10.0 ");
    pinta_test_format(L"%+#07.1d", L"10.9", L"+10.0  ");
    pinta_test_format(L"%+-5.1d", L"10.9", L"+10.0");
    pinta_test_format(L"%+-#5.3d", L"10.9", L"+10.000");

    pinta_test_format(L"%d", L"-10.4", L"-10");
    pinta_test_format(L"%d", L"-10.9", L"-10");
    pinta_test_format(L"%5d", L"-10.9", L"-10  ");
    pinta_test_format(L"%-5d", L"-10.9", L"  -10");
    pinta_test_format(L"%05d", L"-10.9", L"-10  ");
    pinta_test_format(L"%#d", L"-10.9", L"-10.");
    pinta_test_format(L"%+d", L"-10.9", L"-10");
    pinta_test_format(L"%+05d", L"-10.9", L"-10  ");
    pinta_test_format(L"% 05d", L"-10.9", L"-10  ");
    pinta_test_format(L"%+#05d", L"-10.9", L"-10. ");
    pinta_test_format(L"%+-5d", L"-10.9", L"  -10");
    pinta_test_format(L"%+-#5d", L"-10.9", L" -10.");

    pinta_test_format(L"%.2d", L"-10.4", L"-10.00");
    pinta_test_format(L"%.1d", L"-10.9", L"-10.0");
    pinta_test_format(L"%6.2d", L"-10.9", L"-10.00");
    pinta_test_format(L"%-5.1d", L"-10.9", L"-10.0");
    pinta_test_format(L"%05.0d", L"-10.9", L"-10  ");
    pinta_test_format(L"%#.2d", L"-10.9", L"-10.00");
    pinta_test_format(L"%+.1d", L"-10.9", L"-10.0");
    pinta_test_format(L"%+05.5d", L"-10.9", L"-10.00000");
    pinta_test_format(L"% 06.1d", L"-10.9", L"-10.0 ");
    pinta_test_format(L"%+#07.1d", L"-10.9", L"-10.0  ");
    pinta_test_format(L"%+-5.1d", L"-10.9", L"-10.0");
    pinta_test_format(L"%+-#5.3d", L"-10.9", L"-10.000");

    /* %f */
    pinta_test_format(L"%f", L"16.7633", L"16.76");
    pinta_test_format(L"%7f", L"16.7633", L"16.76  ");
    pinta_test_format(L"%7f", L"16.7653", L"16.77  ");
    pinta_test_format(L"%7f", L"16.7693", L"16.77  ");
    pinta_test_format(L"%-7f", L"16.7633", L"  16.76");
    pinta_test_format(L"%-7f", L"16.7653", L"  16.77");
    pinta_test_format(L"%-7f", L"16.7693", L"  16.77");
    pinta_test_format(L"%.0f", L"16.7693", L"17");
    pinta_test_format(L"%.5f", L"16.8", L"16.80000");
    pinta_test_format(L"%.20f", L"15988", L"15988.00000000000000000000");
    pinta_test_format(L"%08f", L"0.005", L"0.01    ");
    pinta_test_format(L"%08.2f", L"0.004", L"0.00    ");
    pinta_test_format(L"%08.2f", L"0.005", L"0.01    ");
    pinta_test_format(L"%-8f", L"0.005", L"    0.01");
    pinta_test_format(L"%-8.2f", L"0.004", L"    0.00");
    pinta_test_format(L"%-8.2f", L"0.005", L"    0.01");
    pinta_test_format(L"%-8.1f", L"0.004", L"     0.0");
    pinta_test_format(L"%-8.1f", L"0.005", L"     0.0");
    pinta_test_format(L"%-8.0f", L"0.004", L"       0");
    pinta_test_format(L"%-8.0f", L"0.005", L"       0");
    pinta_test_format(L"%+8f", L"147.789", L"+147.79 ");
    pinta_test_format(L"%-+8f", L"147.789", L" +147.79");
    pinta_test_format(L"%+#8f", L"147.789", L"+147.79 ");
    pinta_test_format(L"%-+#8f", L"147.789", L" +147.79");
    pinta_test_format(L"% #8f", L"147.789", L" 147.79 ");
    pinta_test_format(L"%- #8f", L"147.789", L"  147.79");

    pinta_test_format(L"%+8.5f", L"147.789", L"+147.78900");
    pinta_test_format(L"%-+8.5f", L"147.789", L"+147.78900");
    pinta_test_format(L"%+#8.5f", L"147.789", L"+147.78900");
    pinta_test_format(L"%-+#8.5f", L"147.789", L"+147.78900");
    pinta_test_format(L"% #8.5f", L"147.789", L" 147.78900");
    pinta_test_format(L"%- #8.5f", L"147.789", L" 147.78900");

    pinta_test_format(L"%+15.5f", L"147.789", L"+147.78900     ");
    pinta_test_format(L"%-+15.5f", L"147.789", L"     +147.78900");
    pinta_test_format(L"%+#15.5f", L"147.789", L"+147.78900     ");
    pinta_test_format(L"%-+#15.5f", L"147.789", L"     +147.78900");
    pinta_test_format(L"% #15.5f", L"147.789", L" 147.78900     ");
    pinta_test_format(L"%- #15.5f", L"147.789", L"      147.78900");

    pinta_test_format(L"%f", L"-16.7633", L"-16.76");
    pinta_test_format(L"%7f", L"-16.7633", L"-16.76 ");
    pinta_test_format(L"%7f", L"-16.7653", L"-16.77 ");
    pinta_test_format(L"%7f", L"-16.7693", L"-16.77 ");
    pinta_test_format(L"%-7f", L"-16.7633", L" -16.76");
    pinta_test_format(L"%-7f", L"-16.7653", L" -16.77");
    pinta_test_format(L"%-7f", L"-16.7693", L" -16.77");
    pinta_test_format(L"%.0f", L"-16.7693", L"-17");
    pinta_test_format(L"%.5f", L"-16.8", L"-16.80000");
    pinta_test_format(L"%.20f", L"-15988", L"-15988.00000000000000000000");
    pinta_test_format(L"%08f", L"-0.005", L"-0.01   ");
    pinta_test_format(L"%08.2f", L"-0.004", L"-0.00   ");
    pinta_test_format(L"%08.2f", L"-0.005", L"-0.01   ");
    pinta_test_format(L"%-8f", L"-0.005", L"   -0.01");
    pinta_test_format(L"%-8.2f", L"-0.004", L"   -0.00");
    pinta_test_format(L"%-8.2f", L"-0.005", L"   -0.01");
    pinta_test_format(L"%-8.1f", L"-0.004", L"    -0.0");
    pinta_test_format(L"%-8.1f", L"-0.005", L"    -0.0");
    pinta_test_format(L"%-8.0f", L"-0.004", L"      -0");
    pinta_test_format(L"%-8.0f", L"-0.005", L"      -0");
    pinta_test_format(L"%+8f", L"-147.789", L"-147.79 ");
    pinta_test_format(L"%-+8f", L"-147.789", L" -147.79");
    pinta_test_format(L"%+#8f", L"-147.789", L"-147.79 ");
    pinta_test_format(L"%-+#8f", L"-147.789", L" -147.79");
    pinta_test_format(L"% #8f", L"-147.789", L"-147.79 ");
    pinta_test_format(L"%- #8f", L"-147.789", L" -147.79");

    pinta_test_format(L"%+8.5f", L"-147.789", L"-147.78900");
    pinta_test_format(L"%-+8.5f", L"-147.789", L"-147.78900");
    pinta_test_format(L"%+#8.5f", L"-147.789", L"-147.78900");
    pinta_test_format(L"%-+#8.5f", L"-147.789", L"-147.78900");
    pinta_test_format(L"% #8.5f", L"-147.789", L"-147.78900");
    pinta_test_format(L"%- #8.5f", L"-147.789", L"-147.78900");

    pinta_test_format(L"%+15.5f", L"-147.789", L"-147.78900     ");
    pinta_test_format(L"%-+15.5f", L"-147.789", L"     -147.78900");
    pinta_test_format(L"%+#15.5f", L"-147.789", L"-147.78900     ");
    pinta_test_format(L"%-+#15.5f", L"-147.789", L"     -147.78900");
    pinta_test_format(L"% #15.5f", L"-147.789", L"-147.78900     ");
    pinta_test_format(L"%- #15.5f", L"-147.789", L"     -147.78900");

    /* %g positive*/
    pinta_test_format(L"%g", L"0.0000005", L"5e-07");
    pinta_test_format(L"%g", L"0.0000005558874784548784", L"5.6e-07");
    pinta_test_format(L"%g", L"0.005", L"0.005");
    pinta_test_format(L"%g", L"0.005000", L"0.005");
    pinta_test_format(L"%g", L"1.111111", L"1.111111");
    pinta_test_format(L"%g", L"157", L"157");
    pinta_test_format(L"%g", L"15.45e6", L"15450000");
    pinta_test_format(L"%g", L"15.78854787e6", L"15788547.87");

    pinta_test_format(L"%.1g", L"157", L"2e+02");

    pinta_test_format(L"%.1G", L"157", L"2E+02");

    pinta_test_format(L"%.3g", L"0.0000005", L"5e-07");
    pinta_test_format(L"%.3g", L"0.005", L"0.005");
    pinta_test_format(L"%.3g", L"0.005000", L"0.005");
    pinta_test_format(L"%.3g", L"1.111111", L"1.11");
    pinta_test_format(L"%.3g", L"157", L"157");
    pinta_test_format(L"%.3g", L"15.45e6", L"1.55e+07");
    pinta_test_format(L"%.3g", L"15.78854787e6", L"1.58e+07");

    pinta_test_format(L"%.3G", L"15.45e6", L"1.55E+07");
    pinta_test_format(L"%.3G", L"15.78854787e6", L"1.58E+07");

    pinta_test_format(L"%.7g", L"0.0000005", L"5e-07");
    pinta_test_format(L"%.7g", L"0.005", L"0.005");
    pinta_test_format(L"%.7g", L"0.005000", L"0.005");
    pinta_test_format(L"%.7g", L"1.111111", L"1.111111");
    pinta_test_format(L"%.7g", L"157", L"157");
    pinta_test_format(L"%.7g", L"15.45e6", L"1.545e+07");
    pinta_test_format(L"%.7g", L"15.78854787e6", L"1.578855e+07");

    pinta_test_format(L"%.7G", L"15.45e6", L"1.545E+07");
    pinta_test_format(L"%.7G", L"15.78854787e6", L"1.578855E+07");

    pinta_test_format(L"%.0g", L"0.0000005", L"5e-07");
    pinta_test_format(L"%.0g", L"0.005", L"0.005");
    pinta_test_format(L"%.0g", L"0.005000", L"0.005");
    pinta_test_format(L"%.0g", L"1.111111", L"1.111111");
    pinta_test_format(L"%.0g", L"157", L"157");
    pinta_test_format(L"%.0g", L"15.45e6", L"15450000");
    pinta_test_format(L"%.0g", L"15.78854787e6", L"15788547.87");

    pinta_test_format(L"%8g", L"0.005", L"0.005   ");
    pinta_test_format(L"%8g", L"0.005000", L"0.005   ");
    pinta_test_format(L"%8g", L"1.111111", L"1.111111");
    pinta_test_format(L"%8g", L"157", L"157     ");
    pinta_test_format(L"%#8g", L"157", L"157.    ");
    pinta_test_format(L"%+8g", L"157", L"+157    ");
    pinta_test_format(L"%-8g", L"157", L"     157");
    pinta_test_format(L"%08g", L"157", L"157     ");
    pinta_test_format(L"% #8g", L"157", L" 157.   ");

    pinta_test_format(L"%8g", L"15.45e6", L"15450000");
    pinta_test_format(L"%8g", L"15.78854787e6", L"15788547.87");

    pinta_test_format(L"%8.1g", L"157", L"2e+02   ");
    pinta_test_format(L"%08.1g", L"157", L"2e+02   ");
    pinta_test_format(L"%+8.1g", L"157", L"+2e+02  ");
    pinta_test_format(L"%-8.1g", L"157", L"   2e+02");
    pinta_test_format(L"%+-8.1g", L"157", L"  +2e+02");
    pinta_test_format(L"%0#8.1g", L"157", L"2.e+02  ");
    pinta_test_format(L"%+08.1g", L"157", L"+2e+02  ");
    pinta_test_format(L"% 08.1g", L"157", L" 2e+02  ");
    pinta_test_format(L"%- #8.1g", L"157", L"  2.e+02");

    pinta_test_format(L"%8.1G", L"157", L"2E+02   ");
    pinta_test_format(L"%08.1G", L"157", L"2E+02   ");
    pinta_test_format(L"%+8.1G", L"157", L"+2E+02  ");
    pinta_test_format(L"%-8.1G", L"157", L"   2E+02");
    pinta_test_format(L"%+-8.1G", L"157", L"  +2E+02");
    pinta_test_format(L"%0#8.1G", L"157", L"2.E+02  ");
    pinta_test_format(L"%+08.1G", L"157", L"+2E+02  ");
    pinta_test_format(L"% 08.1G", L"157", L" 2E+02  ");
    pinta_test_format(L"%- #8.1G", L"157", L"  2.E+02");

    pinta_test_format(L"%8.1g", L"1000", L"1e+03   ");
    pinta_test_format(L"%08.1g", L"1000", L"1e+03   ");
    pinta_test_format(L"%+8.1g", L"1000", L"+1e+03  ");
    pinta_test_format(L"%-8.1g", L"1000", L"   1e+03");
    pinta_test_format(L"%+-8.1g", L"1000", L"  +1e+03");
    pinta_test_format(L"%0#8.1g", L"1000", L"1.e+03  ");
    pinta_test_format(L"%+08.1g", L"1000", L"+1e+03  ");
    pinta_test_format(L"% 08.1g", L"1000", L" 1e+03  ");
    pinta_test_format(L"%- #8.1g", L"1000", L"  1.e+03");

    pinta_test_format(L"%8.1G", L"1000", L"1E+03   ");
    pinta_test_format(L"%08.1G", L"1000", L"1E+03   ");
    pinta_test_format(L"%+8.1G", L"1000", L"+1E+03  ");
    pinta_test_format(L"%-8.1G", L"1000", L"   1E+03");
    pinta_test_format(L"%+-8.1G", L"1000", L"  +1E+03");
    pinta_test_format(L"%0#8.1G", L"1000", L"1.E+03  ");
    pinta_test_format(L"%+08.1G", L"1000", L"+1E+03  ");
    pinta_test_format(L"% 08.1G", L"1000", L" 1E+03  ");
    pinta_test_format(L"%- #8.1G", L"1000", L"  1.E+03");

    pinta_test_format(L"%8.2g", L"1000", L"1e+03   ");
    pinta_test_format(L"%08.2g", L"1000", L"1e+03   ");
    pinta_test_format(L"%+8.2g", L"1000", L"+1e+03  ");
    pinta_test_format(L"%-8.2g", L"1000", L"   1e+03");
    pinta_test_format(L"%+-8.2g", L"1000", L"  +1e+03");
    pinta_test_format(L"%0#8.2g", L"1000", L"1.e+03  ");
    pinta_test_format(L"%+08.2g", L"1000", L"+1e+03  ");
    pinta_test_format(L"% 08.2g", L"1000", L" 1e+03  ");
    pinta_test_format(L"%- #8.2g", L"1000", L"  1.e+03");

    pinta_test_format(L"%8.2G", L"1000", L"1E+03   ");
    pinta_test_format(L"%08.2G", L"1000", L"1E+03   ");
    pinta_test_format(L"%+8.2G", L"1000", L"+1E+03  ");
    pinta_test_format(L"%-8.2G", L"1000", L"   1E+03");
    pinta_test_format(L"%+-8.2G", L"1000", L"  +1E+03");
    pinta_test_format(L"%0#8.2G", L"1000", L"1.E+03  ");
    pinta_test_format(L"%+08.2G", L"1000", L"+1E+03  ");
    pinta_test_format(L"% 08.2G", L"1000", L" 1E+03  ");
    pinta_test_format(L"%- #8.2G", L"1000", L"  1.E+03");

    pinta_test_format(L"%8.3g", L"0.0000005", L"5e-07   ");
    pinta_test_format(L"%8.3g", L"0.005", L"0.005   ");
    pinta_test_format(L"%8.3g", L"0.005000", L"0.005   ");
    pinta_test_format(L"%8.3g", L"1.111111", L"1.11    ");
    pinta_test_format(L"%8.3g", L"157", L"157     ");
    pinta_test_format(L"%8.3g", L"15.45e6", L"1.55e+07");
    pinta_test_format(L"%8.3g", L"15.78854787e6", L"1.58e+07");

    pinta_test_format(L"%8.3G", L"15.45e6", L"1.55E+07");
    pinta_test_format(L"%8.3G", L"15.78854787e6", L"1.58E+07");

    pinta_test_format(L"%8.7g", L"0.0000005", L"5e-07   ");

    pinta_test_format(L"%8.7G", L"0.0000005", L"5E-07   ");

    pinta_test_format(L"%8.7g", L"0.005", L"0.005   ");
    pinta_test_format(L"%8.7g", L"0.005000", L"0.005   ");
    pinta_test_format(L"%8.7g", L"1.111111", L"1.111111");
    pinta_test_format(L"%8.7g", L"157", L"157     ");
    pinta_test_format(L"%8.7g", L"15.45e6", L"1.545e+07");
    pinta_test_format(L"%8.7g", L"15.78854787e6", L"1.578855e+07");

    pinta_test_format(L"%8.7G", L"15.45e6", L"1.545E+07");
    pinta_test_format(L"%8.7G", L"15.78854787e6", L"1.578855E+07");

    pinta_test_format(L"%8.0g", L"0.0000005", L"5e-07   ");
    pinta_test_format(L"%8.0g", L"0.005", L"0.005   ");
    pinta_test_format(L"%8.0g", L"0.005000", L"0.005   ");
    pinta_test_format(L"%8.0g", L"1.111111", L"1.111111");
    pinta_test_format(L"%8.0g", L"157", L"157     ");
    pinta_test_format(L"%8.0g", L"15.45e6", L"15450000");
    pinta_test_format(L"%8.0g", L"15.78854787e6", L"15788547.87");

    pinta_test_format(L"%8.0G", L"15.45e6", L"15450000");
    pinta_test_format(L"%8.0G", L"15.78854787e6", L"15788547.87");

    /*g - negative*/
    pinta_test_format(L"%g", L"-0.0000005", L"-5e-07"); 
    pinta_test_format(L"%g", L"-0.0000005558874784548784", L"-5.6e-07");
    pinta_test_format(L"%g", L"-0.005", L"-0.005");
    pinta_test_format(L"%g", L"-0.005000", L"-0.005");
    pinta_test_format(L"%g", L"-1.111111", L"-1.111111");
    pinta_test_format(L"%g", L"-157", L"-157");
    pinta_test_format(L"%g", L"-15.45e6", L"-15450000");
    pinta_test_format(L"%g", L"-15.78854787e6", L"-15788547.87");

    pinta_test_format(L"%.1g", L"-157", L"-2e+02");

    pinta_test_format(L"%.1G", L"-157", L"-2E+02");

    pinta_test_format(L"%.3g", L"-0.0000005", L"-5e-07");
    pinta_test_format(L"%.3g", L"-0.005", L"-0.005");
    pinta_test_format(L"%.3g", L"-0.005000", L"-0.005");
    pinta_test_format(L"%.3g", L"-1.111111", L"-1.11");
    pinta_test_format(L"%.3g", L"-157", L"-157");
    pinta_test_format(L"%.3g", L"-15.45e6", L"-1.55e+07");
    pinta_test_format(L"%.3g", L"-15.78854787e6", L"-1.58e+07");

    pinta_test_format(L"%.3G", L"-15.45e6", L"-1.55E+07");
    pinta_test_format(L"%.3G", L"-15.78854787e6", L"-1.58E+07");

    pinta_test_format(L"%.7g", L"-0.0000005", L"-5e-07");
    pinta_test_format(L"%.7g", L"-0.005", L"-0.005");
    pinta_test_format(L"%.7g", L"-0.005000", L"-0.005");
    pinta_test_format(L"%.7g", L"-1.111111", L"-1.111111");
    pinta_test_format(L"%.7g", L"-157", L"-157");
    pinta_test_format(L"%.7g", L"-15.45e6", L"-1.545e+07");
    pinta_test_format(L"%.7g", L"-15.78854787e6", L"-1.578855e+07");

    pinta_test_format(L"%.7G", L"-0.0000005", L"-5E-07");
    pinta_test_format(L"%.7G", L"-0.005", L"-0.005");
    pinta_test_format(L"%.7G", L"-0.005000", L"-0.005");
    pinta_test_format(L"%.7G", L"-1.111111", L"-1.111111");
    pinta_test_format(L"%.7G", L"-157", L"-157");
    pinta_test_format(L"%.7G", L"-15.45e6", L"-1.545E+07");
    pinta_test_format(L"%.7G", L"-15.78854787e6", L"-1.578855E+07");

    pinta_test_format(L"%.0g", L"-0.0000005", L"-5e-07");
    pinta_test_format(L"%.0g", L"-0.005", L"-0.005");
    pinta_test_format(L"%.0g", L"-0.005000", L"-0.005");
    pinta_test_format(L"%.0g", L"-1.111111", L"-1.111111");
    pinta_test_format(L"%.0g", L"-157", L"-157");
    pinta_test_format(L"%.0g", L"-15.45e6", L"-15450000");
    pinta_test_format(L"%.0g", L"-15.78854787e6", L"-15788547.87");

    pinta_test_format(L"%8g", L"-0.005", L"-0.005  ");
    pinta_test_format(L"%8g", L"-0.005000", L"-0.005  ");
    pinta_test_format(L"%8g", L"-1.111111", L"-1.111111");
    pinta_test_format(L"%8g", L"-157", L"-157    ");
    pinta_test_format(L"%#8g", L"-157", L"-157.   ");
    pinta_test_format(L"%+8g", L"-157", L"-157    ");
    pinta_test_format(L"%-8g", L"-157", L"    -157");
    pinta_test_format(L"%08g", L"-157", L"-157    ");
    pinta_test_format(L"% #8g", L"-157", L"-157.   ");

    pinta_test_format(L"%8g", L"-15.45e6", L"-15450000");
    pinta_test_format(L"%8g", L"-15.78854787e6", L"-15788547.87");

    pinta_test_format(L"%8.1g", L"-157", L"-2e+02  ");
    pinta_test_format(L"%08.1g", L"-157", L"-2e+02  ");
    pinta_test_format(L"%+8.1g", L"-157", L"-2e+02  ");
    pinta_test_format(L"%-8.1g", L"-157", L"  -2e+02");
    pinta_test_format(L"%+-8.1g", L"-157", L"  -2e+02");
    pinta_test_format(L"%0#8.1g", L"-157", L"-2.e+02 ");
    pinta_test_format(L"%+08.1g", L"-157", L"-2e+02  ");
    pinta_test_format(L"% 08.1g", L"-157", L"-2e+02  ");
    pinta_test_format(L"%- #8.1g", L"-157", L" -2.e+02");

    pinta_test_format(L"%8.1G", L"-157", L"-2E+02  ");
    pinta_test_format(L"%08.1G", L"-157", L"-2E+02  ");
    pinta_test_format(L"%+8.1G", L"-157", L"-2E+02  ");
    pinta_test_format(L"%-8.1G", L"-157", L"  -2E+02");
    pinta_test_format(L"%+-8.1G", L"-157", L"  -2E+02");
    pinta_test_format(L"%0#8.1G", L"-157", L"-2.E+02 ");
    pinta_test_format(L"%+08.1G", L"-157", L"-2E+02  ");
    pinta_test_format(L"% 08.1G", L"-157", L"-2E+02  ");
    pinta_test_format(L"%- #8.1G", L"-157", L" -2.E+02");

    pinta_test_format(L"%8.1g", L"-1000", L"-1e+03  ");
    pinta_test_format(L"%08.1g", L"-1000", L"-1e+03  ");
    pinta_test_format(L"%+8.1g", L"-1000", L"-1e+03  ");
    pinta_test_format(L"%-8.1g", L"-1000", L"  -1e+03");
    pinta_test_format(L"%+-8.1g", L"-1000", L"  -1e+03");
    pinta_test_format(L"%0#8.1g", L"-1000", L"-1.e+03 ");
    pinta_test_format(L"%+08.1g", L"-1000", L"-1e+03  ");
    pinta_test_format(L"% 08.1g", L"-1000", L"-1e+03  ");
    pinta_test_format(L"%- #8.1g", L"-1000", L" -1.e+03");

    pinta_test_format(L"%8.1G", L"-1000", L"-1E+03  ");
    pinta_test_format(L"%08.1G", L"-1000", L"-1E+03  ");
    pinta_test_format(L"%+8.1G", L"-1000", L"-1E+03  ");
    pinta_test_format(L"%-8.1G", L"-1000", L"  -1E+03");
    pinta_test_format(L"%+-8.1G", L"-1000", L"  -1E+03");
    pinta_test_format(L"%0#8.1G", L"-1000", L"-1.E+03 ");
    pinta_test_format(L"%+08.1G", L"-1000", L"-1E+03  ");
    pinta_test_format(L"% 08.1G", L"-1000", L"-1E+03  ");
    pinta_test_format(L"%- #8.1G", L"-1000", L" -1.E+03");

    pinta_test_format(L"%8.2g", L"-1000", L"-1e+03  ");
    pinta_test_format(L"%08.2g", L"-1000", L"-1e+03  ");
    pinta_test_format(L"%+8.2g", L"-1000", L"-1e+03  ");
    pinta_test_format(L"%-8.2g", L"-1000", L"  -1e+03");
    pinta_test_format(L"%+-8.2g", L"-1000", L"  -1e+03");
    pinta_test_format(L"%0#8.2g", L"-1000", L"-1.e+03 ");
    pinta_test_format(L"%+08.2g", L"-1000", L"-1e+03  ");
    pinta_test_format(L"% 08.2g", L"-1000", L"-1e+03  ");
    pinta_test_format(L"%- #8.2g", L"-1000", L" -1.e+03");

    pinta_test_format(L"%8.2G", L"-1000", L"-1E+03  ");
    pinta_test_format(L"%08.2G", L"-1000", L"-1E+03  ");
    pinta_test_format(L"%+8.2G", L"-1000", L"-1E+03  ");
    pinta_test_format(L"%-8.2G", L"-1000", L"  -1E+03");
    pinta_test_format(L"%+-8.2G", L"-1000", L"  -1E+03");
    pinta_test_format(L"%0#8.2G", L"-1000", L"-1.E+03 ");
    pinta_test_format(L"%+08.2G", L"-1000", L"-1E+03  ");
    pinta_test_format(L"% 08.2G", L"-1000", L"-1E+03  ");
    pinta_test_format(L"%- #8.2G", L"-1000", L" -1.E+03");

    pinta_test_format(L"%8.3g", L"-0.0000005", L"-5e-07  ");
    pinta_test_format(L"%8.3g", L"-0.005", L"-0.005  ");
    pinta_test_format(L"%8.3g", L"-0.005000", L"-0.005  ");
    pinta_test_format(L"%8.3g", L"-1.111111", L"-1.11   ");
    pinta_test_format(L"%8.3g", L"-157", L"-157    ");
    pinta_test_format(L"%8.3g", L"-15.45e6", L"-1.55e+07");
    pinta_test_format(L"%8.3g", L"-15.78854787e6", L"-1.58e+07");

    pinta_test_format(L"%8.3G", L"-15.45e6", L"-1.55E+07");
    pinta_test_format(L"%8.3G", L"-15.78854787e6", L"-1.58E+07");

    pinta_test_format(L"%8.7g", L"-0.0000005", L"-5e-07  ");
    pinta_test_format(L"%8.7g", L"-0.005", L"-0.005  ");
    pinta_test_format(L"%8.7g", L"-0.005000", L"-0.005  ");
    pinta_test_format(L"%8.7g", L"-1.111111", L"-1.111111");
    pinta_test_format(L"%8.7g", L"-157", L"-157    ");
    pinta_test_format(L"%8.7g", L"-15.45e6", L"-1.545e+07");
    pinta_test_format(L"%8.7g", L"-15.78854787e6", L"-1.578855e+07");

    pinta_test_format(L"%8.0g", L"-0.0000005", L"-5e-07  ");
    pinta_test_format(L"%8.0g", L"-0.005", L"-0.005  ");
    pinta_test_format(L"%8.0g", L"-0.005000", L"-0.005  ");
    pinta_test_format(L"%8.0g", L"-1.111111", L"-1.111111");
    pinta_test_format(L"%8.0g", L"-157", L"-157    ");
    pinta_test_format(L"%8.0g", L"-15.45e6", L"-15450000");
    pinta_test_format(L"%8.0g", L"-15.78854787e6", L"-15788547.87");

    pinta_test_format(L"%8.0G", L"-15.45e6", L"-15450000");
    pinta_test_format(L"%8.0G", L"-15.78854787e6", L"-15788547.87");

    ///* %e */
    pinta_test_format(L"%e", L"0.0000005", L"5.000000e-007");
    pinta_test_format(L"%e", L"0.0000005558874784548784", L"5.600000e-007");
    pinta_test_format(L"%e", L"0.005", L"5.000000e-003");
    pinta_test_format(L"%e", L"0.005000", L"5.000000e-003");
    pinta_test_format(L"%e", L"1.111111", L"1.111111e+000");
    pinta_test_format(L"%e", L"157", L"1.570000e+002");
    pinta_test_format(L"%e", L"15.45e6", L"1.545000e+007");
    pinta_test_format(L"%e", L"15.78854787e6", L"1.578855e+007");

    pinta_test_format(L"%.1e", L"157", L"1.6e+002");

    pinta_test_format(L"%.1E", L"157", L"1.6E+002");

    pinta_test_format(L"%.3e", L"0.0000005", L"5.000e-007");
    pinta_test_format(L"%.3e", L"0.005", L"5.000e-003");
    pinta_test_format(L"%.3e", L"0.005000", L"5.000e-003");
    pinta_test_format(L"%.3e", L"1.111111", L"1.111e+000");
    pinta_test_format(L"%.3e", L"157", L"1.570e+002");
    pinta_test_format(L"%.3e", L"15.45e6", L"1.545e+007");
    pinta_test_format(L"%.3e", L"15.78854787e6", L"1.579e+007");

    pinta_test_format(L"%.3E", L"15.45e6", L"1.545E+007");
    pinta_test_format(L"%.3E", L"15.78854787e6", L"1.579E+007");

    pinta_test_format(L"%.7e", L"0.0000005", L"5.0000000e-007");
    pinta_test_format(L"%.7e", L"0.005", L"5.0000000e-003");
    pinta_test_format(L"%.7e", L"0.005000", L"5.0000000e-003");
    pinta_test_format(L"%.7e", L"1.111111", L"1.1111110e+000");
    pinta_test_format(L"%.7e", L"157", L"1.5700000e+002");
    pinta_test_format(L"%.7e", L"15.45e6", L"1.5450000e+007");
    pinta_test_format(L"%.7e", L"15.78854787e6", L"1.5788548e+007");

    pinta_test_format(L"%.7E", L"15.45e6", L"1.5450000E+007");
    pinta_test_format(L"%.7E", L"15.78854787e6", L"1.5788548E+007");

    pinta_test_format(L"%.0e", L"0.0000005", L"5e-007");
    pinta_test_format(L"%.0e", L"0.005", L"5e-003");
    pinta_test_format(L"%.0e", L"0.005000", L"5e-003");
    pinta_test_format(L"%.0e", L"1.111111", L"1e+000");
    pinta_test_format(L"%.0e", L"157", L"2e+002");
    pinta_test_format(L"%.0e", L"15.45e6", L"2e+007");
    pinta_test_format(L"%.0e", L"15.78854787e6", L"2e+007");

    pinta_test_format(L"%8e", L"0.005", L"5.000000e-003");
    pinta_test_format(L"%8e", L"0.005000", L"5.000000e-003");
    pinta_test_format(L"%8e", L"1.111111", L"1.111111e+000");
    pinta_test_format(L"%8e", L"157", L"1.570000e+002");
    pinta_test_format(L"%#8e", L"157", L"1.570000e+002");
    pinta_test_format(L"%+8e", L"157", L"+1.570000e+002");
    pinta_test_format(L"%-8e", L"157", L"1.570000e+002");
    pinta_test_format(L"%08e", L"157", L"1.570000e+002");
    pinta_test_format(L"% #8e", L"157", L" 1.570000e+002");

    pinta_test_format(L"%15e", L"0.005", L"5.000000e-003  ");
    pinta_test_format(L"%15e", L"0.005000", L"5.000000e-003  ");
    pinta_test_format(L"%15e", L"1.111111", L"1.111111e+000  ");
    pinta_test_format(L"%15e", L"157", L"1.570000e+002  ");
    pinta_test_format(L"%#15e", L"157", L"1.570000e+002  ");
    pinta_test_format(L"%+15e", L"157", L"+1.570000e+002 ");
    pinta_test_format(L"%-15e", L"157", L"  1.570000e+002");
    pinta_test_format(L"%015e", L"157", L"1.570000e+002  ");
    pinta_test_format(L"% #15e", L"157", L" 1.570000e+002 ");

    pinta_test_format(L"%9.1e", L"157", L"1.6e+002 ");
    pinta_test_format(L"%09.1e", L"157", L"1.6e+002 ");
    pinta_test_format(L"%+9.1e", L"157", L"+1.6e+002");
    pinta_test_format(L"%-9.1e", L"157", L" 1.6e+002");
    pinta_test_format(L"%+-9.1e", L"157", L"+1.6e+002");
    pinta_test_format(L"%0#9.1e", L"157", L"1.6e+002 ");
    pinta_test_format(L"%+09.1e", L"157", L"+1.6e+002");
    pinta_test_format(L"% 09.1e", L"157", L" 1.6e+002");
    pinta_test_format(L"%- #9.1e", L"157", L" 1.6e+002");

    pinta_test_format(L"%9.1E", L"157", L"1.6E+002 ");
    pinta_test_format(L"%09.1E", L"157", L"1.6E+002 ");
    pinta_test_format(L"%+9.1E", L"157", L"+1.6E+002");
    pinta_test_format(L"%-9.1E", L"157", L" 1.6E+002");
    pinta_test_format(L"%+-9.1E", L"157", L"+1.6E+002");
    pinta_test_format(L"%0#9.1E", L"157", L"1.6E+002 ");
    pinta_test_format(L"%+09.1E", L"157", L"+1.6E+002");
    pinta_test_format(L"% 09.1E", L"157", L" 1.6E+002");
    pinta_test_format(L"%- #9.1E", L"157", L" 1.6E+002");

    pinta_test_format(L"%11.1e", L"1000", L"1.0e+003   ");
    pinta_test_format(L"%011.1e", L"1000", L"1.0e+003   ");
    pinta_test_format(L"%+11.1e", L"1000", L"+1.0e+003  ");
    pinta_test_format(L"%-11.1e", L"1000", L"   1.0e+003");
    pinta_test_format(L"%+-11.1e", L"1000", L"  +1.0e+003");
    pinta_test_format(L"%0#11.1e", L"1000", L"1.0e+003   ");
    pinta_test_format(L"%+011.1e", L"1000", L"+1.0e+003  ");
    pinta_test_format(L"% 011.1e", L"1000", L" 1.0e+003  ");
    pinta_test_format(L"%- #11.1e", L"1000", L"   1.0e+003");

    pinta_test_format(L"%11.1E", L"1000", L"1.0E+003   ");
    pinta_test_format(L"%011.1E", L"1000", L"1.0E+003   ");
    pinta_test_format(L"%+11.1E", L"1000", L"+1.0E+003  ");
    pinta_test_format(L"%-11.1E", L"1000", L"   1.0E+003");
    pinta_test_format(L"%+-11.1E", L"1000", L"  +1.0E+003");
    pinta_test_format(L"%0#11.1E", L"1000", L"1.0E+003   ");
    pinta_test_format(L"%+011.1E", L"1000", L"+1.0E+003  ");
    pinta_test_format(L"% 011.1E", L"1000", L" 1.0E+003  ");
    pinta_test_format(L"%- #11.1E", L"1000", L"   1.0E+003");

    pinta_test_format(L"%12.2e", L"1000", L"1.00e+003   ");
    pinta_test_format(L"%012.2e", L"1000", L"1.00e+003   ");
    pinta_test_format(L"%+12.2e", L"1000", L"+1.00e+003  ");
    pinta_test_format(L"%-12.2e", L"1000", L"   1.00e+003");
    pinta_test_format(L"%+-12.2e", L"1000", L"  +1.00e+003");
    pinta_test_format(L"%0#12.2e", L"1000", L"1.00e+003   ");
    pinta_test_format(L"%+012.2e", L"1000", L"+1.00e+003  ");
    pinta_test_format(L"% 012.2e", L"1000", L" 1.00e+003  ");
    pinta_test_format(L"%- #12.2e", L"1000", L"   1.00e+003");

    pinta_test_format(L"%12.2E", L"1000", L"1.00E+003   ");
    pinta_test_format(L"%012.2E", L"1000", L"1.00E+003   ");
    pinta_test_format(L"%+12.2E", L"1000", L"+1.00E+003  ");
    pinta_test_format(L"%-12.2E", L"1000", L"   1.00E+003");
    pinta_test_format(L"%+-12.2E", L"1000", L"  +1.00E+003");
    pinta_test_format(L"%0#12.2E", L"1000", L"1.00E+003   ");
    pinta_test_format(L"%+012.2E", L"1000", L"+1.00E+003  ");
    pinta_test_format(L"% 012.2E", L"1000", L" 1.00E+003  ");
    pinta_test_format(L"%- #12.2E", L"1000", L"   1.00E+003");

    pinta_test_format(L"%13.3e", L"0.0000005", L"5.000e-007   ");
    pinta_test_format(L"%13.3e", L"0.005", L"5.000e-003   ");
    pinta_test_format(L"%13.3e", L"0.005000", L"5.000e-003   ");
    pinta_test_format(L"%13.3e", L"1.111111", L"1.111e+000   ");
    pinta_test_format(L"%13.3e", L"157", L"1.570e+002   ");
    pinta_test_format(L"%13.3e", L"15.45e6", L"1.545e+007   ");
    pinta_test_format(L"%13.3e", L"15.78854787e6", L"1.579e+007   ");

    pinta_test_format(L"%13.3E", L"15.45e6", L"1.545E+007   ");
    pinta_test_format(L"%13.3E", L"15.78854787e6", L"1.579E+007   ");

    pinta_test_format(L"%17.7e", L"0.0000005", L"5.0000000e-007   ");
    pinta_test_format(L"%17.7E", L"0.0000005", L"5.0000000E-007   ");

    pinta_test_format(L"%17.7e", L"0.005", L"5.0000000e-003   ");
    pinta_test_format(L"%17.7e", L"0.005000", L"5.0000000e-003   ");
    pinta_test_format(L"%17.7e", L"1.111111", L"1.1111110e+000   ");
    pinta_test_format(L"%17.7e", L"157", L"1.5700000e+002   ");
    pinta_test_format(L"%17.7e", L"15.45e6", L"1.5450000e+007   ");
    pinta_test_format(L"%17.7e", L"15.78854787e6", L"1.5788548e+007   ");

    pinta_test_format(L"%17.7E", L"15.45e6", L"1.5450000E+007   ");
    pinta_test_format(L"%17.7E", L"15.78854787e6", L"1.5788548E+007   ");

    pinta_test_format(L"%9.0e", L"0.0000005", L"5e-007   ");
    pinta_test_format(L"%9.0e", L"0.005", L"5e-003   ");
    pinta_test_format(L"%9.0e", L"0.005000", L"5e-003   ");
    pinta_test_format(L"%#9.0e", L"0.005", L"5.e-003  ");
    pinta_test_format(L"%#9.0e", L"0.005000", L"5.e-003  ");
    pinta_test_format(L"%9.0e", L"1.111111", L"1e+000   ");
    pinta_test_format(L"%9.0e", L"157", L"2e+002   ");
    pinta_test_format(L"%9.0e", L"15.45e6", L"2e+007   ");
    pinta_test_format(L"%9.0e", L"15.78854787e6", L"2e+007   ");

    pinta_test_format(L"%9.0E", L"15.45e6", L"2E+007   ");
    pinta_test_format(L"%9.0E", L"15.78854787e6", L"2E+007   ");

    ///* %e - negative*/
    pinta_test_format(L"%e", L"-0.0000005", L"-5.000000e-007");
    pinta_test_format(L"%e", L"-0.0000005558874784548784", L"-5.600000e-007");
    pinta_test_format(L"%e", L"-0.005", L"-5.000000e-003");
    pinta_test_format(L"%e", L"-0.005000", L"-5.000000e-003");
    pinta_test_format(L"%e", L"-1.111111", L"-1.111111e+000");
    pinta_test_format(L"%e", L"-157", L"-1.570000e+002");
    pinta_test_format(L"%e", L"-15.45e6", L"-1.545000e+007");
    pinta_test_format(L"%e", L"-15.78854787e6", L"-1.578855e+007");

    pinta_test_format(L"%.1e", L"-157", L"-1.6e+002");

    pinta_test_format(L"%.1E", L"-157", L"-1.6E+002");

    pinta_test_format(L"%.3e", L"-0.0000005", L"-5.000e-007");
    pinta_test_format(L"%.3e", L"-0.005", L"-5.000e-003");
    pinta_test_format(L"%.3e", L"-0.005000", L"-5.000e-003");
    pinta_test_format(L"%.3e", L"-1.111111", L"-1.111e+000");
    pinta_test_format(L"%.3e", L"-157", L"-1.570e+002");
    pinta_test_format(L"%.3e", L"-15.45e6", L"-1.545e+007");
    pinta_test_format(L"%.3e", L"-15.78854787e6", L"-1.579e+007");

    pinta_test_format(L"%.3E", L"-15.45e6", L"-1.545E+007");
    pinta_test_format(L"%.3E", L"-15.78854787e6", L"-1.579E+007");

    pinta_test_format(L"%.7e", L"-0.0000005", L"-5.0000000e-007");
    pinta_test_format(L"%.7e", L"-0.005", L"-5.0000000e-003");
    pinta_test_format(L"%.7e", L"-0.005000", L"-5.0000000e-003");
    pinta_test_format(L"%.7e", L"-1.111111", L"-1.1111110e+000");
    pinta_test_format(L"%.7e", L"-157", L"-1.5700000e+002");
    pinta_test_format(L"%.7e", L"-15.45e6", L"-1.5450000e+007");
    pinta_test_format(L"%.7e", L"-15.78854787e6", L"-1.5788548e+007");

    pinta_test_format(L"%.7E", L"-15.45e6", L"-1.5450000E+007");
    pinta_test_format(L"%.7E", L"-15.78854787e6", L"-1.5788548E+007");

    pinta_test_format(L"%.0e", L"-0.0000005", L"-5e-007");
    pinta_test_format(L"%.0e", L"-0.005", L"-5e-003");
    pinta_test_format(L"%.0e", L"-0.005000", L"-5e-003");
    pinta_test_format(L"%.0e", L"-1.111111", L"-1e+000");
    pinta_test_format(L"%.0e", L"-157", L"-2e+002");
    pinta_test_format(L"%.0e", L"-15.45e6", L"-2e+007");
    pinta_test_format(L"%.0e", L"-15.78854787e6", L"-2e+007");

    pinta_test_format(L"%8e", L"-0.005", L"-5.000000e-003");
    pinta_test_format(L"%8e", L"-0.005000", L"-5.000000e-003");
    pinta_test_format(L"%8e", L"-1.111111", L"-1.111111e+000");
    pinta_test_format(L"%8e", L"-157", L"-1.570000e+002");
    pinta_test_format(L"%#8e", L"-157", L"-1.570000e+002");
    pinta_test_format(L"%+8e", L"-157", L"-1.570000e+002");
    pinta_test_format(L"%-8e", L"-157", L"-1.570000e+002");
    pinta_test_format(L"%08e", L"-157", L"-1.570000e+002");
    pinta_test_format(L"% #8e", L"-157", L"-1.570000e+002");

    pinta_test_format(L"%15e", L"-0.005", L"-5.000000e-003 ");
    pinta_test_format(L"%15e", L"-0.005000", L"-5.000000e-003 ");
    pinta_test_format(L"%15e", L"-1.111111", L"-1.111111e+000 ");
    pinta_test_format(L"%15e", L"-157", L"-1.570000e+002 ");
    pinta_test_format(L"%#15e", L"-157", L"-1.570000e+002 ");
    pinta_test_format(L"%+15e", L"-157", L"-1.570000e+002 ");
    pinta_test_format(L"%-15e", L"-157", L" -1.570000e+002");
    pinta_test_format(L"%015e", L"-157", L"-1.570000e+002 ");
    pinta_test_format(L"% #15e", L"-157", L"-1.570000e+002 ");

    pinta_test_format(L"%9.1e", L"-157", L"-1.6e+002");
    pinta_test_format(L"%09.1e", L"-157", L"-1.6e+002");
    pinta_test_format(L"%+9.1e", L"-157", L"-1.6e+002");
    pinta_test_format(L"%-9.1e", L"-157", L"-1.6e+002");
    pinta_test_format(L"%+-9.1e", L"-157", L"-1.6e+002");
    pinta_test_format(L"%0#9.1e", L"-157", L"-1.6e+002");
    pinta_test_format(L"%+09.1e", L"-157", L"-1.6e+002");
    pinta_test_format(L"% 09.1e", L"-157", L"-1.6e+002");
    pinta_test_format(L"%- #9.1e", L"-157", L"-1.6e+002");

    pinta_test_format(L"%9.1E", L"-157", L"-1.6E+002");
    pinta_test_format(L"%09.1E", L"-157", L"-1.6E+002");
    pinta_test_format(L"%+9.1E", L"-157", L"-1.6E+002");
    pinta_test_format(L"%-9.1E", L"-157", L"-1.6E+002");
    pinta_test_format(L"%+-9.1E", L"-157", L"-1.6E+002");
    pinta_test_format(L"%0#9.1E", L"-157", L"-1.6E+002");
    pinta_test_format(L"%+09.1E", L"-157", L"-1.6E+002");
    pinta_test_format(L"% 09.1E", L"-157", L"-1.6E+002");
    pinta_test_format(L"%- #9.1E", L"-157", L"-1.6E+002");

    pinta_test_format(L"%11.1e", L"-1000", L"-1.0e+003  ");
    pinta_test_format(L"%011.1e", L"-1000", L"-1.0e+003  ");
    pinta_test_format(L"%+11.1e", L"-1000", L"-1.0e+003  ");
    pinta_test_format(L"%-11.1e", L"-1000", L"  -1.0e+003");
    pinta_test_format(L"%+-11.1e", L"-1000", L"  -1.0e+003");
    pinta_test_format(L"%0#11.1e", L"-1000", L"-1.0e+003  ");
    pinta_test_format(L"%+011.1e", L"-1000", L"-1.0e+003  ");
    pinta_test_format(L"% 011.1e", L"-1000", L"-1.0e+003  ");
    pinta_test_format(L"%- #11.1e", L"-1000", L"  -1.0e+003");

    pinta_test_format(L"%11.1E", L"-1000", L"-1.0E+003  ");
    pinta_test_format(L"%011.1E", L"-1000", L"-1.0E+003  ");
    pinta_test_format(L"%+11.1E", L"-1000", L"-1.0E+003  ");
    pinta_test_format(L"%-11.1E", L"-1000", L"  -1.0E+003");
    pinta_test_format(L"%+-11.1E", L"-1000", L"  -1.0E+003");
    pinta_test_format(L"%0#11.1E", L"-1000", L"-1.0E+003  ");
    pinta_test_format(L"%+011.1E", L"-1000", L"-1.0E+003  ");
    pinta_test_format(L"% 011.1E", L"-1000", L"-1.0E+003  ");
    pinta_test_format(L"%- #11.1E", L"-1000", L"  -1.0E+003");

    pinta_test_format(L"%12.2e", L"-1000", L"-1.00e+003  ");
    pinta_test_format(L"%012.2e", L"-1000", L"-1.00e+003  ");
    pinta_test_format(L"%+12.2e", L"-1000", L"-1.00e+003  ");
    pinta_test_format(L"%-12.2e", L"-1000", L"  -1.00e+003");
    pinta_test_format(L"%+-12.2e", L"-1000", L"  -1.00e+003");
    pinta_test_format(L"%0#12.2e", L"-1000", L"-1.00e+003  ");
    pinta_test_format(L"%+012.2e", L"-1000", L"-1.00e+003  ");
    pinta_test_format(L"% 012.2e", L"-1000", L"-1.00e+003  ");
    pinta_test_format(L"%- #12.2e", L"-1000", L"  -1.00e+003");

    pinta_test_format(L"%12.2E", L"-1000", L"-1.00E+003  ");
    pinta_test_format(L"%012.2E", L"-1000", L"-1.00E+003  ");
    pinta_test_format(L"%+12.2E", L"-1000", L"-1.00E+003  ");
    pinta_test_format(L"%-12.2E", L"-1000", L"  -1.00E+003");
    pinta_test_format(L"%+-12.2E", L"-1000", L"  -1.00E+003");
    pinta_test_format(L"%0#12.2E", L"-1000", L"-1.00E+003  ");
    pinta_test_format(L"%+012.2E", L"-1000", L"-1.00E+003  ");
    pinta_test_format(L"% 012.2E", L"-1000", L"-1.00E+003  ");
    pinta_test_format(L"%- #12.2E", L"-1000", L"  -1.00E+003");

    pinta_test_format(L"%13.3e", L"-0.0000005", L"-5.000e-007  ");
    pinta_test_format(L"%13.3e", L"-0.005", L"-5.000e-003  ");
    pinta_test_format(L"%13.3e", L"-0.005000", L"-5.000e-003  ");
    pinta_test_format(L"%13.3e", L"-1.111111", L"-1.111e+000  ");
    pinta_test_format(L"%13.3e", L"-157", L"-1.570e+002  ");
    pinta_test_format(L"%13.3e", L"-15.45e6", L"-1.545e+007  ");
    pinta_test_format(L"%13.3e", L"-15.78854787e6", L"-1.579e+007  ");

    pinta_test_format(L"%13.3E", L"-15.45e6", L"-1.545E+007  ");
    pinta_test_format(L"%13.3E", L"-15.78854787e6", L"-1.579E+007  ");

    pinta_test_format(L"%17.7e", L"-0.0000005", L"-5.0000000e-007  ");
    pinta_test_format(L"%17.7E", L"-0.0000005", L"-5.0000000E-007  ");

    pinta_test_format(L"%17.7e", L"-0.005", L"-5.0000000e-003  ");
    pinta_test_format(L"%17.7e", L"-0.005000", L"-5.0000000e-003  ");
    pinta_test_format(L"%17.7e", L"-1.111111", L"-1.1111110e+000  ");
    pinta_test_format(L"%17.7e", L"-157", L"-1.5700000e+002  ");
    pinta_test_format(L"%17.7e", L"-15.45e6", L"-1.5450000e+007  ");
    pinta_test_format(L"%17.7e", L"-15.78854787e6", L"-1.5788548e+007  ");

    pinta_test_format(L"%17.7E", L"-15.45e6", L"-1.5450000E+007  ");
    pinta_test_format(L"%17.7E", L"-15.78854787e6", L"-1.5788548E+007  ");

    pinta_test_format(L"%9.0e", L"-0.0000005", L"-5e-007  ");
    pinta_test_format(L"%9.0e", L"-0.005", L"-5e-003  ");
    pinta_test_format(L"%9.0e", L"-0.005000", L"-5e-003  ");
    pinta_test_format(L"%#9.0e", L"-0.005", L"-5.e-003 ");
    pinta_test_format(L"%#9.0e", L"-0.005000", L"-5.e-003 ");
    pinta_test_format(L"%9.0e", L"-1.111111", L"-1e+000  ");
    pinta_test_format(L"%9.0e", L"-157", L"-2e+002  ");
    pinta_test_format(L"%9.0e", L"-15.45e6", L"-2e+007  ");
    pinta_test_format(L"%9.0e", L"-15.78854787e6", L"-2e+007  ");

    pinta_test_format(L"%9.0E", L"-15.45e6", L"-2E+007  ");
    pinta_test_format(L"%9.0E", L"-15.78854787e6", L"-2E+007  ");

    pinta_dispose_tests();
}

void pinta_tests_format()
{
    sput_enter_suite("Format tests");
    sput_run_test(pinta_test_format_strings);
    sput_run_test(format_multiple_arguments);
    sput_leave_suite();
}
