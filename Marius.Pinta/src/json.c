#include "pinta.h"

#define PINTA_JSON_ARRAY        4
#define PINTA_JSON_NAME         2
#define PINTA_JSON_VALUE        1

#define PINTA_JSON_MASK         7
#define PINTA_JSON_LAST_MASK    0xE0000000
#define PINTA_JSON_SIZE         3

static wchar null_literal[] = { 110, 117, 108, 108, 0 };
static wchar true_literal[] = { 116, 114, 117, 101, 0 };
static wchar false_literal[] = { 102, 97, 108, 115, 101, 0 };

static wchar skipped_literal[] = { 36, 115, 107, 105, 112, 112, 101, 100, 0 };

static wchar base64_literal[] = { 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 43, 47, 61, 0 };
static wchar hex_literal[] = { 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 65, 66, 67, 68, 69, 70, 0 };

static wchar open_brace_literal[] = { 123, 0 };
static wchar close_brace_literal[] = { 125, 0 };

static wchar open_bracket_literal[] = { 91, 0 };
static wchar close_bracket_literal[] = { 93, 0 };

static wchar quote_literal[] = { 34, 0 };
static wchar slash_literal[] = { 92, 0 };
static wchar comma_literal[] = { 44, 32, 0 }; // ", "
static wchar colon_literal[] = { 58, 32, 0 }; // ": "

void pinta_json_stack_set_value(PintaJsonWriter *writer)
{
    pinta_assert(writer != NULL);

    writer->stack = (writer->stack & ~PINTA_JSON_NAME) | PINTA_JSON_VALUE;
}

void pinta_json_stack_set_name(PintaJsonWriter *writer)
{
    pinta_assert(writer != NULL);

    writer->stack = (writer->stack & ~PINTA_JSON_VALUE) | PINTA_JSON_NAME;
}

PintaException pinta_json_write_raw(PintaCore *core, PintaJsonWriter *writer, u32 emit_separator, wchar *value, u32 length)
{
    PintaException exception = PINTA_OK;

    if (emit_separator)
    {
        if (writer->stack & PINTA_JSON_VALUE)
        {
            PINTA_CHECK(writer->write(core, writer, comma_literal, PINTA_LITERAL_LENGTH(comma_literal)));
            writer->stack = writer->stack & ~PINTA_JSON_VALUE;
        }

        if (writer->stack & PINTA_JSON_NAME)
        {
            PINTA_CHECK(writer->write(core, writer, colon_literal, PINTA_LITERAL_LENGTH(colon_literal)));
            writer->stack = writer->stack & ~PINTA_JSON_NAME;
        }
    }

    PINTA_CHECK(writer->write(core, writer, value, length));

PINTA_EXIT:
    return exception;
}

PintaException pinta_json_write_skipped(PintaCore *core, PintaJsonWriter *writer)
{
    return pinta_json_write_string(core, writer, skipped_literal, PINTA_LITERAL_LENGTH(skipped_literal));
}

PintaException pinta_json_write_null(PintaCore *core, PintaJsonWriter *writer)
{
    PintaException exception = PINTA_OK;
    exception = pinta_json_write_raw(core, writer, 1, null_literal, PINTA_LITERAL_LENGTH(null_literal));

    pinta_json_stack_set_value(writer);

    return exception;
}

PintaException pinta_json_write_bool(PintaCore *core, PintaJsonWriter *writer, u32 value)
{
    PintaException exception = PINTA_OK;

    if (value != 0)
        exception = pinta_json_write_raw(core, writer, 1, true_literal, PINTA_LITERAL_LENGTH(true_literal));
    else
        exception = pinta_json_write_raw(core, writer, 1, false_literal, PINTA_LITERAL_LENGTH(false_literal));

    pinta_json_stack_set_value(writer);

    return exception;
}

PintaException pinta_json_write_i32(PintaCore *core, PintaJsonWriter *writer, i32 value)
{
    PintaException exception = PINTA_OK;
    wchar buffer[PINTA_INT_DIGITS];
    u32 length;

    length = integer_to_string(value, buffer);
    exception = pinta_json_write_raw(core, writer, 1, buffer, length);

    pinta_json_stack_set_value(writer);

    return exception;
}

PintaException pinta_json_write_u32(PintaCore *core, PintaJsonWriter *writer, u32 value)
{
    PintaException exception = PINTA_OK;
    wchar buffer[PINTA_INT_DIGITS];
    u32 length;

    length = integer_to_string((i32)value, buffer);
    exception = pinta_json_write_raw(core, writer, 1, buffer, length);

    pinta_json_stack_set_value(writer);

    return exception;
}

PintaException pinta_json_write_decimal(PintaCore *core, PintaJsonWriter *writer, decimal value)
{
    PintaException exception = PINTA_OK;
    wchar buffer[PINTA_DECIMAL_DIGITS];
    u32 length;

    length = decimal_to_string(value, buffer);
    exception = pinta_json_write_raw(core, writer, 1, buffer, length);

    pinta_json_stack_set_value(writer);

    return exception;
}

PintaException pinta_json_write_string(PintaCore *core, PintaJsonWriter *writer, wchar *data, u32 length)
{
    PintaException exception = PINTA_OK;
    wchar *end;
    wchar hex[6];
    u32 escape = 0;
    wchar current;

    end = &data[length];

    hex[0] = PINTA_CHAR('\\');
    hex[1] = PINTA_CHAR('u');

    PINTA_CHECK(pinta_json_write_raw(core, writer, 1, quote_literal, PINTA_LITERAL_LENGTH(quote_literal)));

    while (data < end)
    {
        escape = 0;

        current = *data;
        switch (current)
        {
        case PINTA_CHAR('"'):
        case PINTA_CHAR('\\'):
        case PINTA_CHAR('/'):
            escape = 1;
            break;
        case PINTA_CHAR('\b'):
            escape = 1;
            current = PINTA_CHAR('b');
            break;
        case PINTA_CHAR('\f'):
            escape = 1;
            current = PINTA_CHAR('f');
            break;
        case PINTA_CHAR('\n'):
            escape = 1;
            current = PINTA_CHAR('n');
            break;
        case PINTA_CHAR('\r'):
            escape = 1;
            current = PINTA_CHAR('r');
            break;
        case PINTA_CHAR('\t'):
            escape = 1;
            current = PINTA_CHAR('t');
            break;
        }

        if (!escape && current < 0x20)
        {
            hex[2] = hex_literal[(current >> 12) & 0x0F];
            hex[3] = hex_literal[(current >> 8) & 0x0F];
            hex[4] = hex_literal[(current >> 4) & 0x0F];
            hex[5] = hex_literal[current & 0x0F];

            PINTA_CHECK(pinta_json_write_raw(core, writer, 0, hex, 6));
        }
        else
        {
            if (escape)
                PINTA_CHECK(pinta_json_write_raw(core, writer, 0, slash_literal, PINTA_LITERAL_LENGTH(slash_literal)));

            PINTA_CHECK(pinta_json_write_raw(core, writer, 0, &current, 1));
        }

        data++;
    }

    PINTA_CHECK(pinta_json_write_raw(core, writer, 0, quote_literal, PINTA_LITERAL_LENGTH(quote_literal)));

    pinta_json_stack_set_value(writer);

PINTA_EXIT:
    return exception;
}

PintaException pinta_json_write_string_c(PintaCore *core, PintaJsonWriter *writer, char *data, u32 length)
{
    PintaException exception = PINTA_OK;
    char *end;
    wchar hex[6];
    u32 escape = 0;
    wchar current;

    if (length == 0xFFFFFFFF)
        length = string_get_length_c(data);

    end = &data[length];

    hex[0] = PINTA_CHAR('\\');
    hex[1] = PINTA_CHAR('u');

    PINTA_CHECK(pinta_json_write_raw(core, writer, 1, quote_literal, PINTA_LITERAL_LENGTH(quote_literal)));

    while (data < end)
    {
        escape = 0;

        current = (wchar)*data;
        switch (current)
        {
        case '"':
        case '\\':
        case '/':
            escape = 1;
            break;
        case '\b':
            escape = 1;
            current = PINTA_CHAR('b');
            break;
        case '\f':
            escape = 1;
            current = PINTA_CHAR('f');
            break;
        case '\n':
            escape = 1;
            current = PINTA_CHAR('n');
            break;
        case '\r':
            escape = 1;
            current = PINTA_CHAR('r');
            break;
        case '\t':
            escape = 1;
            current = PINTA_CHAR('t');
            break;
        }

        if (!escape && current < 0x20)
        {
            hex[2] = hex_literal[(current >> 12) & 0x0F];
            hex[3] = hex_literal[(current >> 8) & 0x0F];
            hex[4] = hex_literal[(current >> 4) & 0x0F];
            hex[5] = hex_literal[current & 0x0F];

            PINTA_CHECK(pinta_json_write_raw(core, writer, 0, hex, 6));
        }
        else
        {
            if (escape)
                PINTA_CHECK(pinta_json_write_raw(core, writer, 0, slash_literal, PINTA_LITERAL_LENGTH(slash_literal)));

            PINTA_CHECK(pinta_json_write_raw(core, writer, 0, &current, 1));
        }

        data++;
    }

    PINTA_CHECK(pinta_json_write_raw(core, writer, 0, quote_literal, PINTA_LITERAL_LENGTH(quote_literal)));

    pinta_json_stack_set_value(writer);

PINTA_EXIT:
    return exception;
}

PintaException pinta_json_write_binary(PintaCore *core, PintaJsonWriter *writer, u8 *data, u32 length)
{
    PintaException exception = PINTA_OK;
    u32 i = 0;
    wchar output[5];

    PINTA_CHECK(pinta_json_write_raw(core, writer, 1, quote_literal, PINTA_LITERAL_LENGTH(quote_literal)));

    while (i < length)
    {
        u32 idx_0, idx_1, idx_2, idx_3;
        u32 i0;

        i0 = (u32)(data[i]) << 16; i++;
        i0 |= (u32)(i < length ? data[i] : 0) << 8; i++;
        i0 |= (i < length ? data[i] : 0); i++;

        idx_0 = (i0 & 0xfc0000) >> 18; i0 <<= 6;
        idx_1 = (i0 & 0xfc0000) >> 18; i0 <<= 6;
        idx_2 = (i0 & 0xfc0000) >> 18; i0 <<= 6;
        idx_3 = (i0 & 0xfc0000) >> 18;

        if (i - 1 > length) idx_2 = 64;
        if (i > length) idx_3 = 64;

        output[0] = base64_literal[idx_0];
        output[1] = base64_literal[idx_1];
        output[2] = base64_literal[idx_2];
        output[3] = base64_literal[idx_3];

        PINTA_CHECK(pinta_json_write_raw(core, writer, 0, output, 4));
    }

    PINTA_CHECK(pinta_json_write_raw(core, writer, 0, quote_literal, PINTA_LITERAL_LENGTH(quote_literal)));

    pinta_json_stack_set_value(writer);

PINTA_EXIT:
    return exception;
}

PintaException pinta_json_write_array_start(PintaCore *core, PintaJsonWriter *writer)
{
    PintaException exception = PINTA_OK;

    PINTA_CHECK(pinta_json_write_raw(core, writer, 1, open_bracket_literal, PINTA_LITERAL_LENGTH(open_bracket_literal)));

    if (writer->stack & PINTA_JSON_LAST_MASK)
        PINTA_THROW(PINTA_EXCEPTION_STACK_OVERFLOW);

    writer->stack = (writer->stack << PINTA_JSON_SIZE) | PINTA_JSON_ARRAY;

PINTA_EXIT:
    return exception;
}

PintaException pinta_json_write_object_start(PintaCore *core, PintaJsonWriter *writer)
{
    PintaException exception = PINTA_OK;

    PINTA_CHECK(pinta_json_write_raw(core, writer, 1, open_brace_literal, PINTA_LITERAL_LENGTH(open_brace_literal)));

    if (writer->stack & PINTA_JSON_LAST_MASK)
        PINTA_THROW(PINTA_EXCEPTION_STACK_OVERFLOW);

    writer->stack = (writer->stack << PINTA_JSON_SIZE);

PINTA_EXIT:
    return exception;
}

PintaException pinta_json_write_end(PintaCore *core, PintaJsonWriter *writer)
{
    PintaException exception = PINTA_OK;

    if ((writer->stack & PINTA_JSON_ARRAY) != 0)
    {
        PINTA_CHECK(pinta_json_write_raw(core, writer, 0, close_bracket_literal, PINTA_LITERAL_LENGTH(close_bracket_literal)));
    }
    else
    {
        if ((writer->stack & PINTA_JSON_NAME) != 0)
            PINTA_THROW(PINTA_EXCEPTION_INVALID_OPCODE);

        PINTA_CHECK(pinta_json_write_raw(core, writer, 0, close_brace_literal, PINTA_LITERAL_LENGTH(close_brace_literal)));
    }

    writer->stack = (writer->stack >> PINTA_JSON_SIZE);
    pinta_json_stack_set_value(writer);

PINTA_EXIT:
    return exception;
}

PintaException pinta_json_write_property_name(PintaCore *core, PintaJsonWriter *writer, wchar *name)
{
    PintaException exception = PINTA_OK;

    if ((writer->stack & PINTA_JSON_ARRAY) != 0) // arrays do not have properties
        return PINTA_EXCEPTION_INVALID_OPERATION;

    if ((writer->stack & PINTA_JSON_NAME) != 0) // we've already emitted property name, waiting for value
        return PINTA_EXCEPTION_INVALID_OPCODE;

    exception = pinta_json_write_string(core, writer, name, string_get_length(name));

    pinta_json_stack_set_name(writer);

    return exception;
}

PintaException pinta_json_write_property_name_c(PintaCore *core, PintaJsonWriter *writer, char *name)
{
    PintaException exception = PINTA_OK;

    if ((writer->stack & PINTA_JSON_ARRAY) != 0) // arrays do not have properties
        return PINTA_EXCEPTION_INVALID_OPERATION;

    if ((writer->stack & PINTA_JSON_NAME) != 0) // we've already emitted property name, waiting for value
        return PINTA_EXCEPTION_INVALID_OPCODE;

    exception = pinta_json_write_string_c(core, writer, name, string_get_length_c(name));

    pinta_json_stack_set_name(writer);

    return exception;
}

PintaException pinta_json_write_property_string(PintaCore *core, PintaJsonWriter *writer, wchar *name, wchar *value)
{
    PintaException exception = PINTA_OK;

    PINTA_CHECK(pinta_json_write_property_name(core, writer, name));
    PINTA_CHECK(pinta_json_write_string(core, writer, value, string_get_length(value)));

PINTA_EXIT:
    return exception;
}

PintaException pinta_json_write_property_i32(PintaCore *core, PintaJsonWriter *writer, wchar *name, i32 value)
{
    PintaException exception = PINTA_OK;

    PINTA_CHECK(pinta_json_write_property_name(core, writer, name));
    PINTA_CHECK(pinta_json_write_i32(core, writer, value));

PINTA_EXIT:
    return exception;
}

PintaException pinta_json_write_property_u32(PintaCore *core, PintaJsonWriter *writer, wchar *name, u32 value)
{
    PintaException exception = PINTA_OK;

    PINTA_CHECK(pinta_json_write_property_name(core, writer, name));
    PINTA_CHECK(pinta_json_write_u32(core, writer, value));

PINTA_EXIT:
    return exception;
}

PintaException pinta_json_write_property_bool(PintaCore *core, PintaJsonWriter *writer, wchar *name, u32 value)
{
    PintaException exception = PINTA_OK;

    PINTA_CHECK(pinta_json_write_property_name(core, writer, name));
    PINTA_CHECK(pinta_json_write_bool(core, writer, value));

PINTA_EXIT:
    return exception;
}

PintaException pinta_json_write_property_binary(PintaCore *core, PintaJsonWriter *writer, wchar *name, u8 *data, u32 length)
{
    PintaException exception = PINTA_OK;

    PINTA_CHECK(pinta_json_write_property_name(core, writer, name));
    PINTA_CHECK(pinta_json_write_binary(core, writer, data, length));

PINTA_EXIT:
    return exception;
}

PintaException pinta_json_write_property_null(PintaCore *core, PintaJsonWriter *writer, wchar *name)
{
    PintaException exception = PINTA_OK;

    PINTA_CHECK(pinta_json_write_property_name(core, writer, name));
    PINTA_CHECK(pinta_json_write_null(core, writer));

PINTA_EXIT:
    return exception;
}
