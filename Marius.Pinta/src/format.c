#include "pinta.h"

u8 pinta_format_parse_flag(wchar flag, PintaFormat *format)
{
    switch (flag)
    {
    case PINTA_CHAR(' '):
        format->print_sign = 1;
        format->plus = PINTA_CHAR(' ');
        return 1;

    case PINTA_CHAR('-'):
        format->is_right_padding = 0;
        return 1;

    case PINTA_CHAR('+'):
        format->print_sign = 1;
        format->plus = PINTA_CHAR('+');
        return 1;

    case PINTA_CHAR('0'):
        format->pad = PINTA_CHAR('0');
        return 1;

    case PINTA_CHAR('#'):
        format->print_decimal = 1;
        return 1;
    }

    return 0;
}

u8 pinta_format_parse_number(wchar digit, i32 *result)
{
    if (digit >= PINTA_CHAR('0') && digit <= PINTA_CHAR('9'))
    {
        *result = (*result * 10) + (digit - PINTA_CHAR('0'));
        return 1;
    }

    return 0;
}

void pinta_format_parse(PintaReference *format_string, i32 start, PintaFormat *format)
{
    wchar *data;
    i32 index;
    i32 length;

    pinta_assert(format_string != NULL);
    pinta_assert(format_string->reference != NULL);
    pinta_assert(format_string->reference->block_kind == PINTA_KIND_STRING);
    pinta_assert(start >= 0);
    pinta_assert(format != NULL);

    format->format_end = -1;

    data = pinta_string_ref_get_data(format_string);
    length = (i32)pinta_string_ref_get_length(format_string);

    if (start >= length)
        return;

    index = start;
    if (data[index] != PINTA_CHAR('%'))
        return;

    /* default values */
    format->is_right_padding = 1;
    format->is_variable_width = 0;
    format->is_variable_precision = 0;

    format->plus = PINTA_CHAR('+');
    format->pad = PINTA_CHAR(' ');
    format->precision = -1;
    format->print_decimal = 0;
    format->print_sign = 0;
    format->specifier = PINTA_CHAR('s');
    format->width = 0;

    /* skip '%' */
    index++;

    /* parse flags */
    while (index < length && pinta_format_parse_flag(data[index], format))
        index++;

    /* parse width */
    if (index < length && data[index] == PINTA_CHAR('*'))
    {
        format->is_variable_width = 1;
        index++;
    }
    else
    {
        while (index < length && pinta_format_parse_number(data[index], &format->width))
            index++;
    }

    /* parse precision */
    if (index < length && data[index] == PINTA_CHAR('.'))
    {
        index++;

        format->precision = 0;
        if (index < length && data[index] == PINTA_CHAR('*'))
        {
            format->is_variable_precision = 1;
            index++;
        }
        else
        {
            while (index < length && pinta_format_parse_number(data[index], &format->precision))
                index++;
        }
    }

    /* parse specifier */
    if (index < length)
    {
        switch (data[index])
        {
        case PINTA_CHAR('d'):
        case PINTA_CHAR('i'):
        case PINTA_CHAR('e'):
        case PINTA_CHAR('E'):
        case PINTA_CHAR('f'):
        case PINTA_CHAR('g'):
        case PINTA_CHAR('G'):
        case PINTA_CHAR('s'):
        case PINTA_CHAR('%'):
            format->specifier = data[index];
            break;
        default:
            return;
        }
    }
    else
    {
        return;
    }

    if (format->is_right_padding)
        format->pad = PINTA_CHAR(' ');

    index++;
    format->format_end = index;
}

PintaException pinta_format_decimal(PintaCore *core, PintaReference *buffer, PintaReference *value, PintaFormat *format)
{
    PintaException exception = PINTA_OK;

    decimal copy;
    u32 is_negative = 0;
    u32 width, precision;
    wchar string[PINTA_DECIMAL_DIGITS];
    wchar *exp, *dot, *end;
    wchar *output;
    u32 index = 0;
    PintaHeapObject *blob; // unsafe to use naked PintaHeapObject* but trust me "I know what I do"!

    u32 sign_length = 0;
    u32 integral_length = 0;
    u32 dot_length = 0;
    u32 fractional_length = 0;
    u32 fractional_padding_length = 0;
    u32 exponent_length = 0;
    u32 padding_length = 0;
    u32 string_length = 0;

    pinta_assert(core != NULL);
    pinta_assert(buffer != NULL);
    pinta_assert(value != NULL);
    pinta_assert(format != NULL);

    if (value->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (value->reference->block_kind != PINTA_KIND_DECIMAL)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    if (format->width <= 0)
        width = 0;
    else
        width = (u32)format->width;

    if (format->precision <= 0)
        precision = 0;
    else
        precision = (u32)format->precision;

    copy = pinta_decimal_ref_get_value(value);
    is_negative = decimal_is_negative(copy);
    if (is_negative == 1)
        copy = decimal_negate(copy);

    switch (format->specifier)
    {
    case PINTA_CHAR('i'):
    case PINTA_CHAR('d'):
        copy = decimal_to_integral_value(copy);
        decimal_to_string(copy, string);
        break;
    case PINTA_CHAR('g'):
    case PINTA_CHAR('G'):
        pinta_format_decimal_general(copy, format->precision, string);
        break;
    case PINTA_CHAR('f'):
        pinta_format_decimal_fixed(copy, format->precision, string);
        break;
    case PINTA_CHAR('e'):
    case PINTA_CHAR('E'):
        pinta_format_decimal_exponential(copy, format->precision, string);
        if (format->precision < 0)
            precision = 6;
        break;
    default:
        PINTA_RETURN();
    }

    end = &string[string_get_length(string)];
    exp = string_find_char(string, PINTA_CHAR('E'));
    dot = string_find_char(string, PINTA_CHAR('.'));

    if (is_negative || format->print_sign)
        sign_length = 1;

    if (dot != NULL)
    {
        dot_length = 1;
        integral_length = (u32)(dot - string);

        if (exp != NULL)
            fractional_length = (u32)(exp - dot - 1);
        else
            fractional_length = (u32)(end - dot - 1);
    }
    else
    {
        if (format->print_decimal)
            dot_length = 1;

        if (exp != NULL)
            integral_length = (u32)(exp - string);
        else
            integral_length = (u32)(end - string);
    }

    if (exp != NULL)
        exponent_length = (u32)(end - exp);

    if (format->specifier != PINTA_CHAR('g') && format->specifier != PINTA_CHAR('G'))
    {
        if (precision > fractional_length)
        {
            dot_length = 1;
            fractional_padding_length = precision - fractional_length;
        }
    }

    string_length = sign_length + integral_length + dot_length + fractional_length + fractional_padding_length + exponent_length;

    if (string_length < width)
        padding_length = width - string_length;

    PINTA_CHECK(pinta_lib_buffer_ensure_write(core, buffer, (string_length + padding_length) * sizeof(wchar)));

    // unsafe to call any of allocation functions until output is in use
    blob = pinta_buffer_ref_get_blob(buffer);
    output = (wchar*)(pinta_blob_get_data(blob) + pinta_buffer_ref_get_position(buffer)); // there should be no alignment issues as this function is called only from format function

    if (format->specifier == PINTA_CHAR('g') || format->specifier == PINTA_CHAR('e'))
    {
        if (exp != NULL)
            *exp = PINTA_CHAR('e');
    }

    if (!format->is_right_padding && padding_length != 0)
    {
        if (sign_length != 0 && format->pad == PINTA_CHAR('0'))
        {
            if (is_negative)
                *output++ = PINTA_CHAR('-');
            else
                *output++ = format->plus;

            sign_length = 0;
        }

        for (index = 0; index < padding_length; index++)
            output[index] = format->pad;
        output += padding_length;
    }

    if (sign_length != 0)
    {
        if (is_negative)
            *output++ = PINTA_CHAR('-');
        else
            *output++ = format->plus;
    }

    if (integral_length != 0)
    {
        memcpy(output, string, integral_length * sizeof(wchar));
        output += integral_length;
    }

    if (dot_length != 0)
    {
        *output++ = PINTA_CHAR('.');
    }

    if (fractional_length != 0)
    {
        memcpy(output, dot + 1, fractional_length * sizeof(wchar));
        output += fractional_length;
    }

    if (fractional_padding_length != 0)
    {
        for (index = 0; index < fractional_padding_length; index++)
            output[index] = PINTA_CHAR('0');
        output += fractional_padding_length;
    }

    if (exponent_length != 0)
    {
        memcpy(output, exp, exponent_length * sizeof(wchar));
        output += exponent_length;
    }

    if (format->is_right_padding && padding_length != 0)
    {
        for (index = 0; index < padding_length; index++)
            output[index] = format->pad;
        output += padding_length;
    }

    pinta_buffer_ref_set_position(buffer, pinta_buffer_ref_get_position(buffer) + (string_length + padding_length) * sizeof(wchar));

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_format_string(PintaCore *core, PintaReference *buffer, PintaReference *value, PintaFormat *format)
{
    PintaException exception = PINTA_OK;
    u32 width = 0;
    u32 padding_length = 0;
    u32 string_length;
    u32 index;
    wchar *output;
    PintaType *type;
    PintaHeapObject *blob; // unsafe to use naked PintaHeapObject* but trust me "I know what I am doing"!

    pinta_assert(core != NULL);
    pinta_assert(buffer != NULL);
    pinta_assert(value != NULL);
    pinta_assert(format != NULL);

    if (value->reference == NULL)
        PINTA_RETURN();

    type = pinta_core_get_type(core, value);
    if (!type->is_string)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    PINTA_CHECK(type->get_length(core, value, &string_length));

    if (format->width <= 0)
        width = 0;
    else
        width = (u32)format->width;

    if (format->precision >= 0 && string_length > (u32)format->precision)
        string_length = (u32)format->precision;

    if (string_length < width)
        padding_length = width - string_length;

    PINTA_CHECK(pinta_lib_buffer_ensure_write(core, buffer, (padding_length + string_length) * sizeof(wchar)));

    if (!format->is_right_padding && padding_length != 0)
    {
        // unsafe to call any of allocation functions until output is in use
        blob = pinta_buffer_ref_get_blob(buffer);
        output = (wchar*)(pinta_blob_get_data(blob) + pinta_buffer_ref_get_position(buffer)); // there should be no alignment issues as this function is called only from format function

        for (index = 0; index < padding_length; index++)
            output[index] = format->pad;
        output += index;

        pinta_buffer_ref_set_position(buffer, pinta_buffer_ref_get_position(buffer) + padding_length * sizeof(wchar));
        PINTA_CHECK(pinta_lib_buffer_write_string(core, buffer, 0, string_length, value));
    }
    else
    {
        if (string_length != 0)
            PINTA_CHECK(pinta_lib_buffer_write_string(core, buffer, 0, string_length, value));

        if (format->is_right_padding && padding_length != 0)
        {
            // unsafe to call any of allocation functions until output is in use
            blob = pinta_buffer_ref_get_blob(buffer);
            output = (wchar*)(pinta_blob_get_data(blob) + pinta_buffer_ref_get_position(buffer)); // there should be no alignment issues as this function is called only from format function

            for (index = 0; index < padding_length; index++)
                output[index] = format->pad;
            output += index;

            pinta_buffer_ref_set_position(buffer, pinta_buffer_ref_get_position(buffer) + padding_length * sizeof(wchar));
        }
    }

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_format(PintaCore *core, PintaReference *buffer, PintaFormatOptions *options, PintaReference *format_string, PintaReference *format_arguments, u32 format_arguments_start)
{
    PintaException exception = PINTA_OK;
    PintaFormat format;

    PintaObjectFunction pinta_convert_decimal = NULL;
    PintaObjectFunction pinta_convert_string = NULL;
    PintaObjectFunction pinta_convert_integer = NULL;

    u32 format_string_length;
    u32 format_arguments_length;
    u32 format_arguments_index;
    u32 length;
    i32 current_position;
    i32 previous_position;
    PintaType *format_string_type;
    struct
    {
        PintaReference item;
        PintaReference item_format;
        PintaReference variable_width;
        PintaReference variable_precision;
    } gc;

    pinta_assert(core != NULL);
    pinta_assert(format_string != NULL);
    pinta_assert(format_arguments != NULL);

    memset(&format, 0, sizeof(format));

    if (format_string->reference == NULL)
        return (PINTA_EXCEPTION_NULL_REFERENCE);

    format_string_type = pinta_core_get_type(core, format_string);
    if (!format_string_type->is_string)
        return (PINTA_EXCEPTION_TYPE_MISMATCH);

    if (format_arguments->reference != NULL && format_arguments->reference->block_kind != PINTA_KIND_ARRAY)
        return (PINTA_EXCEPTION_TYPE_MISMATCH);

    PINTA_GC_ENTER(core, gc);

    if (options != NULL)
    {
        pinta_convert_decimal = options->convert_decimal;
        pinta_convert_string = options->convert_string;
        pinta_convert_integer = options->convert_integer;
    }

    if (pinta_convert_decimal == NULL)
        pinta_convert_decimal = pinta_lib_format_convert_decimal;

    if (pinta_convert_string == NULL)
        pinta_convert_string = pinta_lib_format_convert_string;

    if (pinta_convert_integer == NULL)
        pinta_convert_integer = pinta_lib_format_convert_integer;

    PINTA_CHECK(pinta_lib_string_to_string(core, format_string, format_string));

    format_string_length = pinta_string_ref_get_length(format_string);

    format_arguments_index = format_arguments_start;
    if (format_arguments->reference != NULL)
        format_arguments_length = pinta_array_ref_get_length(format_arguments);
    else
        format_arguments_length = 0;

    current_position = 0;
    do
    {
        previous_position = current_position;

        if (format_arguments_index < format_arguments_length)
            current_position = pinta_string_index_of(format_string, (u32)current_position, PINTA_CHAR('%'));
        else
            current_position = -1;

        if (current_position < 0)
        {
            length = format_string_length - (u32)previous_position;

            if (length > 0)
                PINTA_CHECK(pinta_lib_buffer_write_string(core, buffer, (u32)previous_position, length, format_string));
        }
        else
        {
            length = (u32)(current_position - previous_position);
            if (length > 0)
                PINTA_CHECK(pinta_lib_buffer_write_string(core, buffer, (u32)previous_position, length, format_string));

            pinta_format_parse(format_string, current_position, &format);
            if (format.is_variable_width)
            {
                if (format_arguments->reference != NULL && format_arguments_index < format_arguments_length)
                {
                    PINTA_CHECK(pinta_lib_array_get_item(core, format_arguments, format_arguments_index, &gc.variable_width));
                    format_arguments_index++;

                    PINTA_CHECK(pinta_convert_integer(core, &gc.variable_width, &gc.variable_width));
                    format.width = pinta_integer_ref_get_value(&gc.variable_width);

                    if (format.width < 0)
                    {
                        format.is_right_padding = !format.is_right_padding;
                        format.width = -format.width;
                    }

                    gc.variable_width.reference = NULL;
                }
            }

            if (format.is_variable_precision)
            {
                if (format_arguments->reference != NULL && format_arguments_index < format_arguments_length)
                {
                    PINTA_CHECK(pinta_lib_array_get_item(core, format_arguments, format_arguments_index, &gc.variable_precision));
                    format_arguments_index++;

                    PINTA_CHECK(pinta_convert_integer(core, &gc.variable_precision, &gc.variable_precision));
                    format.precision = pinta_integer_ref_get_value(&gc.variable_precision);

                    if (format.precision < 0)
                        format.precision = -1;

                    gc.variable_precision.reference = NULL;
                }
            }

            if (format.format_end >= 0)
            {
                switch (format.specifier)
                {
                case PINTA_CHAR('i'):
                case PINTA_CHAR('d'):
                case PINTA_CHAR('g'):
                case PINTA_CHAR('G'):
                case PINTA_CHAR('f'):
                case PINTA_CHAR('e'):
                case PINTA_CHAR('E'):

                    if (format_arguments->reference != NULL && format_arguments_index < format_arguments_length)
                    {
                        PINTA_CHECK(pinta_lib_array_get_item(core, format_arguments, format_arguments_index, &gc.item));
                        format_arguments_index++;
                    }
                    else
                    {
                        PINTA_CHECK(pinta_lib_decimal_alloc_zero(core, &gc.item));
                    }

                    PINTA_CHECK(pinta_convert_decimal(core, &gc.item, &gc.item));
                    PINTA_CHECK(pinta_format_decimal(core, buffer, &gc.item, &format));
                    break;

                case PINTA_CHAR('%'):
                    PINTA_CHECK(pinta_lib_buffer_write_string(core, buffer, (u32)current_position, 1, format_string));
                    break;

                case PINTA_CHAR('s'):
                default:

                    if (format_arguments->reference != NULL && format_arguments_index < format_arguments_length)
                    {
                        PINTA_CHECK(pinta_lib_array_get_item(core, format_arguments, format_arguments_index, &gc.item));
                        format_arguments_index++;
                    }
                    else
                    {
                        gc.item.reference = NULL;
                    }

                    PINTA_CHECK(pinta_convert_string(core, &gc.item, &gc.item));
                    PINTA_CHECK(pinta_format_string(core, buffer, &gc.item, &format));
                    break;
                }

                current_position = format.format_end;
            }
            else
            {
                PINTA_CHECK(pinta_lib_buffer_write_string(core, buffer, (u32)current_position, 1, format_string));
                current_position++;
            }
        }

    } while (current_position >= 0);

    PINTA_GC_RETURN(core);
}

PintaException pinta_format_copy(PintaCore *core, PintaReference *buffer, PintaFormatOptions *options, PintaReference *format_string, PintaReference *format_arguments, u32 format_arguments_start)
{
    PintaException exception = PINTA_OK;
    PintaReference inner;

    PINTA_GC_ENTER(core, inner);

    PINTA_CHECK(pinta_lib_buffer_alloc(core, &inner));
    PINTA_CHECK(pinta_format(core, &inner, options, format_string, format_arguments, format_arguments_start));
    PINTA_CHECK(pinta_lib_buffer_write(core, buffer, &inner));

    PINTA_GC_RETURN(core);
}

PintaException pinta_lib_format_convert_decimal(PintaCore *core, PintaReference *value, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    PintaType *type;

    pinta_assert(core != NULL);
    pinta_assert(value != NULL);
    pinta_assert(result != NULL);

    type = pinta_core_get_type(core, value);
    PINTA_CHECK(type->to_decimal(core, value, result));

    if (result->reference == NULL)
    {
        PINTA_CHECK(pinta_lib_decimal_alloc_zero(core, result));
        PINTA_RETURN();
    }

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_format_convert_string(PintaCore *core, PintaReference *value, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    PintaType *type;

    pinta_assert(core != NULL);
    pinta_assert(value != NULL);
    pinta_assert(result != NULL);

    type = pinta_core_get_type(core, value);
    PINTA_CHECK(type->to_string(core, value, result));

    if (result->reference == NULL)
    {
        PINTA_CHECK(pinta_lib_string_alloc(core, 0, result));
        PINTA_RETURN();
    }

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_format_convert_integer(PintaCore *core, PintaReference *value, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    PintaType *type;

    pinta_assert(core != NULL);
    pinta_assert(value != NULL);
    pinta_assert(result != NULL);

    type = pinta_core_get_type(core, value);
    PINTA_CHECK(type->to_integer(core, value, result));

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_format_buffer(PintaCore *core, PintaReference *buffer, PintaFormatOptions *options, PintaReference *format_string, PintaReference *format_arguments, u32 format_arguments_start)
{
    u32 position;

    if (buffer->reference == NULL)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_NULL_REFERENCE);

    position = pinta_buffer_ref_get_position(buffer);
    if ((position & 1) == 1)
        return pinta_format_copy(core, buffer, options, format_string, format_arguments, format_arguments_start);

    return pinta_format(core, buffer, options, format_string, format_arguments, format_arguments_start);
}

PintaException pinta_lib_format(PintaCore *core, PintaFormatOptions *options, PintaReference *format_string, PintaReference *format_arguments, u32 format_arguments_start, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    PintaReference buffer;

    pinta_assert(core != NULL);
    pinta_assert(format_string != NULL);
    pinta_assert(format_arguments != NULL);

    PINTA_GC_ENTER(core, buffer);

    PINTA_CHECK(pinta_lib_buffer_alloc(core, &buffer));
    PINTA_CHECK(pinta_format(core, &buffer, options, format_string, format_arguments, format_arguments_start));
    PINTA_CHECK(pinta_lib_buffer_set_length_position(core, &buffer));
    PINTA_CHECK(pinta_lib_buffer_in_place_to_string(core, &buffer, result));

    PINTA_GC_RETURN(core);
}
