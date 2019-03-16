#include "pinta.h"

PintaException pinta_binary_read_uint(PintaReader *reader, u32 *result)
{
    PintaException exception = PINTA_OK;
    u32 value = 0;
    u8 *next;

    pinta_assert(reader != NULL);
    pinta_assert(reader->start != NULL);
    pinta_assert(reader->end != NULL);
    pinta_assert(reader->start <= reader->end);
    pinta_assert(reader->current != NULL);
    pinta_assert(reader->start <= reader->current);
    pinta_assert(reader->current <= reader->end);

    next = reader->current + sizeof(u32);
    if (next < reader->start || next >= reader->end)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

    value = *(u32*)reader->current;
    reader->current = next;

    *result = value;

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_binary_read_sint(PintaReader *reader, i32 *result)
{
    PintaException exception = PINTA_OK;
    u32 value = 0;

    PINTA_CHECK(pinta_binary_read_uint(reader, &value));

    *result = (i32)value;

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_binary_read_uleb128(PintaReader *reader, u32 *length, u32 *result)
{
    PintaException exception = PINTA_OK;
    u8 current;
    u32 value = 0;
    u32 shift = 0;
    u32 count = 0;

    pinta_assert(reader != NULL);
    pinta_assert(reader->start != NULL);
    pinta_assert(reader->end != NULL);
    pinta_assert(reader->start <= reader->end);
    pinta_assert(reader->current != NULL);
    pinta_assert(reader->start <= reader->current);
    pinta_assert(reader->current <= reader->end);

    do
    {
        if (count > 5)
            PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

        if (reader->current < reader->start || reader->current >= reader->end)
            PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

        current = *reader->current;
        value = value | ((u32)(current &  0x7F) << shift);
        shift += 7;

        count++;
        reader->current++;

    } while ((current & 0x80) == 0x80);

    if (length != NULL)
        *length = count;

    *result = value;

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_binary_read_uleb128p1(PintaReader *reader, u32 *length, u32 *result)
{
    PintaException exception = PINTA_OK;
    u32 value = 0;

    PINTA_CHECK(pinta_binary_read_uleb128(reader, length, &value));

    value = value - 1;
    *result = value;

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_binary_read_sleb128(PintaReader *reader, u32 *length, i32 *result)
{
    PintaException exception = PINTA_OK;
    u32 value = 0;

    PINTA_CHECK(pinta_binary_read_uleb128(reader, length, &value));

    *result = ((i32)value >> 1) ^ (-((i32)value & 1));

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_binary_write_uint(PintaWriter *writer, u32 value)
{
    PintaException exception = PINTA_OK;
    u8 *next;

    pinta_assert(writer != NULL);
    pinta_assert(writer->start != NULL);
    pinta_assert(writer->end != NULL);
    pinta_assert(writer->start <= writer->end);
    pinta_assert(writer->current != NULL);
    pinta_assert(writer->start <= writer->current);
    pinta_assert(writer->current <= writer->end);

    next = writer->current + sizeof(u32);
    if (next < writer->start || next >= writer->end)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

    *(u32*)writer->current = value;
    writer->current = next;

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_binary_write_sint(PintaWriter *writer, i32 value)
{
    return pinta_binary_write_uint(writer, (u32)value);
}

PintaException pinta_binary_write_uleb128(PintaWriter *writer, u32 *length, u32 value)
{
    PintaException exception = PINTA_OK;
    u8 current;
    u32 count = 0;

    pinta_assert(writer != NULL);
    pinta_assert(writer->start != NULL);
    pinta_assert(writer->end != NULL);
    pinta_assert(writer->start <= writer->end);
    pinta_assert(writer->current != NULL);
    pinta_assert(writer->start <= writer->current);
    pinta_assert(writer->current <= writer->end);

    do
    {
        if (count > 5)
            PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

        if (writer->current < writer->start || writer->current >= writer->end)
            PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

        current = (u8)(value & 0x7F);
        value = value >> 7;

        if (value != 0)
            current |= 0x80;

        *writer->current = current;

        count++;
        writer->current++;

    } while (value != 0);

    if (length != NULL)
        *length = count;

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_binary_write_uleb128p1(PintaWriter *writer, u32 *length, u32 value)
{
    value = value + 1;
    return pinta_binary_write_uleb128(writer, length, value);
}

PintaException pinta_binary_write_sleb128(PintaWriter *writer, u32 *length, i32 value)
{
    u32 uvalue = (u32)(value << 1) ^ (u32)(value >> 31);
    return pinta_binary_write_uleb128(writer, length, uvalue);
}
