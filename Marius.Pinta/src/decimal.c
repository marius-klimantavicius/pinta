#include "pinta.h"

/* DECIMAL Utilities */

static const decimal pinta_decimal_powers_of_10[] = { 1LL, 10LL, 100LL, 1000LL, 10000LL, 100000LL, 1000000LL, 10000000LL, 100000000LL, 1000000000LL, 10000000000LL, 100000000000LL, 1000000000000LL, 10000000000000LL, 100000000000000LL, 1000000000000000LL, 10000000000000000LL, 100000000000000000LL, 1000000000000000000LL };

decimal pinta_soft_multiply32(u32 left, u32 right)
{
    u32 low;
    u32 high;

    low = (left & 0xFFFF) * (right & 0xFFFF);

    u32 t = low >> 16;
    low &= 0xFFFF;
    t += (left >> 16) * (right & 0xFFFF);
    low += (t & 0xFFFF) << 16;
    high = t >> 16;
    t = low >> 16;
    low &= 0xFFFF;
    t += (right >> 16) * (left & 0xFFFF);
    low += (t & 0xFFFF) << 16;
    high += t >> 16;
    high += (left >> 16) * (right >> 16);
    return ((decimal)high << 32) | low;
}

decimal pinta_soft_multiply64(decimal left, decimal right)
{
    u32 left_low, right_low;
    u32 left_high, right_high;

    decimal result;
    i32 high;

    left_low = left & 0xFFFFFFFF;
    right_low = right & 0xFFFFFFFF;

    left_high = (u32)(left >> 32);
    right_high = (u32)(right >> 32);

    result = pinta_soft_multiply32(left_low, right_low);
    high = (i32)(result >> 32);
    high += (i32)(left_high * right_low + left_low * right_high);
    return ((decimal)high << 32) | (result & 0xFFFFFFFF);
}

decimal decimal_abs(decimal value)
{
    if (value < 0)
        return -value;
    return value;
}

decimal decimal_negate(decimal value)
{
    return -value;
}

decimal decimal_add(decimal left, decimal right)
{
    decimal result = left + right;
    return result % PINTA_DECIMAL_LIMIT;
}

decimal decimal_subtract(decimal left, decimal right)
{
    decimal result = left - right;
    return result % PINTA_DECIMAL_LIMIT;
}

decimal decimal_multiply(decimal left, decimal right)
{
    decimal result, rounder;

    u8 left_negative;
    decimal left_value;
    decimal left_integral;
    decimal left_fraction;

    u8 right_negative;
    decimal right_value;
    decimal right_integral;
    decimal right_fraction;

    left_negative = left < 0;
    right_negative = right < 0;

    left_value = left;
    if (left_negative)
        left_value = -left_value;

    right_value = right;
    if (right_negative)
        right_value = -right_value;

    left_integral = left_value / PINTA_DECIMAL_SCALE;
    left_fraction = left_value % PINTA_DECIMAL_SCALE;

    right_integral = right_value / PINTA_DECIMAL_SCALE;
    right_fraction = right_value % PINTA_DECIMAL_SCALE;

    rounder = left_fraction * right_fraction;
    result = left_integral * right_integral;
    result = result * PINTA_DECIMAL_SCALE;
    result = result + left_integral * right_fraction;
    result = result + right_integral * left_fraction;
    result = result + rounder / PINTA_DECIMAL_SCALE;
    result = result % PINTA_DECIMAL_LIMIT;

    if (rounder% PINTA_DECIMAL_SCALE >= PINTA_DECIMAL_ROUNDER)
        result = result + 1;

    if (left_negative ^ right_negative)
        result = -result;

    return result;
}

decimal decimal_divide(decimal left, decimal right)
{
    decimal result, rounder;
    decimal scale;

    u8 left_negative;
    decimal left_value;

    u8 right_negative;
    decimal right_value;

    decimal quotent, remainder;

    left_negative = left < 0;
    right_negative = right > 0;

    left_value = left;
    if (left_negative)
        left_value = -left_value;

    right_value = right;
    if (right_negative)
        right_value = -right_value;

    result = 0;
    scale = PINTA_DECIMAL_SCALE;
    while (scale > 0)
    {
        if (left_value > right_value)
        {
            quotent = left_value / right_value;
            remainder = left_value % right_value;

            result = result + quotent * scale;
            left_value = remainder * 10;
        }
        else
        {
            left_value = left_value * 10;
        }
        scale = scale / 10;
    }

    rounder = left_value / right_value;
    if (rounder >= 5)
        result = result + 1;

    result = result % PINTA_DECIMAL_LIMIT;

    if (left_negative ^ right_negative)
        return -result;
    return result;
}

decimal decimal_from32(i32 value)
{
    decimal result;

    if (value < 0)
    {
        value = -value;
        result = value * PINTA_DECIMAL_SCALE;
        result = result % PINTA_DECIMAL_LIMIT;
        result = -result;
    }
    else
    {
        result = value * PINTA_DECIMAL_SCALE;
        result = result % PINTA_DECIMAL_LIMIT;
    }

    return result;
}

i32 decimal_to32(decimal value)
{
    return (i32)(value / PINTA_DECIMAL_SCALE);
}

decimal decimal_to_integral_value(decimal value)
{
    decimal integral;

    integral = value / PINTA_DECIMAL_SCALE;
    return integral * PINTA_DECIMAL_SCALE;
}

i32 decimal_compare(decimal left, decimal right)
{
    if (left == right)
        return 0;

    if (left < right)
        return -1;

    return 1;
}

u32 decimal_is_zero(decimal value)
{
    if (value == 0)
        return 1;
    return 0;
}

u32 decimal_is_positive(decimal value)
{
    if (value > 0)
        return 1;
    return 0;
}

u32 decimal_is_negative(decimal value)
{
    if (value < 0)
        return 1;
    return 0;
}

u32 decimal_to_string(decimal value, wchar *string)
{
    u32 index;
    decimal integral;
    decimal fraction;
    u8 is_negative;

    u32 string_length;
    u32 integral_digit_count, fraction_digit_count, fractional_padding_length;

    is_negative = value < 0;
    if (is_negative)
        value = -value;

    integral = value / PINTA_DECIMAL_SCALE;
    fraction = value % PINTA_DECIMAL_SCALE;

    integral_digit_count = integer_get_digit_count((u32)integral) + 1;

    if (fraction == 0)
    {
        fraction_digit_count = 0;
        fractional_padding_length = 0;
        string_length = is_negative + integral_digit_count;
    }
    else
    {
        fraction_digit_count = integer_get_digit_count((u32)fraction) + 1;
        fractional_padding_length = PINTA_DECIMAL_PRECISION - fraction_digit_count;

        while ((fraction % 10) == 0)
        {
            fraction = fraction / 10;
            fraction_digit_count--;
        }

        string_length = is_negative + integral_digit_count + fractional_padding_length + fraction_digit_count + 1 /* '.' */;
    }

    if (!string)
        return string_length;

    if (is_negative)
        *string++ = PINTA_CHAR('-');

    string += integer_to_string((i32)integral, string);

    if (fraction != 0)
    {
        *string++ = PINTA_CHAR('.');

        for (index = 0; index < fractional_padding_length; index++)
            *string++ = PINTA_CHAR('0');

        string += integer_to_string((i32)fraction, string);
    }

    return string_length;
}

u8 decimal_from_string(wchar *string, u32 string_length, decimal *value)
{
    u8 is_negative, is_negative_exponent;
    decimal result, next;
    i32 exponent, integral_digits, fraction_zeroes, fraction_digits;
    i32 scale, last_digit;

    wchar current, *string_end;
    wchar *integral_start = NULL, *fraction_start = NULL;

    if (string_length == 0xFFFFFFFF)
        string_length = string_get_length(string);

    string_end = string + string_length;

    while (string < string_end && char_is_whitespace(*string))
        string++;

    is_negative = 0;
    is_negative_exponent = 0;
    exponent = 0;
    next = 0;
    result = 0;
    integral_digits = 0;
    fraction_zeroes = 0;
    fraction_digits = 0;
    last_digit = 0;

    if (*string == PINTA_CHAR('-') || *string == PINTA_CHAR('+'))
    {
        is_negative = *string == PINTA_CHAR('-');
        string++;
    }

    if (*string >= PINTA_CHAR('0') && *string <= PINTA_CHAR('9'))
        integral_start = string;

    while (string < string_end  && *string >= PINTA_CHAR('0') && *string <= PINTA_CHAR('9'))
        string++;

    if (*string == PINTA_CHAR('.'))
    {
        string++;

        if (*string >= PINTA_CHAR('0') && *string <= PINTA_CHAR('9'))
            fraction_start = string;

        while (string < string_end  && *string >= PINTA_CHAR('0') && *string <= PINTA_CHAR('9'))
            string++;
    }

    if (integral_start == NULL && fraction_start == NULL)
        return 0;

    if (*string == PINTA_CHAR('e') || *string == PINTA_CHAR('E'))
    {
        string++;

        if (*string == PINTA_CHAR('-') || *string == PINTA_CHAR('+'))
        {
            is_negative_exponent = *string == PINTA_CHAR('-');

            string++;
        }

        while (string < string_end && *string >= PINTA_CHAR('0') && *string <= PINTA_CHAR('9'))
        {
            current = *string - PINTA_CHAR('0');

            if (exponent > 99)
                return 0;

            exponent = (exponent * 10) + current;

            string++;
        }

        if (is_negative_exponent)
            exponent = -exponent;
    }

    while (string < string_end && char_is_whitespace(*string))
        string++;

    if (string != string_end)
        return 0;

    if (integral_start != NULL)
    {
        string = integral_start;
        while (string < string_end && *string >= PINTA_CHAR('0') && *string <= PINTA_CHAR('9'))
        {
            if (next < PINTA_DECIMAL_LIMIT)
            {
                current = *string - PINTA_CHAR('0');

                next = (result * 10) + current;
                if (next < PINTA_DECIMAL_LIMIT)
                    result = next;
                else
                    last_digit = current;
            }

            if (result != 0)
                integral_digits++;

            string++;
        }
    }

    if (fraction_start != NULL)
    {
        string = fraction_start;
        while (string < string_end && *string >= PINTA_CHAR('0') && *string <= PINTA_CHAR('9'))
        {
            if (next < PINTA_DECIMAL_LIMIT)
            {
                current = *string - PINTA_CHAR('0');

                next = (result * 10) + current;
                if (next < PINTA_DECIMAL_LIMIT)
                {
                    result = next;

                    if (result != 0)
                        fraction_digits++;
                }
                else
                {
                    last_digit = current;
                }
            }

            if (result == 0)
                fraction_zeroes++;

            string++;
        }
    }

    exponent = exponent - fraction_zeroes - fraction_digits;
    if (integral_digits == 0)
        integral_digits = fraction_digits;
    else
        integral_digits = integral_digits + fraction_zeroes + fraction_digits;

    if (integral_digits + exponent > PINTA_DECIMAL_PRECISION)
        return 0;

    scale = PINTA_DECIMAL_PRECISION + exponent;
    if (integral_digits > 2 * PINTA_DECIMAL_PRECISION)
        scale = scale + (integral_digits - 2 * PINTA_DECIMAL_PRECISION);

    if (scale > 0)
    {
        last_digit = 0;
        while (scale > 0)
        {
            result = result * 10;
            scale--;
        }
    }
    else if (scale < 0)
    {
        while (scale < -1 && result > 0)
        {
            result = result / 10;
            scale++;
        }

        last_digit = result % 10;
        result = result / 10;
    }

    if (last_digit >= 5)
        result++;

    if (is_negative)
        result = -result;

    if (value)
        *value = result;

    return 1;
}

void pinta_format_decimal_general(decimal value, i32 precision, wchar *string)
{
    decimal integral, fraction;
    u32 integral_digits, fraction_digits, fraction_padding, digits;
    i32 exp = -PINTA_DECIMAL_PRECISION;
    decimal last_digit;
    decimal scale;

    if (value == 0) // do not want to deal with 0
    {
        *string++ = PINTA_CHAR('0');
        *string = 0;
        return;
    }

    if (value < 0)
    {
        *string++ = PINTA_CHAR('-');
        value = -value;
    }

    integral = value / PINTA_DECIMAL_SCALE;
    fraction = value % PINTA_DECIMAL_SCALE;

    if (precision <= 0)
        precision = 2 * PINTA_DECIMAL_PRECISION;
    else if (precision > 2 * PINTA_DECIMAL_PRECISION)
        precision = 2 * PINTA_DECIMAL_PRECISION;

    integral_digits = 0;
    if (integral > 0)
        integral_digits = integer_get_digit_count((u32)integral) + 1;

    fraction_digits = 0;
    fraction_padding = 0;
    if (fraction > 0)
    {
        fraction_digits = integer_get_digit_count((u32)fraction) + 1;
        fraction_padding = PINTA_DECIMAL_PRECISION - fraction_digits;

        while ((fraction % 10) == 0)
        {
            fraction = fraction / 10;
            fraction_digits--;
        }
    }

    if (integral_digits > 0)
        digits = integral_digits + fraction_padding + fraction_digits;
    else
        digits = fraction_digits;

    if (digits == 0)
        digits = 1;

    if (digits > (u32)precision || (integral_digits == 0 && fraction_padding >= 6))
    {
        if (integral_digits > 0)
            digits = integral_digits + PINTA_DECIMAL_PRECISION;

        last_digit = 0;
        if (digits > (u32)precision)
        {
            scale = pinta_decimal_powers_of_10[digits - (u32)precision - 1];

            value = value / scale;
            last_digit = value % 10;
            value = value / 10;

            if (last_digit >= 5)
                value = value + 1;

            exp = exp + ((i32)digits - precision);
            digits = (u32)precision;
        }

        while ((value % 10) == 0)
        {
            value = value / 10;
            exp++;
            if (exp >= 0)
                digits--;
        }

        if (exp > 0 || (exp + (i32)digits - 1) < -6)
        {
            exp = exp + (i32)digits - 1;

            scale = pinta_decimal_powers_of_10[digits - 1];

            integral = value / scale;
            fraction = value % scale;

            string += long_to_string((i64)integral, string);

            if (fraction != 0)
            {
                *string++ = PINTA_CHAR('.');
                string += long_to_string((i64)fraction, string);
            }

            *string++ = PINTA_CHAR('E');
            if (exp < 0)
            {
                *string++ = PINTA_CHAR('-');
                exp = -exp;
            }
            else
            {
                *string++ = PINTA_CHAR('+');
            }

            if (exp < 10 && exp > -10)
                *string++ = PINTA_CHAR('0');
            string += integer_to_string(exp, string);
        }
        else
        {
            scale = pinta_decimal_powers_of_10[PINTA_DECIMAL_PRECISION + exp];
            value = value * scale;
            decimal_to_string(value, string);
        }
    }
    else
    {
        decimal_to_string(value, string);
    }
}

void pinta_format_decimal_fixed(decimal value, i32 precision, wchar *string)
{
    decimal scale;
    decimal last_digit;

    if (value == 0) // do not want to deal with 0
    {
        *string++ = PINTA_CHAR('0');
        *string = 0;
        return;
    }

    if (value < 0)
    {
        *string++ = PINTA_CHAR('-');
        value = -value;
    }

    if (precision < 0)
        precision = 2;
    else if (precision > PINTA_DECIMAL_PRECISION)
        precision = PINTA_DECIMAL_PRECISION;

    if (precision < PINTA_DECIMAL_PRECISION)
    {
        scale = pinta_decimal_powers_of_10[PINTA_DECIMAL_PRECISION - precision - 1];
        value = value / scale;
        last_digit = value % 10;
        value = value - last_digit;
        if (last_digit >= 5)
            value += 10;
        value = value * scale;
    }

    decimal_to_string(value, string);
}

void pinta_format_decimal_exponential(decimal value, i32 precision, wchar *string)
{
    decimal integral, fraction;
    u32 integral_digits, fraction_digits, fraction_padding, digits;
    i32 exp = -PINTA_DECIMAL_PRECISION;
    decimal last_digit;
    decimal scale;

    if (precision < 0)
        precision = 6;
    else if (precision > (2 * PINTA_DECIMAL_PRECISION - 1))
        precision = 2 * PINTA_DECIMAL_PRECISION - 1;

    precision++;

    if (value == 0) // do not want to deal with 0
    {
        *string++ = PINTA_CHAR('0');

        precision--;
        if (precision > 0)
        {
            *string++ = PINTA_CHAR('.');
            while (precision-- > 0)
                *string++ = PINTA_CHAR('0');
        }

        *string++ = PINTA_CHAR('E');
        *string++ = PINTA_CHAR('+');
        *string++ = PINTA_CHAR('0');
        *string++ = PINTA_CHAR('0');
        *string++ = PINTA_CHAR('0');

        *string = 0;
        return;
    }

    if (value < 0)
    {
        *string++ = PINTA_CHAR('-');
        value = -value;
    }

    while ((value % 10) == 0)
    {
        value = value / 10;
        exp++;
    }

    integral = value / PINTA_DECIMAL_SCALE;
    fraction = value % PINTA_DECIMAL_SCALE;

    integral_digits = 0;
    if (integral > 0)
        integral_digits = integer_get_digit_count((u32)integral) + 1;

    fraction_digits = 0;
    fraction_padding = PINTA_DECIMAL_PRECISION;
    if (fraction > 0)
    {
        fraction_digits = integer_get_digit_count((u32)fraction) + 1;
        fraction_padding = PINTA_DECIMAL_PRECISION - fraction_digits;
    }

    if (integral_digits > 0)
        digits = integral_digits + fraction_padding + fraction_digits;
    else
        digits = fraction_digits;

    if (digits == 0)
        digits = 1;

    last_digit = 0;
    if (digits > (u32)precision)
    {
        scale = pinta_decimal_powers_of_10[digits - (u32)precision - 1];

        value = value / scale;
        last_digit = value % 10;
        value = value / 10;

        if (last_digit >= 5)
            value = value + 1;

        exp = exp + ((i32)digits - precision);
        digits = (u32)precision;
    }

    while ((value % 10) == 0)
    {
        value = value / 10;
        digits--;
        exp++;
    }

    exp = exp + (i32)digits - 1;

    // digits cannot be 0 in this case, but I am adding assert just in case
    pinta_assert(digits > 0);

#if defined(_MSC_VER)
#pragma warning(suppress: 6385)
#endif
    scale = pinta_decimal_powers_of_10[digits - 1];

    integral = value / scale;
    fraction = value % scale;

    string += long_to_string((i64)integral, string);

    if (fraction != 0)
    {
        *string++ = PINTA_CHAR('.');
        string += long_to_string((i64)fraction, string);
    }

    *string++ = PINTA_CHAR('E');
    if (exp < 0)
    {
        *string++ = PINTA_CHAR('-');
        exp = -exp;
    }
    else
    {
        *string++ = PINTA_CHAR('+');
    }

    if (exp < 100 && exp > -100)
        *string++ = PINTA_CHAR('0');

    if (exp < 10 && exp > -10)
        *string++ = PINTA_CHAR('0');

    string += integer_to_string(exp, string);
}

/* DECIMAL ACCESSORS */

decimal pinta_decimal_get_value(PintaHeapObject *object)
{
    pinta_assert(object != NULL);
    pinta_assert(object->block_kind == PINTA_KIND_DECIMAL);

    return object->data.decimal.decimal_value;
}

void pinta_decimal_set_value(PintaHeapObject *object, decimal value)
{
    pinta_assert(object != NULL);
    pinta_assert(object->block_kind == PINTA_KIND_DECIMAL);

    object->data.decimal.decimal_value = value;
}

decimal pinta_decimal_ref_get_value(PintaReference *reference)
{
    pinta_assert(reference != NULL);

    return pinta_decimal_get_value(reference->reference);
}

void pinta_decimal_ref_set_value(PintaReference *reference, decimal value)
{
    pinta_assert(reference != NULL);

    pinta_decimal_set_value(reference->reference, value);
}

/* DECIMAL */

void pinta_decimal_init_type(PintaType *type)
{
    type->is_string = 0;

    type->to_integer = pinta_lib_decimal_to_int32;
    type->to_integer_value = pinta_lib_decimal_to_int32_value;
    type->to_string = pinta_lib_decimal_to_string;
    type->to_decimal = pinta_core_default_copy;
    type->to_numeric = pinta_core_default_copy;
    type->to_bool = pinta_lib_decimal_to_bool;

#if PINTA_DEBUG
    type->debug_write = pinta_lib_decimal_debug_write;
#endif
}

PintaHeapObject *pinta_decimal_alloc_object(PintaCore *core)
{
    PintaHeapObject *result;

    pinta_assert(core != NULL);

    result = pinta_core_alloc(core, PINTA_KIND_DECIMAL, PINTA_FLAG_NONE, 1);

    return result;
}

void pinta_decimal_set_zero(PintaHeapObject *value)
{
    pinta_decimal_set_value(value, 0);
}

PintaException pinta_lib_decimal_alloc(PintaCore *core, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    PintaHeapObject *value;

    pinta_assert(core != NULL);
    pinta_assert(result != NULL);

    value = pinta_decimal_alloc_object(core);
    if (value == NULL)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_MEMORY);

    result->reference = value;

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_decimal_alloc_zero(PintaCore *core, PintaReference *result)
{
    PintaException exception = PINTA_OK;

    pinta_assert(core != NULL);
    pinta_assert(result != NULL);

    PINTA_CHECK(pinta_lib_decimal_alloc(core, result));
    pinta_decimal_set_zero(result->reference);

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_decimal_to_int32(PintaCore *core, PintaReference *value, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    i32 int_value;
    PintaHeapObject *integer;

    pinta_assert(core != NULL);
    pinta_assert(value != NULL);
    pinta_assert(result != NULL);

    PINTA_CHECK(pinta_lib_decimal_to_int32_value(core, value, &int_value));
    integer = pinta_integer_alloc_object_value(core, int_value);
    if (integer == NULL)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_MEMORY);

    result->reference = integer;
PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_decimal_to_int32_value(PintaCore *core, PintaReference *value, i32 *result)
{
    PintaException exception = PINTA_OK;

    pinta_assert(core != NULL);
    pinta_assert(value != NULL);
    pinta_assert(result != NULL);

    if (value->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (value->reference->block_kind != PINTA_KIND_DECIMAL)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    *result = decimal_to32(pinta_decimal_ref_get_value(value));

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_decimal_from_int32(PintaCore *core, PintaReference *value, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    decimal decimal_value;
    PintaHeapObject *decimal;

    pinta_assert(core != NULL);
    pinta_assert(value != NULL);
    pinta_assert(result != NULL);

    if (value->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (value->reference->block_kind != PINTA_KIND_INTEGER)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    decimal = pinta_decimal_alloc_object(core);
    if (decimal == NULL)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_MEMORY);

    decimal_value = decimal_from32(pinta_integer_ref_get_value(value));
    pinta_decimal_set_value(decimal, decimal_value);
    result->reference = decimal;

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_decimal_to_string(PintaCore *core, PintaReference *value, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    u32 string_length;
    PintaHeapObject *string;

    pinta_assert(core != NULL);
    pinta_assert(value != NULL);
    pinta_assert(result != NULL);

    if (value->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (value->reference->block_kind != PINTA_KIND_DECIMAL)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    string = pinta_string_alloc_object(core, PINTA_DECIMAL_DIGITS);

    string_length = decimal_to_string(pinta_decimal_ref_get_value(value), pinta_string_get_data(string));
    pinta_string_set_length(string, string_length);

    result->reference = string;
PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_decimal_parse(PintaCore *core, PintaReference *value, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    u8 isFailed;

    pinta_assert(core != NULL);
    pinta_assert(value != NULL);
    pinta_assert(result != NULL);

    PINTA_CHECK(pinta_lib_decimal_try_parse(core, value, &isFailed, result));

    if (isFailed)
        PINTA_THROW(PINTA_EXCEPTION_BAD_FORMAT);

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_decimal_try_parse(PintaCore *core, PintaReference *value, u8 *isFailed, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    u8 succeeded = 0;
    decimal decimal_value = 0;
    PintaHeapObject *decimal;

    pinta_assert(core != NULL);
    pinta_assert(value != NULL);
    pinta_assert(result != NULL);

    if (isFailed)
        *isFailed = 1;

    if (value->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    // convert sub- and multi- strings to string, perform conversion in place (change reference, this is possible because both substring and multistring are strings for the outside world)
    PINTA_CHECK(pinta_lib_string_to_string(core, value, value));
    decimal = pinta_decimal_alloc_object(core);
    if (decimal == NULL)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_MEMORY);

    succeeded = decimal_from_string(pinta_string_ref_get_data(value), pinta_string_ref_get_length(value), &decimal_value);

    if (!succeeded)
        decimal_value = 0;

    pinta_decimal_set_value(decimal, decimal_value);
    result->reference = decimal;

    if (isFailed)
        *isFailed = !succeeded;

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_decimal_add(PintaCore *core, PintaReference *left, PintaReference *right, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    decimal left_value, right_value, result_value;
    PintaHeapObject *decimal_value;

    pinta_assert(core != NULL);
    pinta_assert(left != NULL);
    pinta_assert(right != NULL);
    pinta_assert(result != NULL);

    if (left->reference == NULL || right->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    decimal_value = pinta_decimal_alloc_object(core);
    if (decimal_value == NULL)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_MEMORY);

    left_value = pinta_decimal_ref_get_value(left);
    right_value = pinta_decimal_ref_get_value(right);
    result_value = decimal_add(left_value, right_value);

    pinta_decimal_set_value(decimal_value, result_value);

    result->reference = decimal_value;
PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_decimal_subtract(PintaCore *core, PintaReference *left, PintaReference *right, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    decimal left_value, right_value, result_value;
    PintaHeapObject *decimal_value;

    pinta_assert(core != NULL);
    pinta_assert(left != NULL);
    pinta_assert(right != NULL);
    pinta_assert(result != NULL);

    if (left->reference == NULL || right->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    decimal_value = pinta_decimal_alloc_object(core);
    if (decimal_value == NULL)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_MEMORY);

    left_value = pinta_decimal_ref_get_value(left);
    right_value = pinta_decimal_ref_get_value(right);
    result_value = decimal_subtract(left_value, right_value);

    pinta_decimal_set_value(decimal_value, result_value);

    result->reference = decimal_value;

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_decimal_multiply(PintaCore *core, PintaReference *left, PintaReference *right, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    decimal left_value, right_value, result_value;
    PintaHeapObject *decimal_value;

    pinta_assert(core != NULL);
    pinta_assert(left != NULL);
    pinta_assert(right != NULL);
    pinta_assert(result != NULL);

    if (left->reference == NULL || right->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    decimal_value = pinta_decimal_alloc_object(core);
    if (decimal_value == NULL)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_MEMORY);

    left_value = pinta_decimal_ref_get_value(left);
    right_value = pinta_decimal_ref_get_value(right);
    result_value = decimal_multiply(left_value, right_value);

    pinta_decimal_set_value(decimal_value, result_value);

    result->reference = decimal_value;

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_decimal_divide(PintaCore *core, PintaReference *left, PintaReference *right, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    decimal left_value, right_value, result_value;
    PintaHeapObject *decimal_value;

    pinta_assert(core != NULL);
    pinta_assert(left != NULL);
    pinta_assert(right != NULL);
    pinta_assert(result != NULL);

    if (left->reference == NULL || right->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    decimal_value = pinta_decimal_alloc_object(core);
    if (decimal_value == NULL)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_MEMORY);

    left_value = pinta_decimal_ref_get_value(left);
    right_value = pinta_decimal_ref_get_value(right);
    if (decimal_is_zero(right_value))
        PINTA_THROW(PINTA_EXCEPTION_DIVISION_BY_ZERO);

    result_value = decimal_divide(left_value, right_value);

    pinta_decimal_set_value(decimal_value, result_value);

    result->reference = decimal_value;

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_decimal_compare(PintaCore *core, PintaReference *left, PintaReference *right, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    decimal left_value, right_value, result_value;
    PintaHeapObject *decimal_value;

    pinta_assert(core != NULL);
    pinta_assert(left != NULL);
    pinta_assert(right != NULL);
    pinta_assert(result != NULL);

    if (left->reference == NULL || right->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    decimal_value = pinta_decimal_alloc_object(core);
    if (decimal_value == NULL)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_MEMORY);

    left_value = pinta_decimal_ref_get_value(left);
    right_value = pinta_decimal_ref_get_value(right);
    result_value = (decimal)decimal_compare(left_value, right_value);

    pinta_decimal_set_value(decimal_value, result_value);

    result->reference = decimal_value;

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_decimal_negate(PintaCore *core, PintaReference *value, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    decimal result_value;
    PintaHeapObject *decimal_value;

    pinta_assert(core != NULL);
    pinta_assert(value != NULL);
    pinta_assert(result != NULL);

    if (value->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    decimal_value = pinta_decimal_alloc_object(core);
    if (decimal_value == NULL)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_MEMORY);

    result_value = pinta_decimal_ref_get_value(value);
    result_value = decimal_negate(result_value);

    pinta_decimal_set_value(decimal_value, result_value);

    result->reference = decimal_value;

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_decimal_is_zero(PintaCore *core, PintaReference *value, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    decimal decimal_value;
    u32 is_zero;

    pinta_assert(core != NULL);
    pinta_assert(value != NULL);
    pinta_assert(result != NULL);

    if (value->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    decimal_value = pinta_decimal_ref_get_value(value);
    is_zero = decimal_is_zero(decimal_value);
    PINTA_CHECK(pinta_lib_integer_alloc_value(core, (i32)is_zero, result));

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_decimal_is_positive(PintaCore *core, PintaReference *value, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    decimal decimal_value;
    u32 is_positive;

    pinta_assert(core != NULL);
    pinta_assert(value != NULL);
    pinta_assert(result != NULL);

    if (value->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    decimal_value = pinta_decimal_ref_get_value(value);
    is_positive = decimal_is_positive(decimal_value);
    PINTA_CHECK(pinta_lib_integer_alloc_value(core, (i32)is_positive, result));

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_decimal_is_negative(PintaCore *core, PintaReference *value, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    decimal decimal_value;
    u32 is_negative;

    pinta_assert(core != NULL);
    pinta_assert(value != NULL);
    pinta_assert(result != NULL);

    if (value->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    decimal_value = pinta_decimal_ref_get_value(value);
    is_negative = decimal_is_negative(decimal_value);
    PINTA_CHECK(pinta_lib_integer_alloc_value(core, (i32)is_negative, result));

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_decimal_to_bool(PintaCore *core, PintaReference *value, u8 *result)
{
    pinta_assert(core != NULL);
    pinta_assert(value != NULL);
    pinta_assert(result != NULL);

    if (decimal_is_zero(pinta_decimal_ref_get_value(value)) == 1)
        *result = 0;
    else
        *result = 1;

    return PINTA_OK;
}

PintaException pinta_lib_decimal_debug_write(PintaCore *core, PintaReference *value, u32 max_depth, PintaJsonWriter *writer)
{
    decimal decimal_value;

    PINTA_UNUSED(max_depth);

    pinta_assert(core != NULL);
    pinta_assert(value != NULL);

    if (value->reference == NULL)
        return pinta_json_write_null(core, writer);

    decimal_value = pinta_decimal_ref_get_value(value);
    return pinta_json_write_decimal(core, writer, decimal_value);
}
