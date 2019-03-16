#include "pinta_tests.h"

u8 decimal_test_parse(wchar *string, decimal *result)
{
    return decimal_from_string(string, wcslen(string), result);
}

static decimal decimal_test_make_decimal(wchar *string)
{
    u8 is_negative = 0;
    decimal result = 0;
    decimal scale = PINTA_DECIMAL_SCALE;

    if (*string == L'-' || *string == L'+')
    {
        is_negative = *string == L'-';
        string++;
    }

    while (*string >= L'0' && *string <= L'9')
    {
        result = result * 10 + *string - L'0';
        string++;
    }

    if (*string == '.')
    {
        string++;

        while (*string >= L'0' && *string <= L'9' && scale > 1)
        {
            result = result * 10 + *string - L'0';

            scale = scale / 10;
            string++;
        }
    }

    result = result * scale;

    if (is_negative)
        result = -result;

    return result;
}

static void decimal_test_format_general(wchar *number, i32 precision, wchar *expected)
{
    decimal value;
    wchar buf[30];

    if (!decimal_test_parse(number, &value))
        sput_fail_if(1, pinta_tests_message(L"Format general: failed to parse input number %ls", number));

    pinta_format_decimal_general(value, precision, buf);

    sput_fail_if(wcscmp(expected, buf), pinta_tests_message(L"Format general: expected - %ls, actual - %ls", expected, buf));
}

static void decimal_test_format_decimal(wchar *number, i32 precision, wchar *expected)
{
    decimal value;
    wchar buf[30];

    if (!decimal_test_parse(number, &value))
        sput_fail_if(1, pinta_tests_message(L"Format decimal: failed to parse input number %ls", number));

    pinta_format_decimal_fixed(value, precision, buf);

    sput_fail_if(wcscmp(expected, buf), pinta_tests_message(L"Format decimal: expected - %ls, actual - %ls", expected, buf));
}

static void decimal_test_format_exponential(wchar *number, i32 precision, wchar *expected)
{
    decimal value;
    wchar buf[30];

    if (!decimal_test_parse(number, &value))
        sput_fail_if(1, pinta_tests_message(L"Format exp: failed to parse input number %ls", number));

    pinta_format_decimal_exponential(value, precision, buf);

    sput_fail_if(wcscmp(expected, buf), pinta_tests_message(L"Format exp (%d): Expected - %ls, actual - %ls", precision, expected, buf));
}

#define MAKE_DECIMAL(val) decimal_test_make_decimal(L ## val)

void decimal_from_string_tests()
{
    /*
    var numbers = new[]
    {
    "1000e-10",
    "1000e4",
    "0.001000",
    "0.001000e-2",
    "0.001000e-10",
    "0.001000e4",
    "1000.001000",
    "1000.001000e-10",
    "1000.001000e4",
    "-1000",
    "-1000e-10",
    "-1000e4",
    "-0.001000",
    "-0.001000e-10",
    "-0.001000e4",
    "-1000.001000",
    "-1000.001000e-10",
    "-1000.001000e4",
    "0001000",
    "0001000e-10",
    "0001000e4",
    "0000.001000",
    "0000.001000e-10",
    "0000.001000e4",
    "0001000.001000",
    "0001000.001000e-10",
    "0001000.001000e4",
    "-0001000",
    "-0001000e-10",
    "-0001000e4",
    "-0000.001000",
    "-0000.001000e-10",
    "-0000.001000e4",
    "-0001000.001000",
    "-0001000.001000e-10",
    "-0001000.001000e4",
    "123456789123456789.123456789e-20",
    "0.123456789123456789123456789e8",
    };
    */

    decimal result;

    sput_fail_unless(decimal_test_parse(L"1000e-10", &result), "decimal_parse('1000e-10') failed");
    sput_fail_if(MAKE_DECIMAL("0.0000001") != result, "decimal_parse('1000e-10') != 0.0000001");

    sput_fail_unless(decimal_test_parse(L"1000e4", &result), "decimal_parse('1000e4') failed");
    sput_fail_if(MAKE_DECIMAL("10000000") != result, "decimal_parse('1000e4') != 10000000");

    sput_fail_unless(decimal_test_parse(L"0.001000", &result), "decimal_parse('0.001000') failed");
    sput_fail_if(MAKE_DECIMAL("0.001") != result, "decimal_parse('0.001000') != 0.001");

    sput_fail_unless(decimal_test_parse(L"0.001000e-2", &result), "decimal_parse('0.001000e-2') failed");
    sput_fail_if(MAKE_DECIMAL("0.00001") != result, "decimal_parse('0.001000e-2') != 0.00001");

    sput_fail_unless(decimal_test_parse(L"0.001000e-10", &result), "decimal_parse('0.001000e-10') failed");
    sput_fail_if(MAKE_DECIMAL("0") != result, "decimal_parse('0.001000e-10') != 0");

    sput_fail_unless(decimal_test_parse(L"0.001000e4", &result), "decimal_parse('0.001000e4') failed");
    sput_fail_if(MAKE_DECIMAL("10") != result, "decimal_parse('0.001000e4') != 10");

    sput_fail_unless(decimal_test_parse(L"1000.001000", &result), "decimal_parse('1000.001000') failed");
    sput_fail_if(MAKE_DECIMAL("1000.001") != result, "decimal_parse('1000.001000') != 1000.001");

    sput_fail_unless(decimal_test_parse(L"1000.001000e-10", &result), "decimal_parse('1000.001000e-10') failed");
    sput_fail_if(MAKE_DECIMAL("0.0000001") != result, "decimal_parse('1000.001000e-10') != 0.0000001");

    sput_fail_unless(decimal_test_parse(L"1000.001000e4", &result), "decimal_parse('1000.001000e4') failed");
    sput_fail_if(MAKE_DECIMAL("10000010") != result, "decimal_parse('1000.001000e4') != 10000010");

    sput_fail_unless(decimal_test_parse(L"-1000", &result), "decimal_parse('-1000') failed");
    sput_fail_if(MAKE_DECIMAL("-1000") != result, "decimal_parse('-1000') != -1000");

    sput_fail_unless(decimal_test_parse(L"-1000e-10", &result), "decimal_parse('-1000e-10') failed");
    sput_fail_if(MAKE_DECIMAL("-0.0000001") != result, "decimal_parse('-1000e-10') != -0.0000001");

    sput_fail_unless(decimal_test_parse(L"-1000e4", &result), "decimal_parse('-1000e4') failed");
    sput_fail_if(MAKE_DECIMAL("-10000000") != result, "decimal_parse('-1000e4') != -10000000");

    sput_fail_unless(decimal_test_parse(L"-0.001000", &result), "decimal_parse('-0.001000') failed");
    sput_fail_if(MAKE_DECIMAL("-0.001") != result, "decimal_parse('-0.001000') != -0.001");

    sput_fail_unless(decimal_test_parse(L"-0.001000e-10", &result), "decimal_parse('-0.001000e-10') failed");
    sput_fail_if(MAKE_DECIMAL("0") != result, "decimal_parse('-0.001000e-10') != 0");

    sput_fail_unless(decimal_test_parse(L"-0.001000e4", &result), "decimal_parse('-0.001000e4') failed");
    sput_fail_if(MAKE_DECIMAL("-10") != result, "decimal_parse('-0.001000e4') != -10");

    sput_fail_unless(decimal_test_parse(L"-1000.001000", &result), "decimal_parse('-1000.001000') failed");
    sput_fail_if(MAKE_DECIMAL("-1000.001") != result, "decimal_parse('-1000.001000') != -1000.001");

    sput_fail_unless(decimal_test_parse(L"-1000.001000e-10", &result), "decimal_parse('-1000.001000e-10') failed");
    sput_fail_if(MAKE_DECIMAL("-0.0000001") != result, "decimal_parse('-1000.001000e-10') != -0.0000001");

    sput_fail_unless(decimal_test_parse(L"-1000.001000e4", &result), "decimal_parse('-1000.001000e4') failed");
    sput_fail_if(MAKE_DECIMAL("-10000010") != result, "decimal_parse('-1000.001000e4') != -10000010");

    sput_fail_unless(decimal_test_parse(L"0001000", &result), "decimal_parse('0001000') failed");
    sput_fail_if(MAKE_DECIMAL("1000") != result, "decimal_parse('0001000') != 1000");

    sput_fail_unless(decimal_test_parse(L"0001000e-10", &result), "decimal_parse('0001000e-10') failed");
    sput_fail_if(MAKE_DECIMAL("0.0000001") != result, "decimal_parse('0001000e-10') != 0.0000001");

    sput_fail_unless(decimal_test_parse(L"0001000e4", &result), "decimal_parse('0001000e4') failed");
    sput_fail_if(MAKE_DECIMAL("10000000") != result, "decimal_parse('0001000e4') != 10000000");

    sput_fail_unless(decimal_test_parse(L"0000.001000", &result), "decimal_parse('0000.001000') failed");
    sput_fail_if(MAKE_DECIMAL("0.001") != result, "decimal_parse('0000.001000') != 0.001");

    sput_fail_unless(decimal_test_parse(L"0000.001000e-10", &result), "decimal_parse('0000.001000e-10') failed");
    sput_fail_if(MAKE_DECIMAL("0") != result, "decimal_parse('0000.001000e-10') != 0");

    sput_fail_unless(decimal_test_parse(L"0000.001000e4", &result), "decimal_parse('0000.001000e4') failed");
    sput_fail_if(MAKE_DECIMAL("10") != result, "decimal_parse('0000.001000e4') != 10");

    sput_fail_unless(decimal_test_parse(L"0001000.001000", &result), "decimal_parse('0001000.001000') failed");
    sput_fail_if(MAKE_DECIMAL("1000.001") != result, "decimal_parse('0001000.001000') != 1000.001");

    sput_fail_unless(decimal_test_parse(L"0001000.001000e-10", &result), "decimal_parse('0001000.001000e-10') failed");
    sput_fail_if(MAKE_DECIMAL("0.0000001") != result, "decimal_parse('0001000.001000e-10') != 0.0000001");

    sput_fail_unless(decimal_test_parse(L"0001000.001000e4", &result), "decimal_parse('0001000.001000e4') failed");
    sput_fail_if(MAKE_DECIMAL("10000010") != result, "decimal_parse('0001000.001000e4') != 10000010");

    sput_fail_unless(decimal_test_parse(L"-0001000", &result), "decimal_parse('-0001000') failed");
    sput_fail_if(MAKE_DECIMAL("-1000") != result, "decimal_parse('-0001000') != -1000");

    sput_fail_unless(decimal_test_parse(L"-0001000e-10", &result), "decimal_parse('-0001000e-10') failed");
    sput_fail_if(MAKE_DECIMAL("-0.0000001") != result, "decimal_parse('-0001000e-10') != -0.0000001");

    sput_fail_unless(decimal_test_parse(L"-0001000e4", &result), "decimal_parse('-0001000e4') failed");
    sput_fail_if(MAKE_DECIMAL("-10000000") != result, "decimal_parse('-0001000e4') != -10000000");

    sput_fail_unless(decimal_test_parse(L"-0000.001000", &result), "decimal_parse('-0000.001000') failed");
    sput_fail_if(MAKE_DECIMAL("-0.001") != result, "decimal_parse('-0000.001000') != -0.001");

    sput_fail_unless(decimal_test_parse(L"-0000.001000e-10", &result), "decimal_parse('-0000.001000e-10') failed");
    sput_fail_if(MAKE_DECIMAL("0") != result, "decimal_parse('-0000.001000e-10') != 0");

    sput_fail_unless(decimal_test_parse(L"-0000.001000e4", &result), "decimal_parse('-0000.001000e4') failed");
    sput_fail_if(MAKE_DECIMAL("-10") != result, "decimal_parse('-0000.001000e4') != -10");

    sput_fail_unless(decimal_test_parse(L"-0001000.001000", &result), "decimal_parse('-0001000.001000') failed");
    sput_fail_if(MAKE_DECIMAL("-1000.001") != result, "decimal_parse('-0001000.001000') != -1000.001");

    sput_fail_unless(decimal_test_parse(L"-0001000.001000e-10", &result), "decimal_parse('-0001000.001000e-10') failed");
    sput_fail_if(MAKE_DECIMAL("-0.0000001") != result, "decimal_parse('-0001000.001000e-10') != -0.0000001");

    sput_fail_unless(decimal_test_parse(L"-0001000.001000e4", &result), "decimal_parse('-0001000.001000e4') failed");
    sput_fail_if(MAKE_DECIMAL("-10000010") != result, "decimal_parse('-0001000.001000e4') != -10000010");

    sput_fail_unless(decimal_test_parse(L"123456789123456789123456789e-20", &result), "decimal_parse('123456789123456789123456789e-20') failed");
    sput_fail_if(MAKE_DECIMAL("1234567.89123457") != result, "decimal_parse('123456789123456789123456789e-20') != 1234567.89123457");

    sput_fail_unless(decimal_test_parse(L"0.123456789123456789123456789", &result), "decimal_parse('0.123456789123456789123456789') failed");
    sput_fail_if(MAKE_DECIMAL("0.12345679") != result, "decimal_parse('0.123456789123456789123456789') != 0.12345679");

    sput_fail_unless(decimal_test_parse(L"123456789123456789.123456789e-20", &result), "decimal_parse('123456789123456789.123456789e-20') failed");
    sput_fail_if(MAKE_DECIMAL("0.00123457") != result, "decimal_parse('123456789123456789.123456789e-20') != 0.00123457");

    sput_fail_unless(decimal_test_parse(L"0.123456789123456789123456789e8", &result), "decimal_parse('0.123456789123456789123456789e8') failed");
    sput_fail_if(MAKE_DECIMAL("12345678.91234568") != result, "decimal_parse('0.123456789123456789123456789e8') != 12345678.91234568");

    sput_fail_unless(decimal_test_parse(L"0.0000000000000000123456789123456789123456789e24", &result), "decimal_parse('0.0000000000000000123456789123456789123456789e24') failed");
    sput_fail_if(MAKE_DECIMAL("12345678.91234568") != result, "decimal_parse('0.0000000000000000123456789123456789123456789e24') != 12345678.91234568");

    sput_fail_if(decimal_test_parse(L"0.123456789123456789123456789e9", &result), "decimal_parse('0.123456789123456789123456789e9') succeeded");
}

void decimal_format_general_tests()
{
    decimal_test_format_general(L"0", 0, L"0");

    decimal_test_format_general(L"12345678.91234567", 0, L"12345678.91234567");
    decimal_test_format_general(L"12345678.91234567", 1, L"1E+07");
    decimal_test_format_general(L"12345678.91234567", 2, L"1.2E+07");
    decimal_test_format_general(L"12345678.91234567", 3, L"1.23E+07");
    decimal_test_format_general(L"12345678.91234567", 4, L"1.235E+07");
    decimal_test_format_general(L"12345678.91234567", 5, L"1.2346E+07");
    decimal_test_format_general(L"12345678.91234567", 6, L"1.23457E+07");
    decimal_test_format_general(L"12345678.91234567", 7, L"1.234568E+07");
    decimal_test_format_general(L"12345678.91234567", 8, L"12345679");
    decimal_test_format_general(L"12345678.91234567", 9, L"12345678.9");
    decimal_test_format_general(L"12345678.91234567", 10, L"12345678.91");
    decimal_test_format_general(L"12345678.91234567", 11, L"12345678.912");
    decimal_test_format_general(L"12345678.91234567", 12, L"12345678.9123");
    decimal_test_format_general(L"12345678.91234567", 13, L"12345678.91235");
    decimal_test_format_general(L"12345678.91234567", 14, L"12345678.912346");
    decimal_test_format_general(L"12345678.91234567", 15, L"12345678.9123457");
    decimal_test_format_general(L"12345678.91234567", 16, L"12345678.91234567");
    decimal_test_format_general(L"1234567.89123457", 0, L"1234567.89123457");
    decimal_test_format_general(L"1234567.89123457", 1, L"1E+06");
    decimal_test_format_general(L"1234567.89123457", 2, L"1.2E+06");
    decimal_test_format_general(L"1234567.89123457", 3, L"1.23E+06");
    decimal_test_format_general(L"1234567.89123457", 4, L"1.235E+06");
    decimal_test_format_general(L"1234567.89123457", 5, L"1.2346E+06");
    decimal_test_format_general(L"1234567.89123457", 6, L"1.23457E+06");
    decimal_test_format_general(L"1234567.89123457", 7, L"1234568");
    decimal_test_format_general(L"1234567.89123457", 8, L"1234567.9");
    decimal_test_format_general(L"1234567.89123457", 9, L"1234567.89");
    decimal_test_format_general(L"1234567.89123457", 10, L"1234567.891");
    decimal_test_format_general(L"1234567.89123457", 11, L"1234567.8912");
    decimal_test_format_general(L"1234567.89123457", 12, L"1234567.89123");
    decimal_test_format_general(L"1234567.89123457", 13, L"1234567.891235");
    decimal_test_format_general(L"1234567.89123457", 14, L"1234567.8912346");
    decimal_test_format_general(L"1234567.89123457", 15, L"1234567.89123457");
    decimal_test_format_general(L"1234567.89123457", 16, L"1234567.89123457");
    decimal_test_format_general(L"123456.78912346", 0, L"123456.78912346");
    decimal_test_format_general(L"123456.78912346", 1, L"1E+05");
    decimal_test_format_general(L"123456.78912346", 2, L"1.2E+05");
    decimal_test_format_general(L"123456.78912346", 3, L"1.23E+05");
    decimal_test_format_general(L"123456.78912346", 4, L"1.235E+05");
    decimal_test_format_general(L"123456.78912346", 5, L"1.2346E+05");
    decimal_test_format_general(L"123456.78912346", 6, L"123457");
    decimal_test_format_general(L"123456.78912346", 7, L"123456.8");
    decimal_test_format_general(L"123456.78912346", 8, L"123456.79");
    decimal_test_format_general(L"123456.78912346", 9, L"123456.789");
    decimal_test_format_general(L"123456.78912346", 10, L"123456.7891");
    decimal_test_format_general(L"123456.78912346", 11, L"123456.78912");
    decimal_test_format_general(L"123456.78912346", 12, L"123456.789123");
    decimal_test_format_general(L"123456.78912346", 13, L"123456.7891235");
    decimal_test_format_general(L"123456.78912346", 14, L"123456.78912346");
    decimal_test_format_general(L"123456.78912346", 15, L"123456.78912346");
    decimal_test_format_general(L"123456.78912346", 16, L"123456.78912346");
    decimal_test_format_general(L"12345.67891235", 0, L"12345.67891235");
    decimal_test_format_general(L"12345.67891235", 1, L"1E+04");
    decimal_test_format_general(L"12345.67891235", 2, L"1.2E+04");
    decimal_test_format_general(L"12345.67891235", 3, L"1.23E+04");
    decimal_test_format_general(L"12345.67891235", 4, L"1.235E+04");
    decimal_test_format_general(L"12345.67891235", 5, L"12346");
    decimal_test_format_general(L"12345.67891235", 6, L"12345.7");
    decimal_test_format_general(L"12345.67891235", 7, L"12345.68");
    decimal_test_format_general(L"12345.67891235", 8, L"12345.679");
    decimal_test_format_general(L"12345.67891235", 9, L"12345.6789");
    decimal_test_format_general(L"12345.67891235", 10, L"12345.67891");
    decimal_test_format_general(L"12345.67891235", 11, L"12345.678912");
    decimal_test_format_general(L"12345.67891235", 12, L"12345.6789124");
    decimal_test_format_general(L"12345.67891235", 13, L"12345.67891235");
    decimal_test_format_general(L"12345.67891235", 14, L"12345.67891235");
    decimal_test_format_general(L"12345.67891235", 15, L"12345.67891235");
    decimal_test_format_general(L"12345.67891235", 16, L"12345.67891235");
    decimal_test_format_general(L"1234.56789123", 0, L"1234.56789123");
    decimal_test_format_general(L"1234.56789123", 1, L"1E+03");
    decimal_test_format_general(L"1234.56789123", 2, L"1.2E+03");
    decimal_test_format_general(L"1234.56789123", 3, L"1.23E+03");
    decimal_test_format_general(L"1234.56789123", 4, L"1235");
    decimal_test_format_general(L"1234.56789123", 5, L"1234.6");
    decimal_test_format_general(L"1234.56789123", 6, L"1234.57");
    decimal_test_format_general(L"1234.56789123", 7, L"1234.568");
    decimal_test_format_general(L"1234.56789123", 8, L"1234.5679");
    decimal_test_format_general(L"1234.56789123", 9, L"1234.56789");
    decimal_test_format_general(L"1234.56789123", 10, L"1234.567891");
    decimal_test_format_general(L"1234.56789123", 11, L"1234.5678912");
    decimal_test_format_general(L"1234.56789123", 12, L"1234.56789123");
    decimal_test_format_general(L"1234.56789123", 13, L"1234.56789123");
    decimal_test_format_general(L"1234.56789123", 14, L"1234.56789123");
    decimal_test_format_general(L"1234.56789123", 15, L"1234.56789123");
    decimal_test_format_general(L"1234.56789123", 16, L"1234.56789123");
    decimal_test_format_general(L"123.45678912", 0, L"123.45678912");
    decimal_test_format_general(L"123.45678912", 1, L"1E+02");
    decimal_test_format_general(L"123.45678912", 2, L"1.2E+02");
    decimal_test_format_general(L"123.45678912", 3, L"123");
    decimal_test_format_general(L"123.45678912", 4, L"123.5");
    decimal_test_format_general(L"123.45678912", 5, L"123.46");
    decimal_test_format_general(L"123.45678912", 6, L"123.457");
    decimal_test_format_general(L"123.45678912", 7, L"123.4568");
    decimal_test_format_general(L"123.45678912", 8, L"123.45679");
    decimal_test_format_general(L"123.45678912", 9, L"123.456789");
    decimal_test_format_general(L"123.45678912", 10, L"123.4567891");
    decimal_test_format_general(L"123.45678912", 11, L"123.45678912");
    decimal_test_format_general(L"123.45678912", 12, L"123.45678912");
    decimal_test_format_general(L"123.45678912", 13, L"123.45678912");
    decimal_test_format_general(L"123.45678912", 14, L"123.45678912");
    decimal_test_format_general(L"123.45678912", 15, L"123.45678912");
    decimal_test_format_general(L"123.45678912", 16, L"123.45678912");
    decimal_test_format_general(L"12.34567891", 0, L"12.34567891");
    decimal_test_format_general(L"12.34567891", 1, L"1E+01");
    decimal_test_format_general(L"12.34567891", 2, L"12");
    decimal_test_format_general(L"12.34567891", 3, L"12.3");
    decimal_test_format_general(L"12.34567891", 4, L"12.35");
    decimal_test_format_general(L"12.34567891", 5, L"12.346");
    decimal_test_format_general(L"12.34567891", 6, L"12.3457");
    decimal_test_format_general(L"12.34567891", 7, L"12.34568");
    decimal_test_format_general(L"12.34567891", 8, L"12.345679");
    decimal_test_format_general(L"12.34567891", 9, L"12.3456789");
    decimal_test_format_general(L"12.34567891", 10, L"12.34567891");
    decimal_test_format_general(L"12.34567891", 11, L"12.34567891");
    decimal_test_format_general(L"12.34567891", 12, L"12.34567891");
    decimal_test_format_general(L"12.34567891", 13, L"12.34567891");
    decimal_test_format_general(L"12.34567891", 14, L"12.34567891");
    decimal_test_format_general(L"12.34567891", 15, L"12.34567891");
    decimal_test_format_general(L"12.34567891", 16, L"12.34567891");
    decimal_test_format_general(L"1.23456789", 0, L"1.23456789");
    decimal_test_format_general(L"1.23456789", 1, L"1");
    decimal_test_format_general(L"1.23456789", 2, L"1.2");
    decimal_test_format_general(L"1.23456789", 3, L"1.23");
    decimal_test_format_general(L"1.23456789", 4, L"1.235");
    decimal_test_format_general(L"1.23456789", 5, L"1.2346");
    decimal_test_format_general(L"1.23456789", 6, L"1.23457");
    decimal_test_format_general(L"1.23456789", 7, L"1.234568");
    decimal_test_format_general(L"1.23456789", 8, L"1.2345679");
    decimal_test_format_general(L"1.23456789", 9, L"1.23456789");
    decimal_test_format_general(L"1.23456789", 10, L"1.23456789");
    decimal_test_format_general(L"1.23456789", 11, L"1.23456789");
    decimal_test_format_general(L"1.23456789", 12, L"1.23456789");
    decimal_test_format_general(L"1.23456789", 13, L"1.23456789");
    decimal_test_format_general(L"1.23456789", 14, L"1.23456789");
    decimal_test_format_general(L"1.23456789", 15, L"1.23456789");
    decimal_test_format_general(L"1.23456789", 16, L"1.23456789");
    decimal_test_format_general(L"0.12345679", 0, L"0.12345679");
    decimal_test_format_general(L"0.12345679", 1, L"0.1");
    decimal_test_format_general(L"0.12345679", 2, L"0.12");
    decimal_test_format_general(L"0.12345679", 3, L"0.123");
    decimal_test_format_general(L"0.12345679", 4, L"0.1235");
    decimal_test_format_general(L"0.12345679", 5, L"0.12346");
    decimal_test_format_general(L"0.12345679", 6, L"0.123457");
    decimal_test_format_general(L"0.12345679", 7, L"0.1234568");
    decimal_test_format_general(L"0.12345679", 8, L"0.12345679");
    decimal_test_format_general(L"0.12345679", 9, L"0.12345679");
    decimal_test_format_general(L"0.12345679", 10, L"0.12345679");
    decimal_test_format_general(L"0.12345679", 11, L"0.12345679");
    decimal_test_format_general(L"0.12345679", 12, L"0.12345679");
    decimal_test_format_general(L"0.12345679", 13, L"0.12345679");
    decimal_test_format_general(L"0.12345679", 14, L"0.12345679");
    decimal_test_format_general(L"0.12345679", 15, L"0.12345679");
    decimal_test_format_general(L"0.12345679", 16, L"0.12345679");
    decimal_test_format_general(L"0.01234568", 0, L"0.01234568");
    decimal_test_format_general(L"0.01234568", 1, L"0.01");
    decimal_test_format_general(L"0.01234568", 2, L"0.012");
    decimal_test_format_general(L"0.01234568", 3, L"0.0123");
    decimal_test_format_general(L"0.01234568", 4, L"0.01235");
    decimal_test_format_general(L"0.01234568", 5, L"0.012346");
    decimal_test_format_general(L"0.01234568", 6, L"0.0123457");
    decimal_test_format_general(L"0.01234568", 7, L"0.01234568");
    decimal_test_format_general(L"0.01234568", 8, L"0.01234568");
    decimal_test_format_general(L"0.01234568", 9, L"0.01234568");
    decimal_test_format_general(L"0.01234568", 10, L"0.01234568");
    decimal_test_format_general(L"0.01234568", 11, L"0.01234568");
    decimal_test_format_general(L"0.01234568", 12, L"0.01234568");
    decimal_test_format_general(L"0.01234568", 13, L"0.01234568");
    decimal_test_format_general(L"0.01234568", 14, L"0.01234568");
    decimal_test_format_general(L"0.01234568", 15, L"0.01234568");
    decimal_test_format_general(L"0.01234568", 16, L"0.01234568");
    decimal_test_format_general(L"0.00123457", 0, L"0.00123457");
    decimal_test_format_general(L"0.00123457", 1, L"0.001");
    decimal_test_format_general(L"0.00123457", 2, L"0.0012");
    decimal_test_format_general(L"0.00123457", 3, L"0.00123");
    decimal_test_format_general(L"0.00123457", 4, L"0.001235");
    decimal_test_format_general(L"0.00123457", 5, L"0.0012346");
    decimal_test_format_general(L"0.00123457", 6, L"0.00123457");
    decimal_test_format_general(L"0.00123457", 7, L"0.00123457");
    decimal_test_format_general(L"0.00123457", 8, L"0.00123457");
    decimal_test_format_general(L"0.00123457", 9, L"0.00123457");
    decimal_test_format_general(L"0.00123457", 10, L"0.00123457");
    decimal_test_format_general(L"0.00123457", 11, L"0.00123457");
    decimal_test_format_general(L"0.00123457", 12, L"0.00123457");
    decimal_test_format_general(L"0.00123457", 13, L"0.00123457");
    decimal_test_format_general(L"0.00123457", 14, L"0.00123457");
    decimal_test_format_general(L"0.00123457", 15, L"0.00123457");
    decimal_test_format_general(L"0.00123457", 16, L"0.00123457");
    decimal_test_format_general(L"0.00012346", 0, L"0.00012346");
    decimal_test_format_general(L"0.00012346", 1, L"0.0001");
    decimal_test_format_general(L"0.00012346", 2, L"0.00012");
    decimal_test_format_general(L"0.00012346", 3, L"0.000123");
    decimal_test_format_general(L"0.00012346", 4, L"0.0001235");
    decimal_test_format_general(L"0.00012346", 5, L"0.00012346");
    decimal_test_format_general(L"0.00012346", 6, L"0.00012346");
    decimal_test_format_general(L"0.00012346", 7, L"0.00012346");
    decimal_test_format_general(L"0.00012346", 8, L"0.00012346");
    decimal_test_format_general(L"0.00012346", 9, L"0.00012346");
    decimal_test_format_general(L"0.00012346", 10, L"0.00012346");
    decimal_test_format_general(L"0.00012346", 11, L"0.00012346");
    decimal_test_format_general(L"0.00012346", 12, L"0.00012346");
    decimal_test_format_general(L"0.00012346", 13, L"0.00012346");
    decimal_test_format_general(L"0.00012346", 14, L"0.00012346");
    decimal_test_format_general(L"0.00012346", 15, L"0.00012346");
    decimal_test_format_general(L"0.00012346", 16, L"0.00012346");
    decimal_test_format_general(L"0.00001235", 0, L"0.00001235");
    decimal_test_format_general(L"0.00001235", 1, L"0.00001");
    decimal_test_format_general(L"0.00001235", 2, L"0.000012");
    decimal_test_format_general(L"0.00001235", 3, L"0.0000124");
    decimal_test_format_general(L"0.00001235", 4, L"0.00001235");
    decimal_test_format_general(L"0.00001235", 5, L"0.00001235");
    decimal_test_format_general(L"0.00001235", 6, L"0.00001235");
    decimal_test_format_general(L"0.00001235", 7, L"0.00001235");
    decimal_test_format_general(L"0.00001235", 8, L"0.00001235");
    decimal_test_format_general(L"0.00001235", 9, L"0.00001235");
    decimal_test_format_general(L"0.00001235", 10, L"0.00001235");
    decimal_test_format_general(L"0.00001235", 11, L"0.00001235");
    decimal_test_format_general(L"0.00001235", 12, L"0.00001235");
    decimal_test_format_general(L"0.00001235", 13, L"0.00001235");
    decimal_test_format_general(L"0.00001235", 14, L"0.00001235");
    decimal_test_format_general(L"0.00001235", 15, L"0.00001235");
    decimal_test_format_general(L"0.00001235", 16, L"0.00001235");
    decimal_test_format_general(L"0.00000123", 0, L"0.00000123");
    decimal_test_format_general(L"0.00000123", 1, L"0.000001");
    decimal_test_format_general(L"0.00000123", 2, L"0.0000012");
    decimal_test_format_general(L"0.00000123", 3, L"0.00000123");
    decimal_test_format_general(L"0.00000123", 4, L"0.00000123");
    decimal_test_format_general(L"0.00000123", 5, L"0.00000123");
    decimal_test_format_general(L"0.00000123", 6, L"0.00000123");
    decimal_test_format_general(L"0.00000123", 7, L"0.00000123");
    decimal_test_format_general(L"0.00000123", 8, L"0.00000123");
    decimal_test_format_general(L"0.00000123", 9, L"0.00000123");
    decimal_test_format_general(L"0.00000123", 10, L"0.00000123");
    decimal_test_format_general(L"0.00000123", 11, L"0.00000123");
    decimal_test_format_general(L"0.00000123", 12, L"0.00000123");
    decimal_test_format_general(L"0.00000123", 13, L"0.00000123");
    decimal_test_format_general(L"0.00000123", 14, L"0.00000123");
    decimal_test_format_general(L"0.00000123", 15, L"0.00000123");
    decimal_test_format_general(L"0.00000123", 16, L"0.00000123");
    decimal_test_format_general(L"0.00000012", 0, L"1.2E-07");
    decimal_test_format_general(L"0.00000012", 1, L"1E-07");
    decimal_test_format_general(L"0.00000012", 2, L"1.2E-07");
    decimal_test_format_general(L"0.00000012", 3, L"1.2E-07");
    decimal_test_format_general(L"0.00000012", 4, L"1.2E-07");
    decimal_test_format_general(L"0.00000012", 5, L"1.2E-07");
    decimal_test_format_general(L"0.00000012", 6, L"1.2E-07");
    decimal_test_format_general(L"0.00000012", 7, L"1.2E-07");
    decimal_test_format_general(L"0.00000012", 8, L"1.2E-07");
    decimal_test_format_general(L"0.00000012", 9, L"1.2E-07");
    decimal_test_format_general(L"0.00000012", 10, L"1.2E-07");
    decimal_test_format_general(L"0.00000012", 11, L"1.2E-07");
    decimal_test_format_general(L"0.00000012", 12, L"1.2E-07");
    decimal_test_format_general(L"0.00000012", 13, L"1.2E-07");
    decimal_test_format_general(L"0.00000012", 14, L"1.2E-07");
    decimal_test_format_general(L"0.00000012", 15, L"1.2E-07");
    decimal_test_format_general(L"0.00000012", 16, L"1.2E-07");
    decimal_test_format_general(L"0.00000001", 0, L"1E-08");
    decimal_test_format_general(L"0.00000001", 1, L"1E-08");
    decimal_test_format_general(L"0.00000001", 2, L"1E-08");
    decimal_test_format_general(L"0.00000001", 3, L"1E-08");
    decimal_test_format_general(L"0.00000001", 4, L"1E-08");
    decimal_test_format_general(L"0.00000001", 5, L"1E-08");
    decimal_test_format_general(L"0.00000001", 6, L"1E-08");
    decimal_test_format_general(L"0.00000001", 7, L"1E-08");
    decimal_test_format_general(L"0.00000001", 8, L"1E-08");
    decimal_test_format_general(L"0.00000001", 9, L"1E-08");
    decimal_test_format_general(L"0.00000001", 10, L"1E-08");
    decimal_test_format_general(L"0.00000001", 11, L"1E-08");
    decimal_test_format_general(L"0.00000001", 12, L"1E-08");
    decimal_test_format_general(L"0.00000001", 13, L"1E-08");
    decimal_test_format_general(L"0.00000001", 14, L"1E-08");
    decimal_test_format_general(L"0.00000001", 15, L"1E-08");
    decimal_test_format_general(L"0.00000001", 16, L"1E-08");

    decimal_test_format_general(L"0.00000019", 1, L"2E-07");
    decimal_test_format_general(L"25500", 2, L"2.6E+04");
    decimal_test_format_general(L"25500", 3, L"2.55E+04");
    decimal_test_format_general(L"25500", 4, L"2.55E+04");
    decimal_test_format_general(L"25500", 5, L"25500");

    decimal_test_format_general(L"0.00000567", 1, L"0.000006");
    decimal_test_format_general(L"0.00000567", 2, L"0.0000057");
    decimal_test_format_general(L"0.00000567", 3, L"0.00000567");

    decimal_test_format_general(L"0.0000005", 0, L"5E-07");

    // negative
    decimal_test_format_general(L"-12345678.91234567", 0, L"-12345678.91234567");
    decimal_test_format_general(L"-12345678.91234567", 1, L"-1E+07");
    decimal_test_format_general(L"-12345678.91234567", 2, L"-1.2E+07");
    decimal_test_format_general(L"-12345678.91234567", 3, L"-1.23E+07");
    decimal_test_format_general(L"-12345678.91234567", 4, L"-1.235E+07");
    decimal_test_format_general(L"-12345678.91234567", 5, L"-1.2346E+07");
    decimal_test_format_general(L"-12345678.91234567", 6, L"-1.23457E+07");
    decimal_test_format_general(L"-12345678.91234567", 7, L"-1.234568E+07");
    decimal_test_format_general(L"-12345678.91234567", 8, L"-12345679");
    decimal_test_format_general(L"-12345678.91234567", 9, L"-12345678.9");
    decimal_test_format_general(L"-12345678.91234567", 10, L"-12345678.91");
    decimal_test_format_general(L"-12345678.91234567", 11, L"-12345678.912");
    decimal_test_format_general(L"-12345678.91234567", 12, L"-12345678.9123");
    decimal_test_format_general(L"-12345678.91234567", 13, L"-12345678.91235");
    decimal_test_format_general(L"-12345678.91234567", 14, L"-12345678.912346");
    decimal_test_format_general(L"-12345678.91234567", 15, L"-12345678.9123457");
    decimal_test_format_general(L"-12345678.91234567", 16, L"-12345678.91234567");
    decimal_test_format_general(L"-1234567.89123457", 0, L"-1234567.89123457");
    decimal_test_format_general(L"-1234567.89123457", 1, L"-1E+06");
    decimal_test_format_general(L"-1234567.89123457", 2, L"-1.2E+06");
    decimal_test_format_general(L"-1234567.89123457", 3, L"-1.23E+06");
    decimal_test_format_general(L"-1234567.89123457", 4, L"-1.235E+06");
    decimal_test_format_general(L"-1234567.89123457", 5, L"-1.2346E+06");
    decimal_test_format_general(L"-1234567.89123457", 6, L"-1.23457E+06");
    decimal_test_format_general(L"-1234567.89123457", 7, L"-1234568");
    decimal_test_format_general(L"-1234567.89123457", 8, L"-1234567.9");
    decimal_test_format_general(L"-1234567.89123457", 9, L"-1234567.89");
    decimal_test_format_general(L"-1234567.89123457", 10, L"-1234567.891");
    decimal_test_format_general(L"-1234567.89123457", 11, L"-1234567.8912");
    decimal_test_format_general(L"-1234567.89123457", 12, L"-1234567.89123");
    decimal_test_format_general(L"-1234567.89123457", 13, L"-1234567.891235");
    decimal_test_format_general(L"-1234567.89123457", 14, L"-1234567.8912346");
    decimal_test_format_general(L"-1234567.89123457", 15, L"-1234567.89123457");
    decimal_test_format_general(L"-1234567.89123457", 16, L"-1234567.89123457");
    decimal_test_format_general(L"-123456.78912346", 0, L"-123456.78912346");
    decimal_test_format_general(L"-123456.78912346", 1, L"-1E+05");
    decimal_test_format_general(L"-123456.78912346", 2, L"-1.2E+05");
    decimal_test_format_general(L"-123456.78912346", 3, L"-1.23E+05");
    decimal_test_format_general(L"-123456.78912346", 4, L"-1.235E+05");
    decimal_test_format_general(L"-123456.78912346", 5, L"-1.2346E+05");
    decimal_test_format_general(L"-123456.78912346", 6, L"-123457");
    decimal_test_format_general(L"-123456.78912346", 7, L"-123456.8");
    decimal_test_format_general(L"-123456.78912346", 8, L"-123456.79");
    decimal_test_format_general(L"-123456.78912346", 9, L"-123456.789");
    decimal_test_format_general(L"-123456.78912346", 10, L"-123456.7891");
    decimal_test_format_general(L"-123456.78912346", 11, L"-123456.78912");
    decimal_test_format_general(L"-123456.78912346", 12, L"-123456.789123");
    decimal_test_format_general(L"-123456.78912346", 13, L"-123456.7891235");
    decimal_test_format_general(L"-123456.78912346", 14, L"-123456.78912346");
    decimal_test_format_general(L"-123456.78912346", 15, L"-123456.78912346");
    decimal_test_format_general(L"-123456.78912346", 16, L"-123456.78912346");
    decimal_test_format_general(L"-12345.67891235", 0, L"-12345.67891235");
    decimal_test_format_general(L"-12345.67891235", 1, L"-1E+04");
    decimal_test_format_general(L"-12345.67891235", 2, L"-1.2E+04");
    decimal_test_format_general(L"-12345.67891235", 3, L"-1.23E+04");
    decimal_test_format_general(L"-12345.67891235", 4, L"-1.235E+04");
    decimal_test_format_general(L"-12345.67891235", 5, L"-12346");
    decimal_test_format_general(L"-12345.67891235", 6, L"-12345.7");
    decimal_test_format_general(L"-12345.67891235", 7, L"-12345.68");
    decimal_test_format_general(L"-12345.67891235", 8, L"-12345.679");
    decimal_test_format_general(L"-12345.67891235", 9, L"-12345.6789");
    decimal_test_format_general(L"-12345.67891235", 10, L"-12345.67891");
    decimal_test_format_general(L"-12345.67891235", 11, L"-12345.678912");
    decimal_test_format_general(L"-12345.67891235", 12, L"-12345.6789124");
    decimal_test_format_general(L"-12345.67891235", 13, L"-12345.67891235");
    decimal_test_format_general(L"-12345.67891235", 14, L"-12345.67891235");
    decimal_test_format_general(L"-12345.67891235", 15, L"-12345.67891235");
    decimal_test_format_general(L"-12345.67891235", 16, L"-12345.67891235");
    decimal_test_format_general(L"-1234.56789123", 0, L"-1234.56789123");
    decimal_test_format_general(L"-1234.56789123", 1, L"-1E+03");
    decimal_test_format_general(L"-1234.56789123", 2, L"-1.2E+03");
    decimal_test_format_general(L"-1234.56789123", 3, L"-1.23E+03");
    decimal_test_format_general(L"-1234.56789123", 4, L"-1235");
    decimal_test_format_general(L"-1234.56789123", 5, L"-1234.6");
    decimal_test_format_general(L"-1234.56789123", 6, L"-1234.57");
    decimal_test_format_general(L"-1234.56789123", 7, L"-1234.568");
    decimal_test_format_general(L"-1234.56789123", 8, L"-1234.5679");
    decimal_test_format_general(L"-1234.56789123", 9, L"-1234.56789");
    decimal_test_format_general(L"-1234.56789123", 10, L"-1234.567891");
    decimal_test_format_general(L"-1234.56789123", 11, L"-1234.5678912");
    decimal_test_format_general(L"-1234.56789123", 12, L"-1234.56789123");
    decimal_test_format_general(L"-1234.56789123", 13, L"-1234.56789123");
    decimal_test_format_general(L"-1234.56789123", 14, L"-1234.56789123");
    decimal_test_format_general(L"-1234.56789123", 15, L"-1234.56789123");
    decimal_test_format_general(L"-1234.56789123", 16, L"-1234.56789123");
    decimal_test_format_general(L"-123.45678912", 0, L"-123.45678912");
    decimal_test_format_general(L"-123.45678912", 1, L"-1E+02");
    decimal_test_format_general(L"-123.45678912", 2, L"-1.2E+02");
    decimal_test_format_general(L"-123.45678912", 3, L"-123");
    decimal_test_format_general(L"-123.45678912", 4, L"-123.5");
    decimal_test_format_general(L"-123.45678912", 5, L"-123.46");
    decimal_test_format_general(L"-123.45678912", 6, L"-123.457");
    decimal_test_format_general(L"-123.45678912", 7, L"-123.4568");
    decimal_test_format_general(L"-123.45678912", 8, L"-123.45679");
    decimal_test_format_general(L"-123.45678912", 9, L"-123.456789");
    decimal_test_format_general(L"-123.45678912", 10, L"-123.4567891");
    decimal_test_format_general(L"-123.45678912", 11, L"-123.45678912");
    decimal_test_format_general(L"-123.45678912", 12, L"-123.45678912");
    decimal_test_format_general(L"-123.45678912", 13, L"-123.45678912");
    decimal_test_format_general(L"-123.45678912", 14, L"-123.45678912");
    decimal_test_format_general(L"-123.45678912", 15, L"-123.45678912");
    decimal_test_format_general(L"-123.45678912", 16, L"-123.45678912");
    decimal_test_format_general(L"-12.34567891", 0, L"-12.34567891");
    decimal_test_format_general(L"-12.34567891", 1, L"-1E+01");
    decimal_test_format_general(L"-12.34567891", 2, L"-12");
    decimal_test_format_general(L"-12.34567891", 3, L"-12.3");
    decimal_test_format_general(L"-12.34567891", 4, L"-12.35");
    decimal_test_format_general(L"-12.34567891", 5, L"-12.346");
    decimal_test_format_general(L"-12.34567891", 6, L"-12.3457");
    decimal_test_format_general(L"-12.34567891", 7, L"-12.34568");
    decimal_test_format_general(L"-12.34567891", 8, L"-12.345679");
    decimal_test_format_general(L"-12.34567891", 9, L"-12.3456789");
    decimal_test_format_general(L"-12.34567891", 10, L"-12.34567891");
    decimal_test_format_general(L"-12.34567891", 11, L"-12.34567891");
    decimal_test_format_general(L"-12.34567891", 12, L"-12.34567891");
    decimal_test_format_general(L"-12.34567891", 13, L"-12.34567891");
    decimal_test_format_general(L"-12.34567891", 14, L"-12.34567891");
    decimal_test_format_general(L"-12.34567891", 15, L"-12.34567891");
    decimal_test_format_general(L"-12.34567891", 16, L"-12.34567891");
    decimal_test_format_general(L"-1.23456789", 0, L"-1.23456789");
    decimal_test_format_general(L"-1.23456789", 1, L"-1");
    decimal_test_format_general(L"-1.23456789", 2, L"-1.2");
    decimal_test_format_general(L"-1.23456789", 3, L"-1.23");
    decimal_test_format_general(L"-1.23456789", 4, L"-1.235");
    decimal_test_format_general(L"-1.23456789", 5, L"-1.2346");
    decimal_test_format_general(L"-1.23456789", 6, L"-1.23457");
    decimal_test_format_general(L"-1.23456789", 7, L"-1.234568");
    decimal_test_format_general(L"-1.23456789", 8, L"-1.2345679");
    decimal_test_format_general(L"-1.23456789", 9, L"-1.23456789");
    decimal_test_format_general(L"-1.23456789", 10, L"-1.23456789");
    decimal_test_format_general(L"-1.23456789", 11, L"-1.23456789");
    decimal_test_format_general(L"-1.23456789", 12, L"-1.23456789");
    decimal_test_format_general(L"-1.23456789", 13, L"-1.23456789");
    decimal_test_format_general(L"-1.23456789", 14, L"-1.23456789");
    decimal_test_format_general(L"-1.23456789", 15, L"-1.23456789");
    decimal_test_format_general(L"-1.23456789", 16, L"-1.23456789");
    decimal_test_format_general(L"-0.12345679", 0, L"-0.12345679");
    decimal_test_format_general(L"-0.12345679", 1, L"-0.1");
    decimal_test_format_general(L"-0.12345679", 2, L"-0.12");
    decimal_test_format_general(L"-0.12345679", 3, L"-0.123");
    decimal_test_format_general(L"-0.12345679", 4, L"-0.1235");
    decimal_test_format_general(L"-0.12345679", 5, L"-0.12346");
    decimal_test_format_general(L"-0.12345679", 6, L"-0.123457");
    decimal_test_format_general(L"-0.12345679", 7, L"-0.1234568");
    decimal_test_format_general(L"-0.12345679", 8, L"-0.12345679");
    decimal_test_format_general(L"-0.12345679", 9, L"-0.12345679");
    decimal_test_format_general(L"-0.12345679", 10, L"-0.12345679");
    decimal_test_format_general(L"-0.12345679", 11, L"-0.12345679");
    decimal_test_format_general(L"-0.12345679", 12, L"-0.12345679");
    decimal_test_format_general(L"-0.12345679", 13, L"-0.12345679");
    decimal_test_format_general(L"-0.12345679", 14, L"-0.12345679");
    decimal_test_format_general(L"-0.12345679", 15, L"-0.12345679");
    decimal_test_format_general(L"-0.12345679", 16, L"-0.12345679");
    decimal_test_format_general(L"-0.01234568", 0, L"-0.01234568");
    decimal_test_format_general(L"-0.01234568", 1, L"-0.01");
    decimal_test_format_general(L"-0.01234568", 2, L"-0.012");
    decimal_test_format_general(L"-0.01234568", 3, L"-0.0123");
    decimal_test_format_general(L"-0.01234568", 4, L"-0.01235");
    decimal_test_format_general(L"-0.01234568", 5, L"-0.012346");
    decimal_test_format_general(L"-0.01234568", 6, L"-0.0123457");
    decimal_test_format_general(L"-0.01234568", 7, L"-0.01234568");
    decimal_test_format_general(L"-0.01234568", 8, L"-0.01234568");
    decimal_test_format_general(L"-0.01234568", 9, L"-0.01234568");
    decimal_test_format_general(L"-0.01234568", 10, L"-0.01234568");
    decimal_test_format_general(L"-0.01234568", 11, L"-0.01234568");
    decimal_test_format_general(L"-0.01234568", 12, L"-0.01234568");
    decimal_test_format_general(L"-0.01234568", 13, L"-0.01234568");
    decimal_test_format_general(L"-0.01234568", 14, L"-0.01234568");
    decimal_test_format_general(L"-0.01234568", 15, L"-0.01234568");
    decimal_test_format_general(L"-0.01234568", 16, L"-0.01234568");
    decimal_test_format_general(L"-0.00123457", 0, L"-0.00123457");
    decimal_test_format_general(L"-0.00123457", 1, L"-0.001");
    decimal_test_format_general(L"-0.00123457", 2, L"-0.0012");
    decimal_test_format_general(L"-0.00123457", 3, L"-0.00123");
    decimal_test_format_general(L"-0.00123457", 4, L"-0.001235");
    decimal_test_format_general(L"-0.00123457", 5, L"-0.0012346");
    decimal_test_format_general(L"-0.00123457", 6, L"-0.00123457");
    decimal_test_format_general(L"-0.00123457", 7, L"-0.00123457");
    decimal_test_format_general(L"-0.00123457", 8, L"-0.00123457");
    decimal_test_format_general(L"-0.00123457", 9, L"-0.00123457");
    decimal_test_format_general(L"-0.00123457", 10, L"-0.00123457");
    decimal_test_format_general(L"-0.00123457", 11, L"-0.00123457");
    decimal_test_format_general(L"-0.00123457", 12, L"-0.00123457");
    decimal_test_format_general(L"-0.00123457", 13, L"-0.00123457");
    decimal_test_format_general(L"-0.00123457", 14, L"-0.00123457");
    decimal_test_format_general(L"-0.00123457", 15, L"-0.00123457");
    decimal_test_format_general(L"-0.00123457", 16, L"-0.00123457");
    decimal_test_format_general(L"-0.00012346", 0, L"-0.00012346");
    decimal_test_format_general(L"-0.00012346", 1, L"-0.0001");
    decimal_test_format_general(L"-0.00012346", 2, L"-0.00012");
    decimal_test_format_general(L"-0.00012346", 3, L"-0.000123");
    decimal_test_format_general(L"-0.00012346", 4, L"-0.0001235");
    decimal_test_format_general(L"-0.00012346", 5, L"-0.00012346");
    decimal_test_format_general(L"-0.00012346", 6, L"-0.00012346");
    decimal_test_format_general(L"-0.00012346", 7, L"-0.00012346");
    decimal_test_format_general(L"-0.00012346", 8, L"-0.00012346");
    decimal_test_format_general(L"-0.00012346", 9, L"-0.00012346");
    decimal_test_format_general(L"-0.00012346", 10, L"-0.00012346");
    decimal_test_format_general(L"-0.00012346", 11, L"-0.00012346");
    decimal_test_format_general(L"-0.00012346", 12, L"-0.00012346");
    decimal_test_format_general(L"-0.00012346", 13, L"-0.00012346");
    decimal_test_format_general(L"-0.00012346", 14, L"-0.00012346");
    decimal_test_format_general(L"-0.00012346", 15, L"-0.00012346");
    decimal_test_format_general(L"-0.00012346", 16, L"-0.00012346");
    decimal_test_format_general(L"-0.00001235", 0, L"-0.00001235");
    decimal_test_format_general(L"-0.00001235", 1, L"-0.00001");
    decimal_test_format_general(L"-0.00001235", 2, L"-0.000012");
    decimal_test_format_general(L"-0.00001235", 3, L"-0.0000124");
    decimal_test_format_general(L"-0.00001235", 4, L"-0.00001235");
    decimal_test_format_general(L"-0.00001235", 5, L"-0.00001235");
    decimal_test_format_general(L"-0.00001235", 6, L"-0.00001235");
    decimal_test_format_general(L"-0.00001235", 7, L"-0.00001235");
    decimal_test_format_general(L"-0.00001235", 8, L"-0.00001235");
    decimal_test_format_general(L"-0.00001235", 9, L"-0.00001235");
    decimal_test_format_general(L"-0.00001235", 10, L"-0.00001235");
    decimal_test_format_general(L"-0.00001235", 11, L"-0.00001235");
    decimal_test_format_general(L"-0.00001235", 12, L"-0.00001235");
    decimal_test_format_general(L"-0.00001235", 13, L"-0.00001235");
    decimal_test_format_general(L"-0.00001235", 14, L"-0.00001235");
    decimal_test_format_general(L"-0.00001235", 15, L"-0.00001235");
    decimal_test_format_general(L"-0.00001235", 16, L"-0.00001235");
    decimal_test_format_general(L"-0.00000123", 0, L"-0.00000123");
    decimal_test_format_general(L"-0.00000123", 1, L"-0.000001");
    decimal_test_format_general(L"-0.00000123", 2, L"-0.0000012");
    decimal_test_format_general(L"-0.00000123", 3, L"-0.00000123");
    decimal_test_format_general(L"-0.00000123", 4, L"-0.00000123");
    decimal_test_format_general(L"-0.00000123", 5, L"-0.00000123");
    decimal_test_format_general(L"-0.00000123", 6, L"-0.00000123");
    decimal_test_format_general(L"-0.00000123", 7, L"-0.00000123");
    decimal_test_format_general(L"-0.00000123", 8, L"-0.00000123");
    decimal_test_format_general(L"-0.00000123", 9, L"-0.00000123");
    decimal_test_format_general(L"-0.00000123", 10, L"-0.00000123");
    decimal_test_format_general(L"-0.00000123", 11, L"-0.00000123");
    decimal_test_format_general(L"-0.00000123", 12, L"-0.00000123");
    decimal_test_format_general(L"-0.00000123", 13, L"-0.00000123");
    decimal_test_format_general(L"-0.00000123", 14, L"-0.00000123");
    decimal_test_format_general(L"-0.00000123", 15, L"-0.00000123");
    decimal_test_format_general(L"-0.00000123", 16, L"-0.00000123");
    decimal_test_format_general(L"-0.00000012", 0, L"-1.2E-07");
    decimal_test_format_general(L"-0.00000012", 1, L"-1E-07");
    decimal_test_format_general(L"-0.00000012", 2, L"-1.2E-07");
    decimal_test_format_general(L"-0.00000012", 3, L"-1.2E-07");
    decimal_test_format_general(L"-0.00000012", 4, L"-1.2E-07");
    decimal_test_format_general(L"-0.00000012", 5, L"-1.2E-07");
    decimal_test_format_general(L"-0.00000012", 6, L"-1.2E-07");
    decimal_test_format_general(L"-0.00000012", 7, L"-1.2E-07");
    decimal_test_format_general(L"-0.00000012", 8, L"-1.2E-07");
    decimal_test_format_general(L"-0.00000012", 9, L"-1.2E-07");
    decimal_test_format_general(L"-0.00000012", 10, L"-1.2E-07");
    decimal_test_format_general(L"-0.00000012", 11, L"-1.2E-07");
    decimal_test_format_general(L"-0.00000012", 12, L"-1.2E-07");
    decimal_test_format_general(L"-0.00000012", 13, L"-1.2E-07");
    decimal_test_format_general(L"-0.00000012", 14, L"-1.2E-07");
    decimal_test_format_general(L"-0.00000012", 15, L"-1.2E-07");
    decimal_test_format_general(L"-0.00000012", 16, L"-1.2E-07");
    decimal_test_format_general(L"-0.00000001", 0, L"-1E-08");
    decimal_test_format_general(L"-0.00000001", 1, L"-1E-08");
    decimal_test_format_general(L"-0.00000001", 2, L"-1E-08");
    decimal_test_format_general(L"-0.00000001", 3, L"-1E-08");
    decimal_test_format_general(L"-0.00000001", 4, L"-1E-08");
    decimal_test_format_general(L"-0.00000001", 5, L"-1E-08");
    decimal_test_format_general(L"-0.00000001", 6, L"-1E-08");
    decimal_test_format_general(L"-0.00000001", 7, L"-1E-08");
    decimal_test_format_general(L"-0.00000001", 8, L"-1E-08");
    decimal_test_format_general(L"-0.00000001", 9, L"-1E-08");
    decimal_test_format_general(L"-0.00000001", 10, L"-1E-08");
    decimal_test_format_general(L"-0.00000001", 11, L"-1E-08");
    decimal_test_format_general(L"-0.00000001", 12, L"-1E-08");
    decimal_test_format_general(L"-0.00000001", 13, L"-1E-08");
    decimal_test_format_general(L"-0.00000001", 14, L"-1E-08");
    decimal_test_format_general(L"-0.00000001", 15, L"-1E-08");
    decimal_test_format_general(L"-0.00000001", 16, L"-1E-08");

    decimal_test_format_general(L"-0.00000019", 1, L"-2E-07");
    decimal_test_format_general(L"-25500", 2, L"-2.6E+04");
    decimal_test_format_general(L"-25500", 3, L"-2.55E+04");
    decimal_test_format_general(L"-25500", 4, L"-2.55E+04");
    decimal_test_format_general(L"-25500", 5, L"-25500");

    decimal_test_format_general(L"-0.00000567", 1, L"-0.000006");
    decimal_test_format_general(L"-0.00000567", 2, L"-0.0000057");
    decimal_test_format_general(L"-0.00000567", 3, L"-0.00000567");
    
    decimal_test_format_general(L"-0.0000001", 0, L"-1E-07");
    decimal_test_format_general(L"-0.0000001", 1, L"-1E-07");
    decimal_test_format_general(L"-0.0000001", 2, L"-1E-07");
    decimal_test_format_general(L"-0.0000001", 3, L"-1E-07");
    decimal_test_format_general(L"-0.0000001", 4, L"-1E-07");
    decimal_test_format_general(L"-0.0000001", 5, L"-1E-07");
    decimal_test_format_general(L"-0.0000001", 6, L"-1E-07");
    decimal_test_format_general(L"-0.0000001", 7, L"-1E-07");
    decimal_test_format_general(L"-0.0000001", 8, L"-1E-07");
    decimal_test_format_general(L"-0.0000001", 9, L"-1E-07");
    decimal_test_format_general(L"-0.0000001", 10, L"-1E-07");
    decimal_test_format_general(L"-0.0000001", 11, L"-1E-07");
    decimal_test_format_general(L"-0.0000001", 12, L"-1E-07");
    decimal_test_format_general(L"-0.0000001", 13, L"-1E-07");
    decimal_test_format_general(L"-0.0000001", 14, L"-1E-07");
    decimal_test_format_general(L"-0.0000001", 15, L"-1E-07");
    decimal_test_format_general(L"-0.0000001", 16, L"-1E-07");

}

void decimal_format_decimal_tests()
{
    decimal_test_format_decimal(L"0", 0, L"0");

    decimal_test_format_decimal(L"12345678.91234567", 0, L"12345679");
    decimal_test_format_decimal(L"12345678.91234567", 1, L"12345678.9");
    decimal_test_format_decimal(L"12345678.91234567", 2, L"12345678.91");
    decimal_test_format_decimal(L"12345678.91234567", 3, L"12345678.912");
    decimal_test_format_decimal(L"12345678.91234567", 4, L"12345678.9123");
    decimal_test_format_decimal(L"12345678.91234567", 5, L"12345678.91235");
    decimal_test_format_decimal(L"12345678.91234567", 6, L"12345678.912346");
    decimal_test_format_decimal(L"12345678.91234567", 7, L"12345678.9123457");
    decimal_test_format_decimal(L"12345678.91234567", 8, L"12345678.91234567");
    decimal_test_format_decimal(L"12345678.91234567", 9, L"12345678.91234567");

    decimal_test_format_decimal(L"12345600", 0, L"12345600");
    decimal_test_format_decimal(L"12345600", 1, L"12345600");
    decimal_test_format_decimal(L"12345600", 2, L"12345600");
    decimal_test_format_decimal(L"12345600", 3, L"12345600");
    decimal_test_format_decimal(L"12345600", 4, L"12345600");
    decimal_test_format_decimal(L"12345600", 5, L"12345600");
    decimal_test_format_decimal(L"12345600", 6, L"12345600");
    decimal_test_format_decimal(L"12345600", 7, L"12345600");
    decimal_test_format_decimal(L"12345600", 8, L"12345600");
    decimal_test_format_decimal(L"12345600", 9, L"12345600");

    decimal_test_format_decimal(L"0.00912345", 0, L"0");
    decimal_test_format_decimal(L"0.00912345", 1, L"0");
    decimal_test_format_decimal(L"0.00912345", 2, L"0.01");
    decimal_test_format_decimal(L"0.00912345", 3, L"0.009");
    decimal_test_format_decimal(L"0.00912345", 4, L"0.0091");
    decimal_test_format_decimal(L"0.00912345", 5, L"0.00912");
    decimal_test_format_decimal(L"0.00912345", 6, L"0.009123");
    decimal_test_format_decimal(L"0.00912345", 7, L"0.0091235");
    decimal_test_format_decimal(L"0.00912345", 8, L"0.00912345");
    decimal_test_format_decimal(L"0.00912345", 9, L"0.00912345");
    // negative

    decimal_test_format_decimal(L"-12345678.91234567", 0, L"-12345679");
    decimal_test_format_decimal(L"-12345678.91234567", 1, L"-12345678.9");
    decimal_test_format_decimal(L"-12345678.91234567", 2, L"-12345678.91");
    decimal_test_format_decimal(L"-12345678.91234567", 3, L"-12345678.912");
    decimal_test_format_decimal(L"-12345678.91234567", 4, L"-12345678.9123");
    decimal_test_format_decimal(L"-12345678.91234567", 5, L"-12345678.91235");
    decimal_test_format_decimal(L"-12345678.91234567", 6, L"-12345678.912346");
    decimal_test_format_decimal(L"-12345678.91234567", 7, L"-12345678.9123457");
    decimal_test_format_decimal(L"-12345678.91234567", 8, L"-12345678.91234567");
    decimal_test_format_decimal(L"-12345678.91234567", 9, L"-12345678.91234567");

    decimal_test_format_decimal(L"-12345600", 0, L"-12345600");
    decimal_test_format_decimal(L"-12345600", 1, L"-12345600");
    decimal_test_format_decimal(L"-12345600", 2, L"-12345600");
    decimal_test_format_decimal(L"-12345600", 3, L"-12345600");
    decimal_test_format_decimal(L"-12345600", 4, L"-12345600");
    decimal_test_format_decimal(L"-12345600", 5, L"-12345600");
    decimal_test_format_decimal(L"-12345600", 6, L"-12345600");
    decimal_test_format_decimal(L"-12345600", 7, L"-12345600");
    decimal_test_format_decimal(L"-12345600", 8, L"-12345600");
    decimal_test_format_decimal(L"-12345600", 9, L"-12345600");

}

void decimal_format_exponential_tests()
{
    decimal_test_format_exponential(L"0", -1, L"0.000000E+000");
    decimal_test_format_exponential(L"0", 0, L"0E+000");

    decimal_test_format_exponential(L"12345678.91234567", -1, L"1.234568E+007");
    decimal_test_format_exponential(L"12345678.91234567", 0, L"1E+007");
    decimal_test_format_exponential(L"12345678.91234567", 1, L"1.2E+007");
    decimal_test_format_exponential(L"12345678.91234567", 2, L"1.23E+007");
    decimal_test_format_exponential(L"12345678.91234567", 3, L"1.235E+007");
    decimal_test_format_exponential(L"12345678.91234567", 4, L"1.2346E+007");
    decimal_test_format_exponential(L"12345678.91234567", 5, L"1.23457E+007");
    decimal_test_format_exponential(L"12345678.91234567", 6, L"1.234568E+007");
    decimal_test_format_exponential(L"12345678.91234567", 7, L"1.2345679E+007");
    decimal_test_format_exponential(L"12345678.99234567", 7, L"1.2345679E+007");
    decimal_test_format_exponential(L"12345678.91234567", 8, L"1.23456789E+007");
    decimal_test_format_exponential(L"12345678.99234567", 8, L"1.2345679E+007");
    decimal_test_format_exponential(L"12345678.91234567", 9, L"1.234567891E+007");
    decimal_test_format_exponential(L"12345678.91234567", 10, L"1.2345678912E+007");
    decimal_test_format_exponential(L"12345678.91234567", 11, L"1.23456789123E+007");
    decimal_test_format_exponential(L"12345678.91234567", 12, L"1.234567891235E+007");
    decimal_test_format_exponential(L"12345678.91234567", 13, L"1.2345678912346E+007");
    decimal_test_format_exponential(L"12345678.91234567", 14, L"1.23456789123457E+007");
    decimal_test_format_exponential(L"12345678.91234567", 15, L"1.234567891234567E+007");
    decimal_test_format_exponential(L"12345678.91234567", 16, L"1.234567891234567E+007");

    decimal_test_format_exponential(L"12345600", -1, L"1.23456E+007");
    decimal_test_format_exponential(L"12345600", 0, L"1E+007");
    decimal_test_format_exponential(L"12345600", 1, L"1.2E+007");
    decimal_test_format_exponential(L"12345600", 2, L"1.23E+007");
    decimal_test_format_exponential(L"12345600", 3, L"1.235E+007");
    decimal_test_format_exponential(L"12345600", 4, L"1.2346E+007");
    decimal_test_format_exponential(L"12345600", 5, L"1.23456E+007");
    decimal_test_format_exponential(L"12345600", 6, L"1.23456E+007");
    decimal_test_format_exponential(L"12345600", 7, L"1.23456E+007");
    decimal_test_format_exponential(L"12345600", 8, L"1.23456E+007");
    decimal_test_format_exponential(L"12345600", 9, L"1.23456E+007");

    decimal_test_format_exponential(L"0.00123457", -1,  L"1.23457E-003");
    decimal_test_format_exponential(L"0.00123457", 0,  L"1E-003");
    decimal_test_format_exponential(L"0.00123457", 1,  L"1.2E-003");
    decimal_test_format_exponential(L"0.00123457", 2,  L"1.23E-003");
    decimal_test_format_exponential(L"0.00123457", 3,  L"1.235E-003");
    decimal_test_format_exponential(L"0.00123457", 4,  L"1.2346E-003");
    decimal_test_format_exponential(L"0.00123457", 5,  L"1.23457E-003");
    decimal_test_format_exponential(L"0.00123457", 6,  L"1.23457E-003");
    decimal_test_format_exponential(L"0.00123457", 7,  L"1.23457E-003");
    decimal_test_format_exponential(L"0.00123457", 8,  L"1.23457E-003");
    decimal_test_format_exponential(L"0.00123457", 9,  L"1.23457E-003");
    decimal_test_format_exponential(L"0.00123457", 10, L"1.23457E-003");
    decimal_test_format_exponential(L"0.00123457", 11, L"1.23457E-003");
    decimal_test_format_exponential(L"0.00123457", 12, L"1.23457E-003");
    decimal_test_format_exponential(L"0.00123457", 13, L"1.23457E-003");
    decimal_test_format_exponential(L"0.00123457", 14, L"1.23457E-003");
    decimal_test_format_exponential(L"0.00123457", 15, L"1.23457E-003");
    decimal_test_format_exponential(L"0.00123457", 16, L"1.23457E-003");

    // negative
    decimal_test_format_exponential(L"-12345678.91234567", -1, L"-1.234568E+007");
    decimal_test_format_exponential(L"-12345678.91234567", 0, L"-1E+007");
    decimal_test_format_exponential(L"-12345678.91234567", 1, L"-1.2E+007");
    decimal_test_format_exponential(L"-12345678.91234567", 2, L"-1.23E+007");
    decimal_test_format_exponential(L"-12345678.91234567", 3, L"-1.235E+007");
    decimal_test_format_exponential(L"-12345678.91234567", 4, L"-1.2346E+007");
    decimal_test_format_exponential(L"-12345678.91234567", 5, L"-1.23457E+007");
    decimal_test_format_exponential(L"-12345678.91234567", 6, L"-1.234568E+007");
    decimal_test_format_exponential(L"-12345678.91234567", 7, L"-1.2345679E+007");
    decimal_test_format_exponential(L"-12345678.99234567", 7, L"-1.2345679E+007");
    decimal_test_format_exponential(L"-12345678.91234567", 8, L"-1.23456789E+007");
    decimal_test_format_exponential(L"-12345678.99234567", 8, L"-1.2345679E+007");
    decimal_test_format_exponential(L"-12345678.91234567", 9, L"-1.234567891E+007");
    decimal_test_format_exponential(L"-12345678.91234567", 10, L"-1.2345678912E+007");
    decimal_test_format_exponential(L"-12345678.91234567", 11, L"-1.23456789123E+007");
    decimal_test_format_exponential(L"-12345678.91234567", 12, L"-1.234567891235E+007");
    decimal_test_format_exponential(L"-12345678.91234567", 13, L"-1.2345678912346E+007");
    decimal_test_format_exponential(L"-12345678.91234567", 14, L"-1.23456789123457E+007");
    decimal_test_format_exponential(L"-12345678.91234567", 15, L"-1.234567891234567E+007");
    decimal_test_format_exponential(L"-12345678.91234567", 16, L"-1.234567891234567E+007");

    decimal_test_format_exponential(L"-12345600", -1, L"-1.23456E+007");
    decimal_test_format_exponential(L"-12345600", 0, L"-1E+007");
    decimal_test_format_exponential(L"-12345600", 1, L"-1.2E+007");
    decimal_test_format_exponential(L"-12345600", 2, L"-1.23E+007");
    decimal_test_format_exponential(L"-12345600", 3, L"-1.235E+007");
    decimal_test_format_exponential(L"-12345600", 4, L"-1.2346E+007");
    decimal_test_format_exponential(L"-12345600", 5, L"-1.23456E+007");
    decimal_test_format_exponential(L"-12345600", 6, L"-1.23456E+007");
    decimal_test_format_exponential(L"-12345600", 7, L"-1.23456E+007");
    decimal_test_format_exponential(L"-12345600", 8, L"-1.23456E+007");
    decimal_test_format_exponential(L"-12345600", 9, L"-1.23456E+007");

    decimal_test_format_exponential(L"-0.00123457", -1, L"-1.23457E-003");
    decimal_test_format_exponential(L"-0.00123457", 0, L"-1E-003");
    decimal_test_format_exponential(L"-0.00123457", 1, L"-1.2E-003");
    decimal_test_format_exponential(L"-0.00123457", 2, L"-1.23E-003");
    decimal_test_format_exponential(L"-0.00123457", 3, L"-1.235E-003");
    decimal_test_format_exponential(L"-0.00123457", 4, L"-1.2346E-003");
    decimal_test_format_exponential(L"-0.00123457", 5, L"-1.23457E-003");
    decimal_test_format_exponential(L"-0.00123457", 6, L"-1.23457E-003");
    decimal_test_format_exponential(L"-0.00123457", 7, L"-1.23457E-003");
    decimal_test_format_exponential(L"-0.00123457", 8, L"-1.23457E-003");
    decimal_test_format_exponential(L"-0.00123457", 9, L"-1.23457E-003");
    decimal_test_format_exponential(L"-0.00123457", 10, L"-1.23457E-003");
    decimal_test_format_exponential(L"-0.00123457", 11, L"-1.23457E-003");
    decimal_test_format_exponential(L"-0.00123457", 12, L"-1.23457E-003");
    decimal_test_format_exponential(L"-0.00123457", 13, L"-1.23457E-003");
    decimal_test_format_exponential(L"-0.00123457", 14, L"-1.23457E-003");
    decimal_test_format_exponential(L"-0.00123457", 15, L"-1.23457E-003");
    decimal_test_format_exponential(L"-0.00123457", 16, L"-1.23457E-003");

    decimal_test_format_exponential(L"-0.005", 0, L"-5E-003");
    decimal_test_format_exponential(L"-0.005", 1, L"-5E-003");
    decimal_test_format_exponential(L"-0.005", 2, L"-5E-003");
    decimal_test_format_exponential(L"-0.005", 3, L"-5E-003");
    decimal_test_format_exponential(L"-0.005", 4, L"-5E-003");
    decimal_test_format_exponential(L"-0.005", 5, L"-5E-003");
    decimal_test_format_exponential(L"-0.005", 6, L"-5E-003");
    decimal_test_format_exponential(L"-0.005", 7, L"-5E-003");
    decimal_test_format_exponential(L"-0.005", 8, L"-5E-003");
    decimal_test_format_exponential(L"-0.005", 9, L"-5E-003");
    decimal_test_format_exponential(L"-0.005", 10, L"-5E-003");
    decimal_test_format_exponential(L"-0.005", 11, L"-5E-003");
    decimal_test_format_exponential(L"-0.005", 12, L"-5E-003");
    decimal_test_format_exponential(L"-0.005", 13, L"-5E-003");
    decimal_test_format_exponential(L"-0.005", 14, L"-5E-003");
    decimal_test_format_exponential(L"-0.005", 15, L"-5E-003");
    decimal_test_format_exponential(L"-0.005", 16, L"-5E-003");
}

void decimal_arithmetic_tests()
{
    decimal a, b, actual;
    decimal expected;

    decimal_test_parse(L"1.2", &a);
    decimal_test_parse(L"-1.2", &b);

    actual = decimal_multiply(a, b);
    decimal_test_parse(L"-1.44", &expected);
    sput_fail_unless(actual == expected, "1.2 * (-1.2) == -1.44");

    decimal_test_parse(L"-1.2", &a);
    decimal_test_parse(L"1.2", &b);

    actual = decimal_multiply(a, b);
    decimal_test_parse(L"-1.44", &expected);
    sput_fail_unless(actual == expected, "1.2 * (-1.2) == -1.44");
    
    decimal_test_parse(L"1.2", &a);
    decimal_test_parse(L"1.2", &b);

    actual = decimal_multiply(a, b);
    decimal_test_parse(L"1.44", &expected);
    sput_fail_unless(actual == expected, "1.2 * (1.2) == 1.44");
    
    decimal_test_parse(L"-1.2", &a);
    decimal_test_parse(L"-1.2", &b);

    actual = decimal_multiply(a, b);
    decimal_test_parse(L"1.44", &expected);
    sput_fail_unless(actual == expected, "-1.2 * (-1.2) == 1.44");
}

void pinta_tests_decimal()
{
    sput_enter_suite("Decimal tests");
    sput_run_test(decimal_from_string_tests);
    sput_run_test(decimal_format_general_tests);
    sput_run_test(decimal_format_decimal_tests);
    sput_run_test(decimal_format_exponential_tests);
    sput_run_test(decimal_arithmetic_tests);
    sput_leave_suite();
}
