#include "pinta.h"

static const u8 leadingBits[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4,
    4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 7, 8 };

PintaException pinta_lib_decode_utf8(PintaCore *core, u8 *data, u32 data_length, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    u32 item = 0xFFFFFFFF;
    u32 count;
    u8 *end;
    u8 first = 1;
    PintaReference buffer;

    pinta_assert(core != NULL);
    pinta_assert(data != NULL);
    pinta_assert(result != NULL);

    PINTA_GC_ENTER(core, buffer);

    PINTA_CHECK(pinta_lib_buffer_alloc(core, &buffer));
    PINTA_CHECK(pinta_lib_buffer_set_length_integer(core, &buffer, data_length));

    end = &data[data_length];
    while (data < end)
    {
        count = leadingBits[*data];
        switch (count)
        {
        case 0:
            item = (u32)*data++;
            break;

        case 1:
            PINTA_THROW(PINTA_EXCEPTION_INVALID_SEQUENCE);
            break;

        case 2:
            if (data + 2 > end)
                PINTA_THROW(PINTA_EXCEPTION_INVALID_SEQUENCE);

            item = (u32)((*data++ & 0x1F) << 6);
            break;

        case 3:
            if (data + 3 > end)
                PINTA_THROW(PINTA_EXCEPTION_INVALID_SEQUENCE);

            item = (u32)((*data++ & 0x0F) << 12);
            break;

        case 4:
            if (data + 4 > end)
                PINTA_THROW(PINTA_EXCEPTION_INVALID_SEQUENCE);

            item = (u32)((*data++ & 0x07) << 18);
            break;

        case 5:
        case 6:
        case 7:
        case 8:
        default:
            PINTA_THROW(PINTA_EXCEPTION_INVALID_SEQUENCE);
            break;
        }

        switch (count)
        {
        case 4:
            if ((*data & 0xC0) != 0x80)
                PINTA_THROW(PINTA_EXCEPTION_INVALID_SEQUENCE);
            item = item | (u32)((*data++ & 0x3F) << 12);

            // fall-through
        case 3:
            if ((*data & 0xC0) != 0x80)
                PINTA_THROW(PINTA_EXCEPTION_INVALID_SEQUENCE);
            item = item | (u32)((*data++ & 0x3F) << 6);

            // fall-through
        case 2:
            if ((*data & 0xC0) != 0x80)
                PINTA_THROW(PINTA_EXCEPTION_INVALID_SEQUENCE);
            item = item | (u32)(*data++ & 0x3F);

            // fall-through
        }

        switch (count)
        {
        case 2:
            if (item < 0x00000080)
                PINTA_THROW(PINTA_EXCEPTION_INVALID_SEQUENCE);
            break;

        case 3:
            if (item < 0x00000800)
                PINTA_THROW(PINTA_EXCEPTION_INVALID_SEQUENCE);
            break;

        case 4:
            if (item < 0x00010000)
                PINTA_THROW(PINTA_EXCEPTION_INVALID_SEQUENCE);
            break;
        }

        if (first && item == 0xFEFF) // skip bom if present
            continue;

        first = 0;

        if (item > 0xFFFF)
        {
            // wiki calls them lead and trail, normally they are called high and low surogates
            u16 lead;
            u16 trail;

            item = item - 0x10000;
            lead = (u16)(0xD800 + (item >> 10));
            trail = (u16)(0xDC00 + (item & 0x3FF));
            PINTA_CHECK(pinta_lib_buffer_write_short_value(core, &buffer, lead));
            PINTA_CHECK(pinta_lib_buffer_write_short_value(core, &buffer, trail));
        }
        else
        {
            PINTA_CHECK(pinta_lib_buffer_write_short_value(core, &buffer, (u16)item));
        }
    }

    PINTA_CHECK(pinta_lib_buffer_set_length_position(core, &buffer));
    PINTA_CHECK(pinta_lib_buffer_in_place_to_string(core, &buffer, result));

    PINTA_GC_RETURN(core);
}

PintaException pinta_lib_decode_string_c(PintaCore *core, char *data, u32 data_length, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    u32 index;
    u16 current;
    PintaReference buffer;

    pinta_assert(core != NULL);
    pinta_assert(data != NULL);
    pinta_assert(result != NULL);

    PINTA_GC_ENTER(core, buffer);

    PINTA_CHECK(pinta_lib_buffer_alloc_value(core, sizeof(wchar) * data_length, &buffer));
    PINTA_CHECK(pinta_lib_buffer_set_length_integer(core, &buffer, sizeof(wchar) * data_length));

    for (index = 0; index < data_length; index++)
    {
        current = (u16)data[index];
        PINTA_CHECK(pinta_lib_buffer_write_short_value(core, &buffer, current));
    }

    PINTA_CHECK(pinta_lib_buffer_set_length_position(core, &buffer));
    PINTA_CHECK(pinta_lib_buffer_in_place_to_string(core, &buffer, result));

    PINTA_GC_RETURN(core);
}

PintaException pinta_lib_decode_mutf8(PintaCore *core, u8 *data, u32 string_length, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    wchar item = 0xFFFF;
    wchar *string_data;
    u32 data_length = 0;
    u32 count;
    PintaHeapObject *string;

    pinta_assert(core != NULL);
    pinta_assert(data != NULL);
    pinta_assert(result != NULL);

    string = pinta_string_alloc_object(core, string_length);
    if (string == NULL)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_MEMORY);
    string_data = pinta_string_get_data(string);

    while (*data != 0)
    {
        count = leadingBits[*data];
        switch (count)
        {
        case 0:
            item = (wchar)*data++;
            break;

        case 1:
            PINTA_THROW(PINTA_EXCEPTION_INVALID_SEQUENCE);
            break;

        case 2:
            item = (wchar)((*data++ & 0x1F) << 6);
            break;

        case 3:
            item = (wchar)((*data++ & 0x0F) << 12);
            break;

        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        default:
            PINTA_THROW(PINTA_EXCEPTION_INVALID_SEQUENCE);
            break;
        }

        switch (count)
        {
        case 3:
            if ((*data & 0xC0) != 0x80)
                PINTA_THROW(PINTA_EXCEPTION_INVALID_SEQUENCE);
            item = item | (wchar)((*data++ & 0x3F) << 6);

            // fall-through
        case 2:
            if ((*data & 0xC0) != 0x80)
                PINTA_THROW(PINTA_EXCEPTION_INVALID_SEQUENCE);
            item = item | (wchar)(*data++ & 0x3F);

            // fall-through
        }

        switch (count)
        {
        case 2:
            if (item < 0x0080 && item != 0x0000)
                PINTA_THROW(PINTA_EXCEPTION_INVALID_SEQUENCE);
            break;

        case 3:
            if (item < 0x0800)
                PINTA_THROW(PINTA_EXCEPTION_INVALID_SEQUENCE);
            break;
        }

        if (data_length == 0 && item == 0xFEFF) // skip BOM
            continue;

        data_length++;
        if (data_length > string_length)
            PINTA_THROW(PINTA_EXCEPTION_INVALID_SEQUENCE);

        *string_data++ = item;
    }

    result->reference = string;
PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_buffer_write_bom_utf8(PintaCore *core, PintaReference *buffer)
{
    static u8 bom[3] = { (u8)(((0xFEFF >> 12) & 0x0F) | 0xE0), (u8)(((0xFEFF >> 6) & 0x3F) | 0x80), (u8)((0xFEFF & 0x3F) | 0x80) };

    return pinta_lib_buffer_write_data(core, buffer, bom, 3);
}

PintaException pinta_lib_buffer_write_string_utf8(PintaCore *core, PintaReference *buffer, PintaReference *string)
{
    PintaException exception = PINTA_OK;
    u32 current;
    u32 next;
    u32 index;
    u32 string_length;
    u8 code[4] = { 0 };
    u32 code_length = 0;
    u32 current_position;
    u32 current_length;

    pinta_assert(core != NULL);
    pinta_assert(buffer != NULL);
    pinta_assert(string != NULL);

    if (buffer->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (buffer->reference->block_kind != PINTA_KIND_BUFFER)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    if (string->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (string->reference->block_kind != PINTA_KIND_STRING
        && string->reference->block_kind != PINTA_KIND_SUBSTRING
        && string->reference->block_kind != PINTA_KIND_MULTISTRING
        && string->reference->block_kind != PINTA_KIND_CHAR)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    PINTA_CHECK(pinta_lib_string_to_string(core, string, string));
    string_length = pinta_string_ref_get_length(string);

    current_position = pinta_buffer_ref_get_position(buffer);
    current_length = pinta_buffer_ref_get_length(buffer);

    if (current_length < (current_position + string_length)) // optimization, not required
        PINTA_CHECK(pinta_lib_buffer_set_length_integer(core, buffer, current_position + string_length));

    for (index = 0; index < string_length; index++)
    {
        current = (u32)pinta_string_ref_get_item(string, index);

        if (current >= 0xD800 && current < 0xDC00)
        {
            if (index + 1 < string_length)
            {
                next = (u32)pinta_string_ref_get_item(string, index + 1);
                if (next >= 0xDC00 && next < 0xC000)
                {
                    u32 high = current - 0xD800;
                    u32 low = next - 0xDC00;

                    current = ((high << 10) | low) + 0x10000;
                    index++;

                }
            }
        }

        if (current < 0x0080 && current != 0x0000)
        {
            code[0] = (u8)current;
            code_length = 1;
        }
        else if (current < 0x0800)
        {
            code[0] = (u8)(((current >> 6) & 0x1F) | 0xC0);
            code[1] = (u8)((current & 0x3F) | 0x80);
            code_length = 2;
        }
        else if (current < 0x10000)
        {
            code[0] = (u8)(((current >> 12) & 0x0F) | 0xE0);
            code[1] = (u8)(((current >> 6) & 0x3F) | 0x80);
            code[2] = (u8)((current & 0x3F) | 0x80);
            code_length = 3;
        }
        else
        {
            code[0] = (u8)(((current >> 18) & 0x07) | 0xF0);
            code[1] = (u8)(((current >> 12) & 0x3F) | 0x80);
            code[2] = (u8)(((current >> 6) & 0x3F) | 0x80);
            code[3] = (u8)((current & 0x3F) | 0x80);
            code_length = 4;
        }

        PINTA_CHECK(pinta_lib_buffer_write_data(core, buffer, code, code_length));
    }

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_buffer_write_bom_mutf8(PintaCore *core, PintaReference *buffer)
{
    static u8 bom[3] = { (u8)(((0xFEFF >> 12) & 0x0F) | 0xE0), (u8)(((0xFEFF >> 6) & 0x3F) | 0x80), (u8)((0xFEFF & 0x3F) | 0x80) };

    return pinta_lib_buffer_write_data(core, buffer, bom, 3);
}

PintaException pinta_lib_buffer_write_string_mutf8(PintaCore *core, PintaReference *buffer, PintaReference *string)
{
    PintaException exception = PINTA_OK;
    wchar current;
    u32 index;
    u32 string_length;
    u8 code[3];
    u32 code_length;
    u32 current_position;
    u32 current_length;

    pinta_assert(core != NULL);
    pinta_assert(buffer != NULL);
    pinta_assert(string != NULL);

    if (buffer->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (buffer->reference->block_kind != PINTA_KIND_BUFFER)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    if (string->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (string->reference->block_kind != PINTA_KIND_STRING
        && string->reference->block_kind != PINTA_KIND_SUBSTRING
        && string->reference->block_kind != PINTA_KIND_MULTISTRING
        && string->reference->block_kind != PINTA_KIND_CHAR)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    PINTA_CHECK(pinta_lib_string_to_string(core, string, string));
    string_length = pinta_string_ref_get_length(string);

    current_position = pinta_buffer_ref_get_position(buffer);
    current_length = pinta_buffer_ref_get_length(buffer);

    if (current_length < (current_position + string_length)) // optimization, not required
        PINTA_CHECK(pinta_lib_buffer_set_length_integer(core, buffer, current_position + string_length));

    for (index = 0; index < string_length; index++)
    {
        current = pinta_string_ref_get_item(string, index);

        if (current <= 0x007F && current != 0x0000)
        {
            code[0] = (u8)current;
            code_length = 1;
        }
        else if (current <= 0x07FF)
        {
            code[0] = (u8)(((current >> 6) & 0x1F) | 0xC0);
            code[1] = (u8)((current & 0x3F) | 0x80);
            code_length = 2;
        }
        else
        {
            code[0] = (u8)(((current >> 12) & 0x0F) | 0xE0);
            code[1] = (u8)(((current >> 6) & 0x3F) | 0x80);
            code[2] = (u8)((current & 0x3F) | 0x80);
            code_length = 3;
        }

        PINTA_CHECK(pinta_lib_buffer_write_data(core, buffer, code, code_length));
    }

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_buffer_write_bom_utf16(PintaCore *core, PintaReference *buffer)
{
    return pinta_lib_buffer_write_short_value(core, buffer, 0xFEFF);
}
