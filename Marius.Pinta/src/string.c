#include "pinta.h"

static const u8 pearson_table[256] = {
    211, 124, 185, 102, 88, 255, 64, 226, 33, 250, 252, 157, 144, 246, 243, 90,
    146, 91, 221, 249, 254, 214, 193, 154, 15, 196, 87, 190, 201, 192, 229, 24,
    84, 230, 40, 11, 109, 75, 70, 83, 220, 162, 232, 37, 132, 25, 92, 18,
    65, 82, 171, 134, 156, 39, 127, 55, 205, 4, 79, 175, 203, 161, 85, 136,
    49, 202, 135, 191, 242, 142, 10, 138, 3, 67, 99, 152, 14, 181, 104, 179,
    172, 52, 73, 186, 106, 166, 21, 100, 59, 93, 223, 119, 103, 183, 50, 16,
    167, 178, 107, 208, 155, 188, 241, 74, 131, 71, 69, 235, 36, 13, 120, 66,
    113, 27, 123, 81, 240, 189, 43, 60, 251, 159, 140, 68, 28, 8, 164, 56,
    149, 233, 253, 89, 45, 101, 160, 110, 199, 248, 187, 245, 96, 76, 128, 46,
    111, 215, 163, 204, 200, 168, 195, 247, 58, 198, 228, 61, 153, 35, 32, 150,
    137, 47, 7, 180, 12, 31, 224, 216, 44, 97, 98, 126, 95, 148, 147, 80,
    22, 34, 94, 38, 30, 238, 6, 5, 212, 117, 197, 29, 236, 115, 225, 112,
    139, 121, 78, 231, 207, 51, 234, 122, 194, 2, 184, 222, 182, 57, 108, 118,
    0, 239, 19, 9, 86, 219, 244, 26, 133, 48, 53, 169, 237, 116, 143, 54,
    114, 42, 165, 23, 177, 210, 158, 209, 20, 151, 173, 170, 129, 213, 218, 174,
    141, 217, 62, 145, 63, 125, 17, 130, 206, 176, 77, 72, 105, 1, 227, 41
};

u8 char_is_whitespace(wchar c)
{
    return ((c >= 0x0009 && c <= 0x000d) || c == 0x0020 ||
        c == 0x1680 || c == 0x180e ||
        (c >= 0x2000 && c <= 0x2006) ||
        (c >= 0x2008 && c <= 0x200a) ||
        c == 0x2028 || c == 0x2029 ||
        c == 0x205f || c == 0x3000);
}

u32 string_get_hashcode(wchar *data, u32 length)
{
    u8 r0, r1, r2, r3;
    u32 index;

    if (length == 0)
        return 0;

    r0 = pearson_table[((u8)*data) + 0];
    r1 = pearson_table[((u8)*data) + 1];
    r2 = pearson_table[((u8)*data) + 2];
    r3 = pearson_table[((u8)*data) + 3];

    for (index = 0; index < length; index++, data++)
    {
        r0 = pearson_table[r0 ^ (u8)*data];
        r1 = pearson_table[r1 ^ (u8)*data];
        r2 = pearson_table[r2 ^ (u8)*data];
        r3 = pearson_table[r3 ^ (u8)*data];

        r0 = pearson_table[r0 ^ (u8)(*data >> 8)];
        r1 = pearson_table[r1 ^ (u8)(*data >> 8)];
        r2 = pearson_table[r2 ^ (u8)(*data >> 8)];
        r3 = pearson_table[r3 ^ (u8)(*data >> 8)];
    }

    return (u32)((r0 << 24) | (r1 << 16) | (r2 << 8) | r3);
}

u32 string_get_length(wchar *data)
{
    wchar *start = data;

    while (*data)
        data++;

    return data - start;
}

u32 string_get_length_c(char *data)
{
    char *start = data;

    while (*data)
        data++;

    return data - start;
}

wchar *string_find_char(wchar *s, wchar c)
{
    wchar *p;

    p = s;
    do
    {
        if (*p == c)
        {
            /* LINTED interface specification */
            return (wchar *)p;
        }
    } while (*p++);

    return NULL;
}

/* STRING accessors */

void pinta_string_set_length(PintaHeapObject *string, u32 length)
{
    pinta_assert(string != NULL);
    pinta_assert(string->block_kind == PINTA_KIND_STRING);

    string->data.string.string_length = length;
}

void pinta_string_set_data(PintaHeapObject *string, wchar *data)
{
    pinta_assert(string != NULL);
    pinta_assert(string->block_kind == PINTA_KIND_STRING);

    string->data.string.string_data = data;
}

void pinta_string_set_item(PintaHeapObject *string, u32 index, wchar value)
{
    pinta_assert(string != NULL);
    pinta_assert(string->block_kind == PINTA_KIND_STRING);
    pinta_assert(index <= string->data.string.string_length); // it is <= instead of == because of the zero char at then end

    string->data.string.string_data[index] = value;
}

u32 pinta_string_get_length(PintaHeapObject *string)
{
    pinta_assert(string != NULL);
    pinta_assert(string->block_kind == PINTA_KIND_STRING);

    return string->data.string.string_length;
}

wchar *pinta_string_get_data(PintaHeapObject *string)
{
    pinta_assert(string != NULL);
    pinta_assert(string->block_kind == PINTA_KIND_STRING);

    return string->data.string.string_data;
}

wchar pinta_string_get_item(PintaHeapObject *string, u32 index)
{
    pinta_assert(string != NULL);
    pinta_assert(string->block_kind == PINTA_KIND_STRING);
    pinta_assert(index <= string->data.string.string_length); // it is <= instead of == because of the zero char at then end

    return string->data.string.string_data[index];
}

wchar *pinta_string_ref_get_data(PintaReference *reference)
{
    pinta_assert(reference != NULL);
    pinta_assert(reference->reference != NULL);

    return pinta_string_get_data(reference->reference);
}

u32 pinta_string_ref_get_length(PintaReference *reference)
{
    pinta_assert(reference != NULL);
    pinta_assert(reference->reference != NULL);

    return pinta_string_get_length(reference->reference);
}

wchar pinta_string_ref_get_item(PintaReference *reference, u32 index)
{
    pinta_assert(reference != NULL);
    pinta_assert(reference->reference != NULL);

    return pinta_string_get_item(reference->reference, index);
}

void pinta_string_ref_set_length(PintaReference *reference, u32 string_length)
{
    pinta_assert(reference != NULL);

    pinta_string_set_length(reference->reference, string_length);
}

void pinta_string_ref_set_data(PintaReference *reference, wchar *string_data)
{
    pinta_assert(reference != NULL);

    pinta_string_set_data(reference->reference, string_data);
}

i32 pinta_string_index_of(PintaReference *reference, u32 start, wchar value)
{
    wchar *data;
    u32 index;
    u32 length;

    pinta_assert(reference != NULL);
    pinta_assert(reference->reference != NULL);
    pinta_assert(reference->reference->block_kind == PINTA_KIND_STRING);

    data = pinta_string_ref_get_data(reference);
    length = pinta_string_ref_get_length(reference);

    for (index = start; index < length; index++)
    {
        if (data[index] == value)
            return (i32)index;
    }

    return -1;
}

/* SUBSTRING accessors */

PintaHeapObject *pinta_substring_get_value(PintaHeapObject *object)
{
    pinta_assert(object != NULL);
    pinta_assert(object->block_kind == PINTA_KIND_SUBSTRING);

    return object->data.substring.string_reference;
}

u16 pinta_substring_get_offset(PintaHeapObject *object)
{
    pinta_assert(object != NULL);
    pinta_assert(object->block_kind == PINTA_KIND_SUBSTRING);

    return object->data.substring.string_offset;
}

u16 pinta_substring_get_length(PintaHeapObject *object)
{
    pinta_assert(object != NULL);
    pinta_assert(object->block_kind == PINTA_KIND_SUBSTRING);

    return object->data.substring.string_length;
}

void pinta_substring_set_value(PintaHeapObject *object, PintaHeapObject *value)
{
    pinta_assert(object != NULL);
    pinta_assert(object->block_kind == PINTA_KIND_SUBSTRING);

    object->data.substring.string_reference = value;
}

void pinta_substring_set_offset(PintaHeapObject *object, u16 value)
{
    pinta_assert(object != NULL);
    pinta_assert(object->block_kind == PINTA_KIND_SUBSTRING);

    object->data.substring.string_offset = value;
}

void pinta_substring_set_length(PintaHeapObject *object, u16 value)
{
    pinta_assert(object != NULL);
    pinta_assert(object->block_kind == PINTA_KIND_SUBSTRING);

    object->data.substring.string_length = value;
}

PintaHeapObject *pinta_substring_ref_get_value(PintaReference *object)
{
    pinta_assert(object != NULL);

    return pinta_substring_get_value(object->reference);
}

u16 pinta_substring_ref_get_offset(PintaReference *object)
{
    pinta_assert(object != NULL);

    return pinta_substring_get_offset(object->reference);
}

u16 pinta_substring_ref_get_length(PintaReference *object)
{
    pinta_assert(object != NULL);

    return pinta_substring_get_length(object->reference);
}

void pinta_substring_ref_set_value(PintaReference *object, PintaReference *value)
{
    pinta_assert(object != NULL);
    pinta_assert(value != NULL);

    pinta_substring_set_value(object->reference, value->reference);
}

void pinta_substring_ref_set_offset(PintaReference *object, u16 value)
{
    pinta_assert(object != NULL);

    pinta_substring_set_offset(object->reference, value);
}

void pinta_substring_ref_set_length(PintaReference *object, u16 value)
{
    pinta_assert(object != NULL);

    pinta_substring_set_length(object->reference, value);
}

/* MULTISTRING accessors */

PintaHeapObject **pinta_multistring_get_value(PintaHeapObject *object)
{
    pinta_assert(object != NULL);
    pinta_assert(object->block_kind == PINTA_KIND_MULTISTRING);

    return object->data.multistring.parts;
}

PintaHeapObject *pinta_multistring_get_item(PintaHeapObject *object, u32 index)
{
    pinta_assert(object != NULL);
    pinta_assert(object->block_kind == PINTA_KIND_MULTISTRING);
    pinta_assert(index < PINTA_MULTISTRING_COUNT);
    pinta_assert(object->data.multistring.parts != NULL);

    return object->data.multistring.parts[index];
}

u32 pinta_multistring_get_length(PintaHeapObject *object)
{
    PintaHeapObject *item;
    u32 index = 0;
    u32 result = 0;

    pinta_assert(object != NULL);
    pinta_assert(object->block_kind == PINTA_KIND_MULTISTRING);

    for (; index < PINTA_MULTISTRING_COUNT; index++)
    {
        item = pinta_multistring_get_item(object, index);
        if (item == NULL)
            continue;

        pinta_assert(item->block_kind == PINTA_KIND_STRING || item->block_kind == PINTA_KIND_SUBSTRING || item->block_kind == PINTA_KIND_CHAR);

        if (item->block_kind == PINTA_KIND_STRING)
            result += pinta_string_get_length(item);
        else if (item->block_kind == PINTA_KIND_SUBSTRING)
            result += pinta_substring_get_length(item);
        else if (item->block_kind == PINTA_KIND_CHAR)
            result += pinta_char_get_length(item);
    }

    return result;
}

void pinta_multistring_set_value(PintaHeapObject *object, PintaHeapObject **value)
{
    pinta_assert(object != NULL);
    pinta_assert(object->block_kind == PINTA_KIND_MULTISTRING);
    pinta_assert(value != NULL);

    object->data.multistring.parts = value;
}

void pinta_multistring_set_item(PintaHeapObject *object, u32 index, PintaHeapObject *value)
{
    pinta_assert(object != NULL);
    pinta_assert(object->block_kind == PINTA_KIND_MULTISTRING);
    pinta_assert(index < PINTA_MULTISTRING_COUNT);
    pinta_assert(object->data.multistring.parts != NULL);

    object->data.multistring.parts[index] = value;
}

PintaHeapObject **pinta_multistring_ref_get_value(PintaReference *object)
{
    pinta_assert(object != NULL);

    return pinta_multistring_get_value(object->reference);
}

PintaHeapObject *pinta_multistring_ref_get_item(PintaReference *object, u32 index)
{
    pinta_assert(object != NULL);

    return pinta_multistring_get_item(object->reference, index);
}

u32 pinta_multistring_ref_get_length(PintaReference *object)
{
    pinta_assert(object != NULL);

    return pinta_multistring_get_length(object->reference);
}

void pinta_multistring_ref_set_value(PintaReference *object, PintaHeapObject **value)
{
    pinta_assert(object != NULL);

    pinta_multistring_set_value(object->reference, value);
}

void pinta_multistring_ref_set_item(PintaReference *object, u32 index, PintaReference *value)
{
    pinta_assert(object != NULL);
    pinta_assert(value != NULL);

    pinta_multistring_set_item(object->reference, index, value->reference);
}

void pinta_multistring_ref_set_null(PintaReference *object, u32 index)
{
    pinta_assert(object != NULL);

    pinta_multistring_set_item(object->reference, index, NULL);
}

void pinta_multistring_ref_set_item_object(PintaReference *object, u32 index, PintaHeapObject *value)
{
    pinta_assert(object != NULL);

    pinta_multistring_set_item(object->reference, index, value);
}

/* CHAR accessors */

wchar pinta_char_get_value(PintaHeapObject *object)
{
    pinta_assert(object != NULL);
    pinta_assert(object->block_kind == PINTA_KIND_CHAR);

    return object->data.character.character_value;
}

void pinta_char_set_value(PintaHeapObject *object, wchar value)
{
    pinta_assert(object != NULL);
    pinta_assert(object->block_kind == PINTA_KIND_CHAR);

    object->data.character.character_value = value;
}

u32 pinta_char_get_length(PintaHeapObject *object)
{
    pinta_assert(object != NULL);
    pinta_assert(object->block_kind == PINTA_KIND_CHAR);

    return 1;
}

wchar pinta_char_ref_get_value(PintaReference *reference)
{
    pinta_assert(reference != NULL);

    return pinta_char_get_value(reference->reference);
}

void pinta_char_ref_set_value(PintaReference *reference, wchar value)
{
    pinta_assert(reference != NULL);

    pinta_char_set_value(reference->reference, value);
}

u32 pinta_char_ref_get_length(PintaReference *reference)
{
    pinta_assert(reference != NULL);

    return pinta_char_get_length(reference->reference);
}

/* STRING */

void pinta_string_init_type(PintaType *type)
{
    type->is_string = 1;

    type->gc_relocate = pinta_string_relocate;

    type->to_integer = pinta_lib_string_to_integer;
    type->to_integer_value = pinta_lib_string_to_integer_value;
    type->to_string = pinta_core_default_copy;
    type->to_decimal = pinta_lib_string_to_decimal;
    type->to_numeric = pinta_lib_string_to_decimal;
    type->to_bool = pinta_lib_string_to_bool;

    type->get_member = pinta_lib_string_get_member;
    type->get_item = pinta_lib_string_get_item;

    type->get_length = pinta_lib_string_get_length;
    type->get_char = pinta_lib_string_get_char;

#if PINTA_DEBUG
    type->debug_write = pinta_lib_string_debug_write;
#endif
}

void pinta_string_relocate(PintaHeapObject *object, PintaHeapReloc *reloc, u32 count)
{
    wchar *data;
    PintaHeapReloc *entry;

    pinta_gc_relocate_validate(PINTA_KIND_STRING, object, reloc, count);

    data = pinta_string_get_data(object);
    if (data == NULL)
        return;

    entry = pinta_gc_relocate_find(data, reloc, count);
    if (entry != NULL)
        pinta_string_set_data(object, data - (sizeof(PintaHeapObject) * entry->offset) / sizeof(wchar));
}

PintaHeapObject *pinta_string_alloc_object(PintaCore *core, u32 length)
{
    u32 size;
    PintaHeapObject *result;

    pinta_assert(core != NULL);

    if (length == 0 && core->cache != NULL)
        return &core->cache->string_empty[0];

    size = (sizeof(PintaHeapObject) + sizeof(wchar)* length + 1 + sizeof(PintaHeapObject) - 1) / sizeof(PintaHeapObject);
    result = pinta_core_alloc(core, PINTA_KIND_STRING, PINTA_FLAG_NONE, size);
    if (result == NULL)
        return NULL;

    pinta_string_set_length(result, length);
    pinta_string_set_data(result, (wchar*)&result[1]);
    pinta_string_set_item(result, length, PINTA_CHAR('\0'));

    return result;
}

PintaHeapObject *pinta_string_alloc_object_value(PintaCore *core, wchar *data, u32 length)
{
    PintaHeapObject *result;

    pinta_assert(core != NULL);
    pinta_assert(data != NULL);

#if PINTA_DEBUG
    {
        PintaHeap *heap;
        for (heap = core->heap; heap != NULL; heap = heap->next)
        {
            pinta_assert((data < (wchar*)heap->start && (data + length) < (wchar*)heap->start) || (data > (wchar*)heap->end && (data + length) > (wchar*)heap->end));
        }
    }
#endif

    result = pinta_core_alloc(core, PINTA_KIND_STRING, PINTA_FLAG_NONE, 1);

    pinta_string_set_length(result, length);
    pinta_string_set_data(result, data);

    return result;
}

PintaHeapObject *pinta_string_alloc_object_copy(PintaCore *core, wchar *data, u32 length)
{
    u32 size;
    wchar *string_data;
    PintaHeapObject *result;

    pinta_assert(core != NULL);

    if (length == 0 && core->cache != NULL)
        return &core->cache->string_empty[0];

    if (length == 1)
        return pinta_char_alloc_object_value(core, data[0]);

    size = (sizeof(PintaHeapObject) + sizeof(wchar)* length + 1 + sizeof(PintaHeapObject) - 1) / sizeof(PintaHeapObject);
    result = pinta_core_alloc(core, PINTA_KIND_STRING, PINTA_FLAG_NONE, size);
    if (result == NULL)
        return NULL;

    pinta_string_set_length(result, length);
    pinta_string_set_data(result, (wchar*)&result[1]);
    pinta_string_set_item(result, length, PINTA_CHAR('\0'));

    string_data = pinta_string_get_data(result);
    memmove(string_data, data, sizeof(wchar) * length);

    return result;
}

i32 pinta_string_compare(wchar *left, wchar *right, u32 length)
{
    for (; length && *left == *right; length--, left++, right++);
    return length ? *left - *right : 0;
}

u8 pinta_string_equals(PintaHeapObject *left, PintaHeapObject *right)
{
    u32 left_length;
    u32 right_length;
    i32 compare_result;

    pinta_assert(left != NULL);
    pinta_assert(right != NULL);

    if (left == right)
        return 1;

    if (left == NULL || right == NULL)
        return 0;

    if (left->block_kind != PINTA_KIND_STRING || right->block_kind != PINTA_KIND_STRING)
        return 0;

    left_length = pinta_string_get_length(left);
    right_length = pinta_string_get_length(right);

    if (left_length != right_length)
        return 0;

    compare_result = pinta_string_compare(pinta_string_get_data(left), pinta_string_get_data(right), left_length);
    if (compare_result == 0)
        return 1;

    return 0;
}

PintaException pinta_string_get_char(PintaHeapObject *string, u32 index, wchar *result)
{
    PintaException exception = PINTA_OK;

    pinta_assert(result != NULL);

    if (string == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (string->block_kind != PINTA_KIND_STRING)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    if (index >= pinta_string_get_length(string))
        PINTA_THROW(PINTA_EXCEPTION_INDEX_OUT_OF_BOUNDS);

    *result = pinta_string_get_item(string, index);

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_string_alloc(PintaCore *core, u32 length, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    PintaHeapObject *value;

    pinta_assert(core != NULL);
    pinta_assert(result != NULL);

    value = pinta_string_alloc_object(core, length);
    if (value == NULL)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_MEMORY);

    result->reference = value;

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_string_alloc_value(PintaCore *core, wchar *data, u32 length, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    PintaHeapObject *value;

    pinta_assert(core != NULL);
    pinta_assert(result != NULL);

    value = pinta_string_alloc_object_value(core, data, length);
    if (value == NULL)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_MEMORY);

    result->reference = value;

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_string_alloc_copy(PintaCore *core, wchar *data, u32 length, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    PintaHeapObject *value;

    pinta_assert(core != NULL);
    pinta_assert(result != NULL);

    value = pinta_string_alloc_object_copy(core, data, length);
    if (value == NULL)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_MEMORY);

    result->reference = value;

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_string_to_string(PintaCore *core, PintaReference *value, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    u8 kind;

    pinta_assert(core != NULL);
    pinta_assert(value != NULL);
    pinta_assert(result != NULL);

    if (value->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    kind = value->reference->block_kind;
    if (kind == PINTA_KIND_STRING)  // nothing to do
    {
        result->reference = value->reference;
        PINTA_RETURN();
    }

    if (kind == PINTA_KIND_SUBSTRING)
        PINTA_CHECK(pinta_lib_substring_to_string(core, value, result));
    else if (kind == PINTA_KIND_MULTISTRING)
        PINTA_CHECK(pinta_lib_multistring_to_string(core, value, result));
    else if (kind == PINTA_KIND_CHAR)
        PINTA_CHECK(pinta_lib_char_to_string(core, value, result));
    else
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_string_get_length(PintaCore *core, PintaReference *string, u32 *result)
{
    PintaException exception = PINTA_OK;

    pinta_assert(core != NULL);
    pinta_assert(string != NULL);
    pinta_assert(result != NULL);

    if (string->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (string->reference->block_kind != PINTA_KIND_STRING)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    *result = pinta_string_ref_get_length(string);

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_string_get_charcode(PintaCore *core, PintaReference *string, u32 index, PintaReference *result)
{
    PintaException exception = PINTA_OK;

    pinta_assert(string != NULL);
    pinta_assert(result != NULL);

    if (string->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    pinta_assert(string->reference->block_kind == PINTA_KIND_STRING);

    if (index >= pinta_string_ref_get_length(string))
        PINTA_THROW(PINTA_EXCEPTION_INDEX_OUT_OF_BOUNDS);

    PINTA_CHECK(pinta_lib_integer_alloc_value(core, (i32)pinta_string_ref_get_item(string, index), result));

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_string_get_char(PintaCore *core, PintaReference *string, u32 index, wchar *result)
{
    PintaException exception = PINTA_OK;

    PINTA_UNUSED(core);

    pinta_assert(string != NULL);
    pinta_assert(result != NULL);

    if (string->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    pinta_assert(string->reference->block_kind == PINTA_KIND_STRING);

    if (index >= pinta_string_ref_get_length(string))
        PINTA_THROW(PINTA_EXCEPTION_INDEX_OUT_OF_BOUNDS);

    *result = pinta_string_ref_get_item(string, index);

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_string_get_member(PintaCore *core, PintaReference *string, PintaReference *name, u8 *is_accessor, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    i32 value;

    if (name->reference != NULL && name->reference->block_kind == PINTA_KIND_INTEGER)
    {
        value = pinta_integer_ref_get_value(name);
        if (value >= 0)
        {
            if (is_accessor != NULL)
                *is_accessor = 0;
            PINTA_CHECK(pinta_lib_string_get_item(core, string, (u32)value, result));
            PINTA_RETURN();
        }
    }

    PINTA_THROW(PINTA_EXCEPTION_NOT_IMPLEMENTED);

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_string_get_item(PintaCore *core, PintaReference *string, u32 index, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    wchar character;

    pinta_assert(string != NULL);
    pinta_assert(result != NULL);

    PINTA_CHECK(pinta_lib_string_get_char(core, string, index, &character));
    PINTA_CHECK(pinta_lib_char_alloc_value(core, character, result));

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_string_to_integer(PintaCore *core, PintaReference *string, PintaReference *result)
{
    return pinta_lib_integer_try_parse(core, string, NULL, result);
}

PintaException pinta_lib_string_to_integer_value(PintaCore *core, PintaReference *string, i32 *result)
{
    return pinta_lib_integer_try_parse_value(core, string, NULL, result);
}

PintaException pinta_lib_string_to_decimal(PintaCore *core, PintaReference *string, PintaReference *result)
{
    return pinta_lib_decimal_try_parse(core, string, NULL, result);
}

PintaException pinta_lib_string_to_bool(PintaCore *core, PintaReference *string, u8 *result)
{
    PINTA_UNUSED(core);

    if (pinta_string_ref_get_length(string) == 0)
        *result = 0;
    else
        *result = 1;

    return PINTA_OK;
}

PintaException pinta_lib_string_get_hashcode(PintaCore *core, PintaReference *string, u32 *result)
{
    PintaException exception = PINTA_OK;
    wchar *data;
    u32 length;

    pinta_assert(core != NULL);
    pinta_assert(string != NULL);
    pinta_assert(result != NULL);

    if (string->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    PINTA_CHECK(pinta_lib_string_to_string(core, string, string));

    data = pinta_string_ref_get_data(string);
    length = pinta_string_ref_get_length(string);

    *result = string_get_hashcode(data, length);

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_string_debug_write(PintaCore *core, PintaReference *value, u32 max_depth, PintaJsonWriter *writer)
{
    PintaException exception = PINTA_OK;
    wchar *data;
    u32 length;

    PINTA_UNUSED(max_depth);

    pinta_assert(core != NULL);
    pinta_assert(value != NULL);

    if (value->reference == NULL)
        return pinta_json_write_null(core, writer);

    pinta_core_pin(value->reference);

    data = pinta_string_ref_get_data(value);
    length = pinta_string_ref_get_length(value);

    PINTA_CHECK(pinta_json_write_string(core, writer, data, length));

PINTA_EXIT:
    pinta_core_unpin(value->reference);
    return exception;
}

/* SUBSTRING */

void pinta_substring_init_type(PintaType *type)
{
    type->is_string = 1;

    type->gc_walk = pinta_substring_walk;
    type->gc_relocate = pinta_substring_relocate;

    type->to_integer = pinta_lib_string_to_integer;
    type->to_integer_value = pinta_lib_string_to_integer_value;
    type->to_string = pinta_core_default_copy;
    type->to_decimal = pinta_lib_string_to_decimal;
    type->to_numeric = pinta_lib_string_to_decimal;
    type->to_bool = pinta_lib_substring_to_bool;

    type->get_member = pinta_lib_substring_get_member;
    type->get_item = pinta_lib_substring_get_item;

    type->get_length = pinta_lib_substring_get_length;
    type->get_char = pinta_lib_substring_get_char;

#if PINTA_DEBUG
    type->debug_write = pinta_lib_substring_debug_write;
#endif
}

u32 pinta_substring_walk(PintaHeapObject *object, PintaHeapObjectWalkerState *state, PintaHeapObject **result)
{
    pinta_assert(object != NULL);
    pinta_assert(state != NULL);
    pinta_assert(result != NULL);
    pinta_assert(object->block_kind == PINTA_KIND_SUBSTRING);

    *result = NULL;

    if (state->field != 0)
        return 0;

    *result = pinta_substring_get_value(object);
    state->field = 1;

    return 1;
}

void pinta_substring_relocate(PintaHeapObject *object, PintaHeapReloc *reloc, u32 count)
{
    PintaHeapObject *data;
    PintaHeapReloc *entry;

    pinta_gc_relocate_validate(PINTA_KIND_SUBSTRING, object, reloc, count);

    data = pinta_substring_get_value(object);
    if (data == NULL)
        return;

    entry = pinta_gc_relocate_find(data, reloc, count);
    if (entry != NULL)
        pinta_substring_set_value(object, data - entry->offset);
}

PintaHeapObject *pinta_substring_alloc_object(PintaCore *core, PintaReference *string, u16 offset, u16 length)
{
    PintaHeapObject *result;

    pinta_assert(core != NULL);
    pinta_assert(string != NULL);
    pinta_assert(string->reference != NULL);
    pinta_assert(string->reference->block_kind == PINTA_KIND_STRING);
    pinta_assert(offset < pinta_string_ref_get_length(string));
    pinta_assert((u32)offset + (u32)length <= pinta_string_ref_get_length(string));

    result = pinta_core_alloc(core, PINTA_KIND_SUBSTRING, PINTA_FLAG_NONE, 1);
    if (result == NULL)
        return NULL;

    pinta_substring_set_value(result, string->reference);
    pinta_substring_set_offset(result, offset);
    pinta_substring_set_length(result, length);

    return result;
}

PintaException pinta_lib_substring_alloc(PintaCore *core, PintaReference *string, u16 offset, u16 length, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    PintaHeapObject *value;

    pinta_assert(core != NULL);
    pinta_assert(string != NULL);
    pinta_assert(result != NULL);

    if (string->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (string->reference->block_kind != PINTA_KIND_STRING)
        PINTA_THROW(PINTA_EXCEPTION_INVALID_OPERATION);

    if (offset >= pinta_string_ref_get_length(string))
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

    if ((u32)offset + (u32)length > pinta_string_ref_get_length(string))
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

    value = pinta_substring_alloc_object(core, string, offset, length);
    if (value == NULL)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_MEMORY);

    result->reference = value;

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_substring_to_string(PintaCore *core, PintaReference *value, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    u32 length, offset;
    u32 source_length;
    PintaHeapObject *source_string;

    wchar *source, *destination;

    PintaHeapObject *string;

    pinta_assert(core != NULL);
    pinta_assert(value != NULL);

    if (value->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    pinta_assert(value->reference->block_kind == PINTA_KIND_SUBSTRING);

    if (pinta_substring_ref_get_value(value) == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    length = pinta_substring_ref_get_length(value);
    offset = pinta_substring_ref_get_offset(value);

    string = pinta_string_alloc_object(core, length);
    if (string == NULL)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_MEMORY);

    source_string = pinta_substring_ref_get_value(value);
    source_length = pinta_string_get_length(source_string);

#if PINTA_DEBUG
    pinta_assert(offset < source_length);
    pinta_assert(offset + length <= source_length);
#else
    if (offset >= source_length)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);    // assertion? (because alloc should not have allowed such attrocity)

    if (offset + length > source_length)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);    // assertion? (because alloc should not have allowed such attrocity)
#endif

    source = pinta_string_get_data(source_string);
    source = &source[offset];

    destination = pinta_string_get_data(string);

    memcpy(destination, source, length * sizeof(wchar));
    pinta_string_set_item(string, length, PINTA_CHAR('\0'));

    result->reference = string;

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_substring_get_length(PintaCore *core, PintaReference *string, u32 *result)
{
    PintaException exception = PINTA_OK;

    pinta_assert(core != NULL);
    pinta_assert(string != NULL);
    pinta_assert(result != NULL);

    if (string->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (string->reference->block_kind != PINTA_KIND_SUBSTRING)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    *result = pinta_substring_ref_get_length(string);

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_substring_get_charcode(PintaCore *core, PintaReference *string, u32 index, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    u32 length, offset;

    PintaReference source;

    pinta_assert(core != NULL);
    pinta_assert(string != NULL);
    pinta_assert(result != NULL);

    if (string->reference == NULL)
        return (PINTA_EXCEPTION_NULL_REFERENCE);

    if (string->reference->block_kind != PINTA_KIND_SUBSTRING)
        return (PINTA_EXCEPTION_TYPE_MISMATCH);

    if (pinta_substring_ref_get_value(string) == NULL)
        return (PINTA_EXCEPTION_NULL_REFERENCE);

    length = pinta_substring_ref_get_length(string);
    offset = pinta_substring_ref_get_offset(string);

    if (index >= length)
        return (PINTA_EXCEPTION_INDEX_OUT_OF_BOUNDS);

    PINTA_GC_ENTER(core, source);

    source.reference = pinta_substring_ref_get_value(string);
    PINTA_CHECK(pinta_lib_string_get_item(core, &source, index + offset, result));

    PINTA_GC_RETURN(core);
}

PintaException pinta_lib_substring_get_char(PintaCore *core, PintaReference *string, u32 index, wchar *result)
{
    PintaException exception = PINTA_OK;
    u32 length, offset;
    PintaHeapObject *source;

    pinta_assert(core != NULL);
    pinta_assert(string != NULL);
    pinta_assert(result != NULL);

    if (string->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (string->reference->block_kind != PINTA_KIND_SUBSTRING)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    if (pinta_substring_ref_get_value(string) == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    length = pinta_substring_ref_get_length(string);
    offset = pinta_substring_ref_get_offset(string);

    if (index >= length)
        PINTA_THROW(PINTA_EXCEPTION_INDEX_OUT_OF_BOUNDS);

    source = pinta_substring_ref_get_value(string);
    PINTA_CHECK(pinta_string_get_char(source, index + offset, result));

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_substring_get_member(PintaCore *core, PintaReference *string, PintaReference *name, u8 *is_accessor, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    i32 value;

    if (name->reference != NULL && name->reference->block_kind == PINTA_KIND_INTEGER)
    {
        value = pinta_integer_ref_get_value(name);
        if (value >= 0)
        {
            if (is_accessor != NULL)
                *is_accessor = 0;
            PINTA_CHECK(pinta_lib_substring_get_item(core, string, (u32)value, result));
            PINTA_RETURN();
        }
    }

    PINTA_THROW(PINTA_EXCEPTION_NOT_IMPLEMENTED);

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_substring_get_item(PintaCore *core, PintaReference *string, u32 index, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    wchar character;

    pinta_assert(core != NULL);
    pinta_assert(string != NULL);
    pinta_assert(result != NULL);

    PINTA_CHECK(pinta_lib_substring_get_char(core, string, index, &character));
    PINTA_CHECK(pinta_lib_char_alloc_value(core, character, result));

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_substring_to_bool(PintaCore *core, PintaReference *string, u8 *result)
{
    PINTA_UNUSED(core);

    if (pinta_substring_ref_get_length(string) == 0)
        *result = 0;
    else
        *result = 1;

    return PINTA_OK;
}

PintaException pinta_lib_substring_debug_write(PintaCore *core, PintaReference *value, u32 max_depth, PintaJsonWriter *writer)
{
    PintaException exception = PINTA_OK;
    PintaHeapObject *string;
    wchar *data;
    u32 length;
    u32 offset;

    PINTA_UNUSED(max_depth);

    pinta_assert(core != NULL);
    pinta_assert(value != NULL);

    if (value->reference == NULL)
        return pinta_json_write_null(core, writer);

    string = pinta_substring_ref_get_value(value);
    pinta_core_pin(string);

    data = pinta_string_get_data(string);
    offset = pinta_substring_ref_get_offset(value);
    length = pinta_substring_ref_get_length(value);

    PINTA_CHECK(pinta_json_write_string(core, writer, &data[offset], length));
PINTA_EXIT:
    pinta_core_unpin(string);
    return exception;
}

/* MULTISTRING */

void pinta_multistring_init_type(PintaType *type)
{
    type->is_string = 1;

    type->gc_walk = pinta_multistring_walk;
    type->gc_relocate = pinta_multistring_relocate;

    type->to_integer = pinta_lib_string_to_integer;
    type->to_integer_value = pinta_lib_string_to_integer_value;
    type->to_string = pinta_core_default_copy;
    type->to_decimal = pinta_lib_string_to_decimal;
    type->to_numeric = pinta_lib_string_to_decimal;
    type->to_bool = pinta_lib_multistring_to_bool;

    type->get_member = pinta_lib_multistring_get_member;
    type->get_item = pinta_lib_multistring_get_item;

    type->get_length = pinta_lib_multistring_get_length;
    type->get_char = pinta_lib_multistring_get_char;

#if PINTA_DEBUG
    type->debug_write = pinta_lib_multistring_debug_write;
#endif
}

u32 pinta_multistring_walk(PintaHeapObject *object, PintaHeapObjectWalkerState *state, PintaHeapObject **result)
{
    pinta_assert(object != NULL);
    pinta_assert(state != NULL);
    pinta_assert(result != NULL);
    pinta_assert(object->block_kind == PINTA_KIND_MULTISTRING);

    *result = NULL;

    if (state->index >= PINTA_MULTISTRING_COUNT)
        return 0;

    *result = pinta_multistring_get_item(object, state->index);
    state->index = state->index + 1;

    return 1;
}

void pinta_multistring_relocate(PintaHeapObject *object, PintaHeapReloc *reloc, u32 count)
{
    PintaHeapObject **data;
    PintaHeapReloc *entry;
    u32 index;

    pinta_gc_relocate_validate(PINTA_KIND_MULTISTRING, object, reloc, count);

    data = pinta_multistring_get_value(object);
    if (data == NULL)
        return;

    entry = pinta_gc_relocate_find(data, reloc, count);
    if (entry != NULL)
        pinta_multistring_set_value(object, (PintaHeapObject**)((PintaHeapObject*)data - entry->offset));

    for (index = 0; index < PINTA_MULTISTRING_COUNT; index++)
    {
        PintaHeapObject *item = pinta_multistring_get_item(object, index);
        if (item == NULL)
            continue;

        entry = pinta_gc_relocate_find(item, reloc, count);
        if (entry != NULL)
            pinta_multistring_set_item(object, index, item - entry->offset);
    }
}

PintaHeapObject *pinta_multistring_alloc_object(PintaCore *core)
{
    u32 index;
    PintaHeapObject *result;

    pinta_assert(core != NULL);
    result = pinta_core_alloc(core, PINTA_KIND_MULTISTRING, PINTA_FLAG_NONE, 2);
    if (result == NULL)
        return NULL;

    pinta_multistring_set_value(result, (PintaHeapObject**)(result + 1));
    for (index = 0; index < PINTA_MULTISTRING_COUNT; index++)
        pinta_multistring_set_item(result, index, NULL);

    return result;
}

PintaHeapObject *pinta_multistring_alloc_object_value(PintaCore *core, PintaReference *value)
{
    u32 index;
    u32 empty_index;
    PintaHeapObject *result;
    PintaHeapObject *valueObject = NULL;

    pinta_assert(core != NULL);
    pinta_assert(value == NULL || value->reference == NULL || (value->reference->block_kind == PINTA_KIND_STRING || value->reference->block_kind == PINTA_KIND_SUBSTRING || value->reference->block_kind == PINTA_KIND_MULTISTRING || value->reference->block_kind == PINTA_KIND_CHAR));

    result = pinta_core_alloc(core, PINTA_KIND_MULTISTRING, PINTA_FLAG_NONE, 2);
    if (result == NULL)
        return NULL;

    pinta_multistring_set_value(result, (PintaHeapObject**)(result + 1));

    if (value != NULL && value->reference != NULL && value->reference->block_kind == PINTA_KIND_MULTISTRING)
    {
        empty_index = 0;
        for (index = 0; index < PINTA_MULTISTRING_COUNT; index++)
        {
            if (pinta_multistring_ref_get_item(value, index) != NULL)
                pinta_multistring_set_item(result, empty_index++, pinta_multistring_ref_get_item(value, index));
        }

        for (; empty_index < PINTA_MULTISTRING_COUNT; empty_index++)
            pinta_multistring_set_item(result, empty_index, NULL);
    }
    else
    {
        if (value != NULL)
            valueObject = value->reference;

        pinta_multistring_set_item(result, 0, valueObject);
        for (index = 1; index < PINTA_MULTISTRING_COUNT; index++)
            pinta_multistring_set_item(result, index, NULL);
    }

    return result;
}

PintaException pinta_lib_multistring_alloc(PintaCore *core, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    PintaHeapObject *value;

    pinta_assert(core != NULL);
    pinta_assert(result != NULL);

    value = pinta_multistring_alloc_object(core);
    if (value == NULL)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_MEMORY);

    result->reference = value;

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_multistring_alloc_value(PintaCore *core, PintaReference *value, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    PintaHeapObject *string;

    pinta_assert(core != NULL);
    pinta_assert(value != NULL);
    pinta_assert(result != NULL);

    if (value->reference != NULL)
        if (value->reference->block_kind != PINTA_KIND_STRING && value->reference->block_kind != PINTA_KIND_MULTISTRING && value->reference->block_kind != PINTA_KIND_SUBSTRING && value->reference->block_kind != PINTA_KIND_CHAR)
            PINTA_THROW(PINTA_EXCEPTION_INVALID_OPERATION);

    string = pinta_multistring_alloc_object_value(core, value);
    if (string == NULL)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_MEMORY);

    result->reference = string;

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_multistring_to_string(PintaCore *core, PintaReference *value, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    u32 index;
    u32 length;
    PintaHeapObject **data;
    wchar *source, *destination;

    struct
    {
        PintaReference item;
        PintaReference string;
    } gc;

    pinta_assert(core != NULL);
    pinta_assert(value != NULL);

    PINTA_GC_ENTER(core, gc);

    if (value->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    pinta_assert(value->reference->block_kind == PINTA_KIND_MULTISTRING);

    data = pinta_multistring_ref_get_value(value);
    if (data == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE); // assertion?

    length = 0;
    for (index = 0; index < PINTA_MULTISTRING_COUNT; index++)
    {
        u8 kind;
        gc.item.reference = pinta_multistring_ref_get_item(value, index);
        if (gc.item.reference == NULL)
            continue;

        // part can be substring or string ONLY
        kind = gc.item.reference->block_kind;
        if (kind == PINTA_KIND_STRING)
            length += pinta_string_ref_get_length(&gc.item);
        else if (kind == PINTA_KIND_SUBSTRING)
            length += pinta_substring_ref_get_length(&gc.item);
        else if (kind == PINTA_KIND_CHAR)
            length += pinta_char_ref_get_length(&gc.item);
        else
            PINTA_THROW(PINTA_EXCEPTION_INVALID_OPERATION); // assertion?
    }

    PINTA_CHECK(pinta_lib_string_alloc(core, length, &gc.string));

    destination = pinta_string_ref_get_data(&gc.string);
    for (index = 0; index < PINTA_MULTISTRING_COUNT; index++)
    {
        u8 kind;

        gc.item.reference = pinta_multistring_ref_get_item(value, index);
        if (gc.item.reference == NULL)
            continue;

        kind = gc.item.reference->block_kind;
        if (kind == PINTA_KIND_STRING)
        {
            u32 part_length = pinta_string_ref_get_length(&gc.item);
            memcpy(destination, pinta_string_ref_get_data(&gc.item), part_length * sizeof(wchar));

            destination = &destination[part_length];
        }
        else if (kind == PINTA_KIND_SUBSTRING)
        {
            u32 source_offset = pinta_substring_ref_get_offset(&gc.item);
            u32 source_length = pinta_substring_ref_get_length(&gc.item);
            PintaHeapObject *source_string = pinta_substring_ref_get_value(&gc.item);

            if (source_offset >= pinta_string_get_length(source_string))
                PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);    // assertion?

            if (source_offset + source_length > pinta_string_get_length(source_string))
                PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);    // assertion?

            source = pinta_string_get_data(source_string);
            source = &source[source_offset];

            memcpy(destination, source, source_length * sizeof(wchar));

            destination = &destination[source_length];
        }
        else if (kind == PINTA_KIND_CHAR)
        {
            *destination = pinta_char_ref_get_value(&gc.item);
            destination = &destination[1];
        }
        else
        {
            pinta_assert(0); // this IS assertion because this case should have been handled before
            PINTA_THROW(PINTA_EXCEPTION_ENGINE);
        }
    }

    result->reference = gc.string.reference;

    PINTA_GC_RETURN(core);
}

PintaException pinta_lib_multistring_append(PintaCore *core, PintaReference *left, PintaReference *right, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    u8 kind;
    u32 index, rindex, rcount;
    PintaHeapObject *item;
    PintaReference string;

    pinta_assert(core != NULL);
    pinta_assert(left != NULL);
    pinta_assert(right != NULL);

    PINTA_GC_ENTER(core, string);

    if (left->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (left->reference->block_kind != PINTA_KIND_MULTISTRING)    // oops this method is supposed to be used with multistring
        PINTA_THROW(PINTA_EXCEPTION_INVALID_OPERATION);

    if (right->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);  // we cannot append null

    PINTA_CHECK(pinta_lib_multistring_alloc_value(core, left, &string));

    // we have a copy of left in string
    // multistring_alloc_value ensures that there are no NULLs between non-NULLs
    index = 0;
    for (index = 0; index < PINTA_MULTISTRING_COUNT; index++)
    {
        item = pinta_multistring_ref_get_item(&string, index);
        if (item == NULL)
            break;
    }

    if (index >= PINTA_MULTISTRING_COUNT)
    {
        // we do not have enough space for anything else in this multistring
        // convert it to string
        PINTA_CHECK(pinta_lib_string_to_string(core, left, left));

        pinta_multistring_ref_set_item(&string, 0, left);
        for (index = 1; index < PINTA_MULTISTRING_COUNT; index++)
            pinta_multistring_ref_set_null(&string, index);
        index = 1;
    }

    // at this point we are guaranteed that index points to an empty parts element
    kind = right->reference->block_kind;
    switch (kind)
    {
    case PINTA_KIND_STRING:
    case PINTA_KIND_SUBSTRING:
    case PINTA_KIND_CHAR:
        pinta_multistring_ref_set_item(&string, index, right);
        break;
    case PINTA_KIND_MULTISTRING:
        rcount = 0;
        for (rindex = 0; rindex < PINTA_MULTISTRING_COUNT; rindex++)
        {
            item = pinta_multistring_ref_get_item(right, rindex);
            if (item != NULL)
                rcount++;
        }

        if (index + rcount > PINTA_MULTISTRING_COUNT)
        {
            PINTA_CHECK(pinta_lib_string_to_string(core, right, right));

            pinta_multistring_ref_set_item(&string, index, right);
        }
        else
        {
            for (rindex = 0; rindex < PINTA_MULTISTRING_COUNT; rindex++)
            {
                item = pinta_multistring_ref_get_item(right, rindex);
                if (item != NULL)
                    pinta_multistring_ref_set_item_object(&string, index++, item);
            }
        }
        break;
    }

    result->reference = string.reference;

    PINTA_GC_RETURN(core);
}

PintaException pinta_lib_multistring_prepend(PintaCore *core, PintaReference *left, PintaReference *right, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    u32 index, lindex, lcount;
    PintaHeapObject *item;
    PintaReference string;

    pinta_assert(core != NULL);
    pinta_assert(left != NULL);
    pinta_assert(right != NULL);

    PINTA_GC_ENTER(core, string);

    if (left->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (left->reference->block_kind != PINTA_KIND_MULTISTRING)    // oops this method is supposed to be used with multistring
        PINTA_THROW(PINTA_EXCEPTION_INVALID_OPERATION);

    if (right->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);  // we cannot append null

    PINTA_CHECK(pinta_lib_multistring_alloc_value(core, right, &string));

    // we have a copy of left in string
    // multistring_alloc_value ensures that there are no NULLs between non-NULLs
    index = 0;
    for (index = 0; index < PINTA_MULTISTRING_COUNT; index++)
    {
        item = pinta_multistring_ref_get_item(&string, index);
        if (item == NULL)
            break;
    }

    if (index >= PINTA_MULTISTRING_COUNT)
    {
        // we do not have enough space for anything else in this multistring
        // convert it to string
        PINTA_CHECK(pinta_lib_string_to_string(core, right, right));

        pinta_multistring_ref_set_item(&string, 0, right);
        for (index = 1; index < PINTA_MULTISTRING_COUNT; index++)
            pinta_multistring_ref_set_item(&string, index, NULL);
        index = 1;
    }

    // at this point we are guaranteed that index points to an empty parts element
    lcount = 0;
    for (lindex = 0; lindex < PINTA_MULTISTRING_COUNT; lindex++)
    {
        item = pinta_multistring_ref_get_item(left, lindex);
        if (item != NULL)
            lcount++;
    }

    if (index + lcount > PINTA_MULTISTRING_COUNT)
    {
        PINTA_CHECK(pinta_lib_string_to_string(core, left, left));

        pinta_multistring_ref_set_item(&string, index, left);
    }
    else
    {
        for (lindex = 0; lindex < PINTA_MULTISTRING_COUNT; lindex++)
        {
            item = pinta_multistring_ref_get_item(left, lindex);
            if (item != NULL)
                pinta_multistring_ref_set_item_object(&string, index++, item);
        }
    }

    result->reference = string.reference;

    PINTA_GC_RETURN(core);
}

PintaException pinta_lib_multistring_get_length(PintaCore *core, PintaReference *string, u32 *result)
{
    PintaException exception = PINTA_OK;

    pinta_assert(core != NULL);
    pinta_assert(string != NULL);
    pinta_assert(result != NULL);

    if (string->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (string->reference->block_kind != PINTA_KIND_MULTISTRING)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    *result = pinta_multistring_ref_get_length(string);

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_multistring_get_charcode(PintaCore *core, PintaReference *string, u32 index, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    PintaHeapObject **data;
    u32 offset = 0;
    u32 offset_end = 0;
    u32 item_index = 0;
    PintaReference item;

    pinta_assert(core != NULL);
    pinta_assert(string != NULL);
    pinta_assert(result != NULL);

    PINTA_GC_ENTER(core, item);

    if (string->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    pinta_assert(string->reference->block_kind == PINTA_KIND_MULTISTRING);

    data = pinta_multistring_ref_get_value(string);
    if (data == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    for (item_index = 0; item_index < PINTA_MULTISTRING_COUNT; item_index++)
    {
        u8 kind;

        item.reference = pinta_multistring_ref_get_item(string, item_index);
        if (item.reference == NULL)
            continue;

        kind = item.reference->block_kind;
        if (kind == PINTA_KIND_STRING)
        {
            offset_end = offset + pinta_string_ref_get_length(&item);
            if (index >= offset && index < offset_end)
            {
                PINTA_CHECK(pinta_lib_string_get_item(core, &item, index - offset, result));
                PINTA_RETURN();
            }
        }
        else if (kind == PINTA_KIND_SUBSTRING)
        {
            offset_end = offset + pinta_substring_ref_get_length(&item);
            if (index >= offset && index < offset_end)
            {
                PINTA_CHECK(pinta_lib_substring_get_item(core, &item, index - offset, result));
                PINTA_RETURN();
            }
        }
        else if (kind == PINTA_KIND_CHAR)
        {
            offset_end = offset + pinta_char_ref_get_length(&item);
            if (index >= offset && index < offset_end)
            {
                PINTA_CHECK(pinta_lib_char_get_item(core, &item, index - offset, result));
                PINTA_RETURN();
            }
        }
        else
        {
            PINTA_THROW(PINTA_EXCEPTION_INVALID_OPERATION);
        }

        offset = offset_end;
    }

    PINTA_THROW(PINTA_EXCEPTION_INDEX_OUT_OF_BOUNDS);

    PINTA_GC_RETURN(core);
}

PintaException pinta_lib_multistring_get_char(PintaCore *core, PintaReference *string, u32 index, wchar *result)
{
    PintaException exception = PINTA_OK;
    PintaHeapObject **data;
    u32 offset = 0;
    u32 offset_end = 0;
    u32 item_index = 0;
    PintaReference item;

    pinta_assert(core != NULL);
    pinta_assert(string != NULL);
    pinta_assert(result != NULL);

    PINTA_GC_ENTER(core, item);

    if (string->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_INDEX_OUT_OF_BOUNDS);

    pinta_assert(string->reference->block_kind == PINTA_KIND_MULTISTRING);

    data = pinta_multistring_ref_get_value(string);
    if (data == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    for (item_index = 0; item_index < PINTA_MULTISTRING_COUNT; item_index++)
    {
        u8 kind;

        item.reference = pinta_multistring_ref_get_item(string, item_index);
        if (item.reference == NULL)
            continue;

        kind = item.reference->block_kind;
        if (kind == PINTA_KIND_STRING)
        {
            offset_end = offset + pinta_string_ref_get_length(&item);
            if (index >= offset && index < offset_end)
            {
                PINTA_CHECK(pinta_lib_string_get_char(core, &item, index - offset, result));
                PINTA_RETURN();
            }
        }
        else if (kind == PINTA_KIND_SUBSTRING)
        {
            offset_end = offset + pinta_substring_ref_get_length(&item);
            if (index >= offset && index < offset_end)
            {
                PINTA_CHECK(pinta_lib_substring_get_char(core, &item, index - offset, result));
                PINTA_RETURN();
            }
        }
        else if (kind == PINTA_KIND_CHAR)
        {
            offset_end = offset + pinta_char_ref_get_length(&item);
            if (index >= offset && index < offset_end)
            {
                PINTA_CHECK(pinta_lib_char_get_char(core, &item, index - offset, result));
                PINTA_RETURN();
            }
        }
        else
        {
            PINTA_THROW(PINTA_EXCEPTION_INVALID_OPERATION);
        }

        offset = offset_end;
    }

    PINTA_THROW(PINTA_EXCEPTION_INDEX_OUT_OF_BOUNDS);

    PINTA_GC_RETURN(core);
}

PintaException pinta_lib_multistring_get_member(PintaCore *core, PintaReference *string, PintaReference *name, u8 *is_accessor, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    i32 value;

    if (name->reference != NULL && name->reference->block_kind == PINTA_KIND_INTEGER)
    {
        value = pinta_integer_ref_get_value(name);
        if (value >= 0)
        {
            if (is_accessor != NULL)
                *is_accessor = 0;
            PINTA_CHECK(pinta_lib_multistring_get_item(core, string, (u32)value, result));
            PINTA_RETURN();
        }
    }

    PINTA_THROW(PINTA_EXCEPTION_NOT_IMPLEMENTED);

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_multistring_get_item(PintaCore *core, PintaReference *string, u32 index, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    wchar character;

    pinta_assert(core != NULL);
    pinta_assert(string != NULL);
    pinta_assert(result != NULL);

    PINTA_CHECK(pinta_lib_multistring_get_char(core, string, index, &character));
    PINTA_CHECK(pinta_lib_char_alloc_value(core, character, result));

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_multistring_to_bool(PintaCore *core, PintaReference *string, u8 *result)
{
    PINTA_UNUSED(core);

    if (pinta_multistring_ref_get_length(string) == 0)
        *result = 0;
    else
        *result = 1;

    return PINTA_OK;
}

PintaException pinta_lib_multistring_debug_write(PintaCore *core, PintaReference *value, u32 max_depth, PintaJsonWriter *writer)
{
    PintaException exception = PINTA_OK;
    PintaReference string;

    pinta_assert(core != NULL);
    pinta_assert(value != NULL);

    if (value->reference == NULL)
        return pinta_json_write_null(core, writer);
    
    PINTA_GC_ENTER(core, string);

    PINTA_CHECK(pinta_lib_multistring_to_string(core, value, &string));
    PINTA_CHECK(pinta_lib_string_debug_write(core, &string, max_depth, writer));

    PINTA_GC_RETURN(core);
}

/* CHAR */

void pinta_char_init_type(PintaType *type)
{
    type->is_string = 1;

    type->to_integer = pinta_lib_string_to_integer;
    type->to_integer_value = pinta_lib_string_to_integer_value;
    type->to_string = pinta_core_default_copy;
    type->to_decimal = pinta_lib_string_to_decimal;
    type->to_numeric = pinta_lib_string_to_decimal;

    type->get_member = pinta_lib_char_get_member;
    type->get_item = pinta_lib_char_get_item;

    type->get_length = pinta_lib_char_get_length;
    type->get_char = pinta_lib_char_get_char;

#if PINTA_DEBUG
    type->debug_write = pinta_lib_char_debug_write;
#endif
}

PintaHeapObject *pinta_char_alloc_object_value(PintaCore *core, wchar value)
{
    PintaHeapObject *result;

    pinta_assert(core != NULL);

    if (value < 128)
        return &core->cache->chars[value];

    result = pinta_core_alloc(core, PINTA_KIND_CHAR, PINTA_FLAG_NONE, 1);
    pinta_char_set_value(result, value);

    return result;
}

PintaException pinta_lib_char_alloc_value(PintaCore *core, wchar value, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    PintaHeapObject *character;

    pinta_assert(core != NULL);
    pinta_assert(result != NULL);

    character = pinta_char_alloc_object_value(core, value);
    if (character == NULL)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_MEMORY);

    result->reference = character;
PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_char_to_string(PintaCore *core, PintaReference *value, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    wchar *destination;
    PintaHeapObject *string;

    pinta_assert(core != NULL);

    if (value->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    pinta_assert(value->reference->block_kind == PINTA_KIND_CHAR);

    string = pinta_string_alloc_object(core, 1);

    destination = pinta_string_get_data(string);

    destination[0] = pinta_char_ref_get_value(value);
    destination[1] = PINTA_CHAR('\0');

    result->reference = string;

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_char_get_length(PintaCore *core, PintaReference *string, u32 *result)
{
    PintaException exception = PINTA_OK;

    pinta_assert(core != NULL);
    pinta_assert(string != NULL);
    pinta_assert(result != NULL);

    if (string->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (string->reference->block_kind != PINTA_KIND_CHAR)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    *result = 1;

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_char_get_charcode(PintaCore *core, PintaReference *string, u32 index, PintaReference *result)
{
    PintaException exception = PINTA_OK;

    pinta_assert(core != NULL);
    pinta_assert(string != NULL);
    pinta_assert(result != NULL);

    if (string->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    pinta_assert(string->reference->block_kind == PINTA_KIND_CHAR);

    if (index > 0)
        PINTA_THROW(PINTA_EXCEPTION_INDEX_OUT_OF_BOUNDS);

    PINTA_CHECK(pinta_lib_integer_alloc_value(core, (i32)pinta_char_ref_get_value(string), result));

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_char_get_char(PintaCore *core, PintaReference *string, u32 index, wchar *result)
{
    PintaException exception = PINTA_OK;

    PINTA_UNUSED(core);

    pinta_assert(core != NULL);
    pinta_assert(string != NULL);
    pinta_assert(result != NULL);

    if (index > 0)
        PINTA_THROW(PINTA_EXCEPTION_INDEX_OUT_OF_BOUNDS);

    *result = pinta_char_ref_get_value(string);

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_char_get_member(PintaCore *core, PintaReference *string, PintaReference *name, u8 *is_accessor, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    i32 value;

    if (name->reference != NULL && name->reference->block_kind == PINTA_KIND_INTEGER)
    {
        value = pinta_integer_ref_get_value(name);
        if (value >= 0)
        {
            if (is_accessor != NULL)
                *is_accessor = 0;
            PINTA_CHECK(pinta_lib_char_get_item(core, string, (u32)value, result));
            PINTA_RETURN();
        }
    }

    PINTA_THROW(PINTA_EXCEPTION_NOT_IMPLEMENTED);

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_char_get_item(PintaCore *core, PintaReference *string, u32 index, PintaReference *result)
{
    PintaException exception = PINTA_OK;

    PINTA_UNUSED(core);

    pinta_assert(core != NULL);
    pinta_assert(string != NULL);
    pinta_assert(result != NULL);

    if (index > 0)
        PINTA_THROW(PINTA_EXCEPTION_INDEX_OUT_OF_BOUNDS);

    result->reference = string->reference;

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_char_debug_write(PintaCore *core, PintaReference *value, u32 max_depth, PintaJsonWriter *writer)
{
    wchar data;
    u32 length;

    PINTA_UNUSED(max_depth);

    pinta_assert(core != NULL);
    pinta_assert(value != NULL);

    if (value->reference == NULL)
        return pinta_json_write_null(core, writer);

    data = pinta_char_ref_get_value(value);
    length = pinta_char_ref_get_length(value);

    return pinta_json_write_string(core, writer, &data, length);
}
