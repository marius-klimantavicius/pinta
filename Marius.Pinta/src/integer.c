#include "pinta.h"

/* INTEGER utilities */

static const u32 pinta_integer_powers_of_10[] = { 0, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000 };
static const u32 pinta_de_bruijn_bit_position[32] = { 0, 9, 1, 10, 13, 21, 2, 29, 11, 14, 16, 18, 22, 25, 3, 30, 8, 12, 20, 28, 15, 17, 24, 7, 19, 27, 23, 6, 26, 5, 4, 31 };

u32 integer_get_log2(u32 value)
{
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;

    return pinta_de_bruijn_bit_position[(u32)(value * 0x07C4ACDDU) >> 27];
}

u32 integer_get_digit_count(u32 value)
{
    u32 temp;

    temp = (integer_get_log2(value) + 1) * 1233 >> 12;
    return temp - (value < pinta_integer_powers_of_10[temp]);
}

u32 integer_to_string(i32 value, wchar *string)
{
    u8 is_negative = 0;
    u32 digit_count;

    if (value < 0)
    {
        is_negative = 1;
        value = -value;
    }

    digit_count = integer_get_digit_count((u32)value) + 1;
    if (!string)
        return digit_count + is_negative;

    if (is_negative)
        *string++ = PINTA_CHAR('-');

    string = string + digit_count;
    *string-- = 0;

    switch (digit_count)
    {
    case 10:
        *string-- = PINTA_CHAR('0') + (value % 10);
        value = value / 10;
        // fall through
    case 9:
        *string-- = PINTA_CHAR('0') + (value % 10);
        value = value / 10;
        // fall through
    case 8:
        *string-- = PINTA_CHAR('0') + (value % 10);
        value = value / 10;
        // fall through
    case 7:
        *string-- = PINTA_CHAR('0') + (value % 10);
        value = value / 10;
        // fall through
    case 6:
        *string-- = PINTA_CHAR('0') + (value % 10);
        value = value / 10;
        // fall through
    case 5:
        *string-- = PINTA_CHAR('0') + (value % 10);
        value = value / 10;
        // fall through
    case 4:
        *string-- = PINTA_CHAR('0') + (value % 10);
        value = value / 10;
        // fall through
    case 3:
        *string-- = PINTA_CHAR('0') + (value % 10);
        value = value / 10;
        // fall through
    case 2:
        *string-- = PINTA_CHAR('0') + (value % 10);
        value = value / 10;
        // fall through
    default:
        *string-- = (wchar)(PINTA_CHAR('0') + value);
        break;
    }

    return digit_count + is_negative;
}

u32 long_to_string(i64 value, wchar *string)
{
    i32 integral, fraction;
    u32 length, result = 0;

    if (value == 0)
    {
        *string++ = PINTA_CHAR('0');
        *string = 0;
        return 1;
    }

    integral = (i32)(value / PINTA_DECIMAL_SCALE);
    fraction = (i32)(value % PINTA_DECIMAL_SCALE);

    if (integral != 0)
    {
        length = integer_to_string(integral, string);
        string += length;
        result += length;
    }

    length = integer_to_string(fraction, string);
    result += length;

    return result;
}

u8 integer_from_string(wchar *string, u32 string_length, i32 *value)
{
    u8 is_negative = 0, any = 0;
    i32 result = 0;
    wchar current;
    wchar *string_end;

    i32 cut, limit;

    if (string_length == 0xFFFFFFFF)
        string_length = string_get_length(string);

    string_end = string + string_length;

    while (string < string_end && char_is_whitespace(*string))
        string++;

    is_negative = 0;
    cut = 214748364;
    limit = 7;

    if (*string == PINTA_CHAR('-'))
    {
        is_negative = 1;
        limit = 8;

        string++;
    }
    else if (*string == PINTA_CHAR('+'))
    {
        string++;
    }

    while (string < string_end &&  *string >= PINTA_CHAR('0') && *string <= PINTA_CHAR('9'))
    {
        any = 1;
        current = *string - PINTA_CHAR('0');

        if (result > cut || (result == cut && current > limit))
            return 0;

        result = (result * 10) + current;

        string++;
    }

    while (any && string < string_end && char_is_whitespace(*string))
        string++;

    if (string != string_end || !any)
        return 0;

    if (is_negative)
        result = -result;

    if (value)
        *value = result;

    return 1;
}

/* INTEGER accessors */

i32 pinta_integer_get_value(PintaHeapObject *object)
{
    pinta_assert(object != NULL);
    pinta_assert(object->block_kind == PINTA_KIND_INTEGER);

    return object->data.integer.integer_value;
}

i32 pinta_integer_ref_get_value(PintaReference *reference)
{
    pinta_assert(reference != NULL);

    return pinta_integer_get_value(reference->reference);
}

void pinta_integer_set_value(PintaHeapObject *object, i32 value)
{
    pinta_assert(object != NULL);
    pinta_assert(object->block_kind == PINTA_KIND_INTEGER);

    object->data.integer.integer_value = value;
}

void pinta_integer_ref_set_value(PintaReference *reference, i32 value)
{
    pinta_assert(reference != NULL);

    pinta_integer_set_value(reference->reference, value);
}

/* INTEGER */

void pinta_integer_init_type(PintaType *type)
{
    type->is_string = 0;

    type->to_integer = pinta_core_default_copy;
    type->to_integer_value = pinta_lib_integer_to_integer_value;
    type->to_string = pinta_lib_integer_to_string;
    type->to_decimal = pinta_lib_decimal_from_int32;
    type->to_numeric = pinta_core_default_copy;
    type->to_bool = pinta_lib_integer_to_bool;
    type->to_zero = pinta_lib_integer_alloc_zero;

#if PINTA_DEBUG
    type->debug_write = pinta_lib_integer_debug_write;
#endif
}

PintaHeapObject *pinta_integer_alloc_object_value(PintaCore *core, i32 value)
{
    PintaHeapObject *result;

    pinta_assert(core != NULL);

    if (value >= -1 && value <= 99 && core->cache != NULL)
        return &core->cache->integers[value + 1];

    result = pinta_core_alloc(core, PINTA_KIND_INTEGER, PINTA_FLAG_NONE, 1);
    if (result == NULL)
        return NULL;

    pinta_integer_set_value(result, value);
    return result;
}

PintaException pinta_lib_integer_alloc_value(PintaCore *core, i32 value, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    PintaHeapObject *integer;

    pinta_assert(core != NULL);
    pinta_assert(result != NULL);

    integer = pinta_integer_alloc_object_value(core, value);
    if (integer == NULL)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_MEMORY);

    result->reference = integer;

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_integer_alloc_zero(PintaCore *core, PintaReference *result)
{
    return pinta_lib_integer_alloc_value(core, 0, result);
}

PintaException pinta_lib_integer_to_string(PintaCore *core, PintaReference *value, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    wchar *string_data;
    i32 integer_value;
    u32 string_length;
    PintaHeapObject *string;

    pinta_assert(core != NULL);
    pinta_assert(value != NULL);
    pinta_assert(result != NULL);

    if (value->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (value->reference->block_kind != PINTA_KIND_INTEGER)
        PINTA_THROW(PINTA_EXCEPTION_INVALID_OPERATION);

    integer_value = pinta_integer_ref_get_value(value);
    string_length = integer_to_string(integer_value, NULL);

    string = pinta_string_alloc_object(core, string_length);
    if (string == NULL)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_MEMORY);

    string_data = pinta_string_get_data(string);
    integer_to_string(integer_value, string_data);

    result->reference = string;

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_integer_parse(PintaCore *core, PintaReference *value, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    u8 isFailed;

    pinta_assert(core != NULL);
    pinta_assert(value != NULL);
    pinta_assert(result != NULL);

    PINTA_CHECK(pinta_lib_integer_try_parse(core, value, &isFailed, result));

    if (isFailed)
        PINTA_THROW(PINTA_EXCEPTION_BAD_FORMAT);

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_integer_try_parse(PintaCore *core, PintaReference *value, u8 *isFailed, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    i32 parsed_value;
    PintaHeapObject *integer;

    pinta_assert(core != NULL);

    PINTA_CHECK(pinta_lib_integer_try_parse_value(core, value, isFailed, &parsed_value));
    integer = pinta_integer_alloc_object_value(core, parsed_value);
    if (integer == NULL)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_MEMORY);

    result->reference = integer;

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_integer_try_parse_value(PintaCore *core, PintaReference *value, u8 *isFailed, i32 *result)
{
    PintaException exception = PINTA_OK;
    PintaType *type;
    u8 failed;

    pinta_assert(core != NULL);
    pinta_assert(value != NULL);
    pinta_assert(result != NULL);

    if (isFailed)
        *isFailed = 1;

    if (value->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    type = pinta_core_get_type(core, value);
    if (!type->is_string)
        PINTA_THROW(PINTA_EXCEPTION_INVALID_OPERATION);

    PINTA_CHECK(pinta_lib_string_to_string(core, value, value));

    failed = !integer_from_string(pinta_string_ref_get_data(value), pinta_string_ref_get_length(value), result);
    if (failed == 1)
        *result = 0;

    if (isFailed)
        *isFailed = failed;

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_integer_to_bool(PintaCore *core, PintaReference *value, u8 *result)
{
    pinta_assert(core != NULL);
    pinta_assert(value != NULL);
    pinta_assert(value->reference != NULL);
    pinta_assert(result != NULL);

    if (pinta_integer_ref_get_value(value) == 0)
        *result = 0;
    else
        *result = 1;

    return PINTA_OK;
}

PintaException pinta_lib_integer_to_integer_value(PintaCore *core, PintaReference *value, i32 *result)
{
    pinta_assert(core != NULL);
    pinta_assert(value != NULL);
    pinta_assert(value->reference != NULL);
    pinta_assert(result != NULL);

    *result = pinta_integer_ref_get_value(value);

    return PINTA_OK;
}

PintaException pinta_lib_integer_debug_write(PintaCore *core, PintaReference *value, u32 max_depth, PintaJsonWriter *writer)
{
    i32 integer_value;

    PINTA_UNUSED(max_depth);

    pinta_assert(core != NULL);
    pinta_assert(value != NULL);

    if (value->reference == NULL)
        return pinta_json_write_null(core, writer);

    integer_value = pinta_integer_ref_get_value(value);
    return pinta_json_write_i32(core, writer, integer_value);
}
