#include "pinta.h"

PintaException pinta_pattern_simple(PintaReference *string, PintaReference *pattern, u32 offset, u32 length, u32 *result)
{
    PintaException exception = PINTA_OK;
    u32 string_length, pattern_length;
    wchar *string_data, *pattern_data;
    u32 is_match, index;

    pinta_assert(string != NULL);
    pinta_assert(pattern != NULL);

    pinta_assert(string->reference != NULL);
    pinta_assert(pattern->reference != NULL);
    pinta_assert(string->reference->block_kind == PINTA_KIND_STRING);
    pinta_assert(pattern->reference->block_kind == PINTA_KIND_STRING);

    if (result)
        *result = 0;

    string_length = pinta_string_ref_get_length(string);
    pattern_length = pinta_string_ref_get_length(pattern);

    if (offset > string_length)
        PINTA_RETURN();

    if (length > pattern_length)
        PINTA_RETURN();

    if (length > (string_length - offset))
        PINTA_RETURN();

    string_data = pinta_string_ref_get_data(string) + offset;
    pattern_data = pinta_string_ref_get_data(pattern);

    is_match = 1;
    for (index = 0; index < length; index++, string_data++, pattern_data++)
    {
        if (*string_data != *pattern_data)
        {
            is_match = 0;
            break;
        }
    }

    if (result)
        *result = is_match;

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_pattern_pan(PintaReference *string, PintaReference *pattern, u32 length, u32 *result)
{
    PintaException exception = PINTA_OK;
    u32 string_length, pattern_length;
    wchar *string_data, *pattern_data;
    u32 is_match, index;

    pinta_assert(string != NULL);
    pinta_assert(pattern != NULL);

    pinta_assert(string->reference != NULL);
    pinta_assert(pattern->reference != NULL);
    pinta_assert(string->reference->block_kind == PINTA_KIND_STRING);
    pinta_assert(pattern->reference->block_kind == PINTA_KIND_STRING);

    if (result)
        *result = 0;

    string_length = pinta_string_ref_get_length(string);
    pattern_length = pinta_string_ref_get_length(pattern);

    if (length != string_length)
        PINTA_RETURN();

    if (pattern_length > string_length)
        PINTA_RETURN();

    string_data = pinta_string_ref_get_data(string);
    pattern_data = pinta_string_ref_get_data(pattern);

    is_match = 1;
    for (index = 0; index < pattern_length; index++, string_data++, pattern_data++)
    {
        if (*string_data != *pattern_data)
        {
            is_match = 0;
            break;
        }
    }

    for (; index < string_length; index++, string_data++)
    {
        if (*string_data < '0' || *string_data > '9')
        {
            is_match = 0;
            break;
        }
    }

    if (result)
        *result = is_match;

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_pattern_simple(PintaCore *core, PintaReference *string, PintaReference *pattern, PintaReference *offset, PintaReference *length, u32 *result)
{
    PintaException exception = PINTA_OK;
    u32 string_length;
    i32 offset_value, length_value;

    pinta_assert(core != NULL);
    pinta_assert(string != NULL);
    pinta_assert(pattern != NULL);
    pinta_assert(offset != NULL);
    pinta_assert(length != NULL);

    if (result)
        *result = 0;

    if (string->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (pattern->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (string->reference->block_kind != PINTA_KIND_STRING)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    if (pattern->reference->block_kind != PINTA_KIND_STRING)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    if (offset->reference == NULL)
    {
        offset_value = 0;
    }
    else
    {
        if (offset->reference->block_kind != PINTA_KIND_DECIMAL && offset->reference->block_kind != PINTA_KIND_INTEGER)
            PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

        if (offset->reference->block_kind == PINTA_KIND_DECIMAL)
            PINTA_CHECK(pinta_lib_decimal_to_int32_value(core, offset, &offset_value));
        else
            offset_value = pinta_integer_ref_get_value(offset);
    }

    if (length->reference == NULL)
    {
        length_value = (i32)pinta_string_ref_get_length(pattern);
    }
    else
    {
        if (length->reference->block_kind != PINTA_KIND_DECIMAL && length->reference->block_kind != PINTA_KIND_INTEGER)
            PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

        if (length->reference->block_kind == PINTA_KIND_DECIMAL)
            PINTA_CHECK(pinta_lib_decimal_to_int32_value(core, length, &length_value));
        else
            length_value = pinta_integer_ref_get_value(length);
    }

    if (offset_value < 0)
    {
        string_length = pinta_string_ref_get_length(string);
        offset_value = (i32)string_length + offset_value;
    }

    if (offset_value < 0)
        PINTA_RETURN();

    if (length_value < 0)
        PINTA_RETURN();

    PINTA_CHECK(pinta_pattern_simple(string, pattern, (u32)offset_value, (u32)length_value, result));

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_pattern_pan(PintaCore *core, PintaReference *string, PintaReference *pattern, PintaReference *length, u32 *result)
{
    PintaException exception = PINTA_OK;
    i32 length_value;

    pinta_assert(core != NULL);
    pinta_assert(string != NULL);
    pinta_assert(pattern != NULL);
    pinta_assert(length != NULL);

    if (result)
        *result = 0;

    if (string->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (pattern->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (string->reference->block_kind != PINTA_KIND_STRING)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    if (pattern->reference->block_kind != PINTA_KIND_STRING)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    if (length->reference == NULL)
    {
        length_value = (i32)pinta_string_ref_get_length(string);
    }
    else
    {
        if (length->reference->block_kind != PINTA_KIND_DECIMAL && length->reference->block_kind != PINTA_KIND_INTEGER)
            PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

        if (length->reference->block_kind == PINTA_KIND_DECIMAL)
            PINTA_CHECK(pinta_lib_decimal_to_int32_value(core, length, &length_value));
        else
            length_value = pinta_integer_ref_get_value(length);
    }

    if (length_value < 0)
        PINTA_RETURN();

    PINTA_CHECK(pinta_pattern_pan(string, pattern, (u32)length_value, result));

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}
