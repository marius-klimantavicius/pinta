#include "pinta.h"

static wchar type_literal[] = { 116, 121, 112, 101, 0 };
static wchar blob_literal[] = { 98, 108, 111, 98, 0 };
static wchar length_literal[] = { 108, 101, 110, 103, 116, 104, 0 };
static wchar data_literal[] = { 100, 97, 116, 97, 0 };
static wchar buffer_literal[] = { 98, 117, 102, 102, 101, 114, 0 };
static wchar position_literal[] = { 112, 111, 115, 105, 116, 105, 111, 110, 0 };

/* BLOB accessors */

u32 pinta_blob_block_length(u32 blob_length)
{
    return (sizeof(PintaHeapObject) + blob_length + 1 + sizeof(PintaHeapObject) - 1) / sizeof(PintaHeapObject);
}

void pinta_blob_set_length(PintaHeapObject *buffer, u32 length)
{
    pinta_assert(buffer != NULL);
    pinta_assert(buffer->block_kind == PINTA_KIND_BLOB);

    buffer->data.blob.blob_length = length;
}

void pinta_blob_set_data(PintaHeapObject *buffer, u8 *data)
{
    pinta_assert(buffer != NULL);
    pinta_assert(buffer->block_kind == PINTA_KIND_BLOB);

    buffer->data.blob.blob_data = data;
}

void pinta_blob_set_item(PintaHeapObject *buffer, u32 index, u8 value)
{
    pinta_assert(buffer != NULL);
    pinta_assert(buffer->block_kind == PINTA_KIND_BLOB);
    pinta_assert(index < buffer->data.blob.blob_length);

    buffer->data.blob.blob_data[index] = value;
}

u32 pinta_blob_get_length(PintaHeapObject *buffer)
{
    pinta_assert(buffer != NULL);
    pinta_assert(buffer->block_kind == PINTA_KIND_BLOB);

    return buffer->data.blob.blob_length;
}

u8 *pinta_blob_get_data(PintaHeapObject *buffer)
{
    pinta_assert(buffer != NULL);
    pinta_assert(buffer->block_kind == PINTA_KIND_BLOB);

    return buffer->data.blob.blob_data;
}

u8 pinta_blob_get_item(PintaHeapObject *buffer, u32 index)
{
    pinta_assert(buffer != NULL);
    pinta_assert(buffer->block_kind == PINTA_KIND_BLOB);
    pinta_assert(index < buffer->data.blob.blob_length);

    return buffer->data.blob.blob_data[index];
}

u32 pinta_blob_ref_get_length(PintaReference *reference)
{
    pinta_assert(reference != NULL);
    pinta_assert(reference->reference != NULL);

    return pinta_blob_get_length(reference->reference);
}

u8 *pinta_blob_ref_get_data(PintaReference *reference)
{
    pinta_assert(reference != NULL);
    pinta_assert(reference->reference != NULL);

    return pinta_blob_get_data(reference->reference);
}

u8 pinta_blob_ref_get_item(PintaReference *reference, u32 index)
{
    pinta_assert(reference != NULL);
    pinta_assert(reference->reference != NULL);

    return pinta_blob_get_item(reference->reference, index);
}

void pinta_blob_ref_set_length(PintaReference *reference, u32 length)
{
    pinta_assert(reference != NULL);
    pinta_assert(reference->reference != NULL);

    pinta_blob_set_length(reference->reference, length);
}

void pinta_blob_ref_set_item(PintaReference *reference, u32 index, u8 value)
{
    pinta_assert(reference != NULL);
    pinta_assert(reference->reference != NULL);

    pinta_blob_set_item(reference->reference, index, value);
}

/* BUFFER accessors */

void pinta_buffer_set_length(PintaHeapObject *buffer, u32 length)
{
    pinta_assert(buffer != NULL);
    pinta_assert(buffer->block_kind == PINTA_KIND_BUFFER);

    buffer->data.buffer.buffer_length = length;
}

void pinta_buffer_set_position(PintaHeapObject *buffer, u32 position)
{
    pinta_assert(buffer != NULL);
    pinta_assert(buffer->block_kind == PINTA_KIND_BUFFER);

    buffer->data.buffer.buffer_position = position;
}

void pinta_buffer_set_blob(PintaHeapObject *buffer, PintaHeapObject *blob)
{
    PintaBufferBlob *buffer_blob;

    pinta_assert(buffer != NULL);
    pinta_assert(buffer->block_kind == PINTA_KIND_BUFFER);
    pinta_assert(buffer->block_length > 1);
    pinta_assert(blob == NULL || blob->block_kind == PINTA_KIND_BLOB);

    buffer_blob = (PintaBufferBlob*)&buffer[1];
    buffer_blob->buffer_blob = blob;
}

u32 pinta_buffer_get_length(PintaHeapObject *buffer)
{
    pinta_assert(buffer != NULL);
    pinta_assert(buffer->block_kind == PINTA_KIND_BUFFER);

    return buffer->data.buffer.buffer_length;
}

u32 pinta_buffer_get_position(PintaHeapObject *buffer)
{
    pinta_assert(buffer != NULL);
    pinta_assert(buffer->block_kind == PINTA_KIND_BUFFER);

    return buffer->data.buffer.buffer_position;
}

PintaHeapObject *pinta_buffer_get_blob(PintaHeapObject *buffer)
{
    PintaBufferBlob *buffer_blob;

    pinta_assert(buffer != NULL);
    pinta_assert(buffer->block_kind == PINTA_KIND_BUFFER);
    pinta_assert(buffer->block_length > 1);

    buffer_blob = (PintaBufferBlob*)&buffer[1];
    return buffer_blob->buffer_blob;
}

void pinta_buffer_ref_set_length(PintaReference *reference, u32 length)
{
    pinta_assert(reference != NULL);
    pinta_assert(reference->reference != NULL);

    pinta_buffer_set_length(reference->reference, length);
}

void pinta_buffer_ref_set_position(PintaReference *reference, u32 position)
{
    pinta_assert(reference != NULL);
    pinta_assert(reference->reference != NULL);

    pinta_buffer_set_position(reference->reference, position);
}

void pinta_buffer_ref_set_blob_object(PintaReference *reference, PintaHeapObject *blob)
{
    pinta_assert(reference != NULL);
    pinta_assert(reference->reference != NULL);
    pinta_assert(blob != NULL);

    pinta_buffer_set_blob(reference->reference, blob);
}

void pinta_buffer_ref_set_blob(PintaReference *reference, PintaReference *blob)
{
    pinta_assert(reference != NULL);
    pinta_assert(reference->reference != NULL);
    pinta_assert(blob != NULL);
    pinta_assert(blob->reference != NULL);

    pinta_buffer_set_blob(reference->reference, blob->reference);
}

void pinta_buffer_ref_set_blob_null(PintaReference *reference)
{
    pinta_assert(reference != NULL);
    pinta_assert(reference->reference != NULL);

    pinta_buffer_set_blob(reference->reference, NULL);
}

u32 pinta_buffer_ref_get_length(PintaReference *reference)
{
    pinta_assert(reference != NULL);
    pinta_assert(reference->reference != NULL);

    return pinta_buffer_get_length(reference->reference);
}

u32 pinta_buffer_ref_get_position(PintaReference *reference)
{
    pinta_assert(reference != NULL);
    pinta_assert(reference->reference != NULL);

    return pinta_buffer_get_position(reference->reference);
}

PintaHeapObject *pinta_buffer_ref_get_blob(PintaReference *reference)
{
    pinta_assert(reference != NULL);
    pinta_assert(reference->reference != NULL);

    return pinta_buffer_get_blob(reference->reference);
}

/* BLOB */

void pinta_blob_init_type(PintaType *type)
{
    type->is_string = 0;

    type->gc_relocate = pinta_blob_relocate;

    type->get_member = pinta_lib_blob_get_member;
    type->set_member = pinta_lib_blob_set_member;

    type->set_item = pinta_lib_blob_write_byte;
    type->get_item = pinta_lib_blob_read_byte;

    type->get_length = pinta_lib_blob_get_length;

#if PINTA_DEBUG
    type->debug_write = pinta_lib_blob_debug_write;
#endif
}

void pinta_blob_relocate(PintaHeapObject *object, PintaHeapReloc *reloc, u32 count)
{
    u8 *data;
    PintaHeapReloc *entry;

    pinta_gc_relocate_validate(PINTA_KIND_BLOB, object, reloc, count);

    data = pinta_blob_get_data(object);
    if (data == NULL)
        return;

    entry = pinta_gc_relocate_find(data, reloc, count);
    if (entry != NULL)
        pinta_blob_set_data(object, data - (sizeof(PintaHeapObject)* entry->offset));
}

PintaHeapObject *pinta_blob_alloc_object_value(PintaCore *core, PintaReference *value, u32 blob_length)
{
    u32 size;
    PintaHeapObject *result;
    u8 *data;
    u32 current_length;
    u32 remaining;

    pinta_assert(core != NULL);
    pinta_assert(value != NULL);
    pinta_assert(value->reference == NULL || value->reference->block_kind == PINTA_KIND_BLOB);

    size = pinta_blob_block_length(blob_length);
    result = pinta_core_alloc(core, PINTA_KIND_BLOB, PINTA_FLAG_NONE, size);
    if (result == NULL)
        return NULL;

    data = (u8*)&result[1];

    pinta_blob_set_length(result, blob_length);
    pinta_blob_set_data(result, data);

    current_length = 0;
    if (value->reference != NULL)
        current_length = pinta_blob_ref_get_length(value);

    if (blob_length > current_length)
    {
        if (value->reference != NULL)
            memcpy(data, pinta_blob_ref_get_data(value), current_length);

        if (size > 1)
        {
            remaining = (size - 1) * sizeof(PintaHeapObject) - current_length;
            if (remaining > 0)
                memset(&data[current_length], 0, remaining);
        }
    }
    else
    {
        if (value->reference != NULL)
            memcpy(data, pinta_blob_ref_get_data(value), blob_length);
        else if (size > 1)
            memset(data, 0, (size - 1) * sizeof(PintaHeapObject));
    }

    return result;
}

PintaHeapObject *pinta_blob_alloc_object(PintaCore *core, u32 blob_length)
{
    u32 size;
    PintaHeapObject *result;
    u8 *data;

    pinta_assert(core != NULL);

    size = pinta_blob_block_length(blob_length);
    result = pinta_core_alloc(core, PINTA_KIND_BLOB, PINTA_FLAG_NONE, size);
    if (result == NULL)
        return NULL;

    data = (u8*)&result[1];

    pinta_blob_set_length(result, blob_length);
    pinta_blob_set_data(result, data);

    if (size > 1)
        memset(data, 0, (size - 1) * sizeof(PintaHeapObject));

    return result;
}

PintaException pinta_lib_blob_alloc(PintaCore *core, u32 blob_length, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    PintaHeapObject *value;

    pinta_assert(core != NULL);
    pinta_assert(result != NULL);

    value = pinta_blob_alloc_object(core, blob_length);
    if (value == NULL)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_MEMORY);

    result->reference = value;

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_blob_get_member(PintaCore *core, PintaReference *blob, PintaReference *name, u8 *is_accessor, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    i32 index;

    if (name->reference != NULL && name->reference->block_kind == PINTA_KIND_INTEGER)
    {
        index = pinta_integer_ref_get_value(name);
        if (index >= 0)
        {
            if (is_accessor)
                *is_accessor = 0;

            PINTA_CHECK(pinta_lib_blob_read_byte(core, blob, (u32)index, result));
            PINTA_RETURN();
        }
    }

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_blob_set_member(PintaCore *core, PintaReference *blob, PintaReference *name, PintaReference *value, u8 *is_accessor, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    i32 index;

    PINTA_UNUSED(result);

    if (name->reference != NULL && name->reference->block_kind == PINTA_KIND_INTEGER)
    {
        index = pinta_integer_ref_get_value(name);
        if (index >= 0)
        {
            if (is_accessor)
                *is_accessor = 0;

            PINTA_CHECK(pinta_lib_blob_write_byte(core, blob, (u32)index, value));
            PINTA_RETURN();
        }
    }

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_blob_get_length(PintaCore *core, PintaReference *blob, u32 *result)
{
    PintaException exception = PINTA_OK;

    pinta_assert(core != NULL);
    pinta_assert(blob != NULL);
    pinta_assert(result != NULL);

    if (blob->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (blob->reference->block_kind != PINTA_KIND_BLOB)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    *result = pinta_blob_ref_get_length(blob);

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_blob_resize(PintaCore *core, PintaReference *blob, u32 blob_length, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    u32 block_length;
    PintaHeapObject *value;

    pinta_assert(core != NULL);
    pinta_assert(blob != NULL);
    pinta_assert(result != NULL);

    if (blob->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (blob->reference->block_kind != PINTA_KIND_BLOB)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    block_length = pinta_blob_block_length(blob_length);
    if (block_length <= blob->reference->block_length)
    {
        pinta_blob_ref_set_length(blob, blob_length);
        result->reference = blob->reference;
        PINTA_RETURN();
    }

    value = pinta_blob_alloc_object_value(core, blob, blob_length);
    if (value == NULL)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_MEMORY);

    result->reference = value;

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_blob_write_byte_value(PintaHeapObject *blob, u32 position, u8 value)
{
    PintaException exception = PINTA_OK;
    u32 length;
    u8 *data;

    if (blob == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (blob->block_kind != PINTA_KIND_BLOB)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    length = pinta_blob_get_length(blob);
    if (position >= length)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

    if ((position + 1) > length)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

    data = pinta_blob_get_data(blob);
    data[position] = value;

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_blob_write_byte(PintaHeapObject *blob, u32 position, PintaReference *value)
{
    PintaException exception = PINTA_OK;
    u32 length;
    i32 write_value;
    u8 *data;

    pinta_assert(value != NULL);

    if (blob == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (blob->block_kind != PINTA_KIND_BLOB)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    if (value->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (value->reference->block_kind != PINTA_KIND_INTEGER)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    length = pinta_blob_get_length(blob);
    if (position >= length)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

    if ((position + 1) > length)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

    write_value = pinta_integer_ref_get_value(value);

    data = pinta_blob_get_data(blob);
    data[position] = (u8)(write_value);

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_blob_write_short_value(PintaHeapObject *blob, u32 position, u16 value)
{
    PintaException exception = PINTA_OK;
    u32 length;
    u8 *data;
    u8 *value_data;

    if (blob == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (blob->block_kind != PINTA_KIND_BLOB)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    length = pinta_blob_get_length(blob);
    if (position >= length)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

    if ((position + 2) > length)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

    data = pinta_blob_get_data(blob);

    value_data = (u8*)&value;

    data[position] = value_data[0];
    data[position + 1] = value_data[1];

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_blob_write_short(PintaHeapObject *blob, u32 position, PintaReference *value)
{
    PintaException exception = PINTA_OK;
    u32 length;
    i32 write_value;
    u8 *write_data;

    pinta_assert(value != NULL);

    if (blob == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (blob->block_kind != PINTA_KIND_BLOB)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    if (value->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (value->reference->block_kind != PINTA_KIND_INTEGER)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    length = pinta_blob_get_length(blob);
    if (position >= length)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

    if ((position + 2) > length)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

    write_value = pinta_integer_ref_get_value(value);
    write_data = (u8*)&write_value;

    pinta_blob_set_item(blob, position, write_data[0]);
    pinta_blob_set_item(blob, position + 1, write_data[1]);

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_blob_write_char_value(PintaHeapObject *blob, u32 position, wchar value)
{
    PintaException exception = PINTA_OK;
    u32 length;
    u8 *data;
    u8 *value_data;

    if (blob == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (blob->block_kind != PINTA_KIND_BLOB)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    length = pinta_blob_get_length(blob);
    if (position >= length)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

    if ((position + sizeof(wchar)) > length)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

    data = pinta_blob_get_data(blob);
    value_data = (u8*)&value;

    data[position + 1] = value_data[1];
    data[position] = value_data[0];

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);

}

PintaException pinta_blob_write_char(PintaHeapObject *blob, u32 position, PintaReference *value)
{
    PintaException exception = PINTA_OK;
    wchar write_value;

    pinta_assert(value != NULL);

    if (blob == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (blob->block_kind != PINTA_KIND_BLOB)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    if (value->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (value->reference->block_kind != PINTA_KIND_CHAR)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    write_value = pinta_char_ref_get_value(value);

    PINTA_CHECK(pinta_blob_write_char_value(blob, position, write_value));

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_blob_write_integer_value(PintaHeapObject *blob, u32 position, u32 value)
{
    PintaException exception = PINTA_OK;
    u32 length;
    u8 *data;
    u8 *value_data;

    if (blob == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (blob->block_kind != PINTA_KIND_BLOB)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    length = pinta_blob_get_length(blob);
    if (position >= length)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

    if ((position + 4) > length)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

    data = pinta_blob_get_data(blob);

    value_data = (u8*)&value;
    data[position] = value_data[0];
    data[position + 1] = value_data[1];
    data[position + 2] = value_data[2];
    data[position + 3] = value_data[3];

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_blob_write_integer(PintaHeapObject *blob, u32 position, PintaReference *value)
{
    PintaException exception = PINTA_OK;
    u32 length;
    i32 write_value;
    u8 *data;
    u8 *write_data;

    pinta_assert(value != NULL);

    if (blob == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (blob->block_kind != PINTA_KIND_BLOB)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    if (value->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (value->reference->block_kind != PINTA_KIND_INTEGER)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    length = pinta_blob_get_length(blob);
    if (position >= length)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

    if ((position + 4) > length)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

    write_value = pinta_integer_ref_get_value(value);

    data = pinta_blob_get_data(blob);

    write_data = (u8*)&write_value;
    data[position] = write_data[0];
    data[position + 1] = write_data[1];
    data[position + 2] = write_data[2];
    data[position + 3] = write_data[3];

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_blob_write_string(PintaHeapObject *blob, u32 position, u32 data_offset, u32 data_length, PintaHeapObject *value)
{
    PintaException exception = PINTA_OK;
    u32 blob_length;
    u8 *destination;
    wchar *source;

    if (blob == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (blob->block_kind != PINTA_KIND_BLOB)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    if (value == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (value->block_kind != PINTA_KIND_STRING)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    blob_length = pinta_blob_get_length(blob);
    if (position >= blob_length)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

    if ((position + data_length) > blob_length)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

    destination = pinta_blob_get_data(blob);
    source = pinta_string_get_data(value);

    destination = &destination[position];
    source = &source[data_offset];

    memmove(destination, source, data_length * sizeof(wchar));

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_blob_write_blob(PintaHeapObject *blob, u32 position, u32 data_offset, u32 data_length, PintaHeapObject *value)
{
    PintaException exception = PINTA_OK;
    u32 blob_length;
    u8 *destination;
    u8 *source;

    if (blob == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (blob->block_kind != PINTA_KIND_BLOB)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    if (value == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (value->block_kind != PINTA_KIND_BLOB)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    blob_length = pinta_blob_get_length(blob);
    if (position >= blob_length)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

    if (data_length > blob_length)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

    if ((position + data_length) > blob_length)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

    destination = pinta_blob_get_data(blob);
    source = pinta_blob_get_data(value);

    destination = &destination[position];
    source = &source[data_offset];

    memmove(destination, source, data_length);

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_blob_write_data(PintaHeapObject *blob, u32 position, u8 *data, u32 data_length)
{
    PintaException exception = PINTA_OK;
    u32 blob_length;
    u8 *destination;

    if (blob == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (blob->block_kind != PINTA_KIND_BLOB)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    if (data == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    blob_length = pinta_blob_get_length(blob);
    if (position >= blob_length)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

    if ((position + data_length) > blob_length)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

    destination = pinta_blob_get_data(blob);
    destination = &destination[position];

    memmove(destination, data, data_length);

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_blob_read_byte(PintaHeapObject *blob, u32 position, i32 *result)
{
    PintaException exception = PINTA_OK;
    u32 length;
    u8 *data;

    pinta_assert(result != NULL);

    if (blob == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (blob->block_kind != PINTA_KIND_BLOB)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    length = pinta_blob_get_length(blob);
    if (position >= length)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

    if ((position + 1) > length)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

    data = pinta_blob_get_data(blob);
    *result = (i32)data[position];

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_blob_read_short(PintaHeapObject *blob, u32 position, i32 *result)
{
    PintaException exception = PINTA_OK;
    u32 length;
    i16 read_value = 0;
    u8 *data;
    u8 *data_value;

    pinta_assert(result != NULL);

    if (blob == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (blob->block_kind != PINTA_KIND_BLOB)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    length = pinta_blob_get_length(blob);
    if (position >= length)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

    if ((position + 2) > length)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

    data = pinta_blob_get_data(blob);

    data_value = (u8*)&read_value;
    data_value[0] = data[position];
    data_value[1] = data[position + 1];

    *result = (i32)read_value;

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_blob_read_integer(PintaHeapObject *blob, u32 position, i32 *result)
{
    PintaException exception = PINTA_OK;
    u32 length;
    i32 read_value = 0;
    u8 *data;
    u8 *data_value;

    pinta_assert(result != NULL);

    if (blob == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (blob->block_kind != PINTA_KIND_BLOB)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    length = pinta_blob_get_length(blob);
    if (position >= length)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

    if ((position + 4) > length)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

    data = pinta_blob_get_data(blob);

    data_value = (u8*)&read_value;
    data_value[0] = data[position];
    data_value[1] = data[position + 1];
    data_value[2] = data[position + 2];
    data_value[3] = data[position + 3];

    *result = read_value;

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_blob_read_string_integer(PintaCore *core, PintaReference *blob, u32 position, u32 string_length, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    u32 blob_length;
    u8 *source;
    wchar *destination;
    PintaHeapObject *value;

    pinta_assert(core != NULL);
    pinta_assert(blob != NULL);
    pinta_assert(result != NULL);

    if (blob->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (blob->reference->block_kind != PINTA_KIND_BLOB)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    blob_length = pinta_blob_ref_get_length(blob);
    if (position >= blob_length)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

    if ((position + string_length * sizeof(wchar)) > blob_length)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

    value = pinta_string_alloc_object(core, string_length);
    if (value == NULL)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_MEMORY);

    source = pinta_blob_ref_get_data(blob);
    source = &source[position];

    destination = pinta_string_get_data(value);

    memcpy(destination, source, string_length * sizeof(wchar));

    result->reference = value;

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_blob_read_blob_integer(PintaCore *core, PintaReference *blob, u32 position, u32 data_length, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    u32 blob_length;
    u8 *source;
    u8 *destination;
    PintaHeapObject *value;

    pinta_assert(core != NULL);
    pinta_assert(blob != NULL);
    pinta_assert(result != NULL);

    if (blob->reference == NULL)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_NULL_REFERENCE);

    if (blob->reference->block_kind != PINTA_KIND_BLOB)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_TYPE_MISMATCH);

    blob_length = pinta_blob_ref_get_length(blob);
    if (position >= blob_length)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_OUT_OF_RANGE);

    if ((position + data_length) > blob_length)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_OUT_OF_RANGE);

    value = pinta_blob_alloc_object(core, data_length);
    if (value == NULL)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_MEMORY);

    source = pinta_blob_ref_get_data(blob);
    source = &source[position];

    destination = pinta_blob_get_data(value);

    memcpy(destination, source, data_length);

    result->reference = value;

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_blob_write_byte(PintaCore *core, PintaReference *blob, u32 position, PintaReference *value)
{
    PINTA_UNUSED(core);

    pinta_assert(core != NULL);
    pinta_assert(blob != NULL);

    return pinta_blob_write_byte(blob->reference, position, value);
}

PintaException pinta_lib_blob_read_byte(PintaCore *core, PintaReference *blob, u32 position, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    i32 value;

    pinta_assert(core != NULL);
    pinta_assert(blob != NULL);
    pinta_assert(result != NULL);

    PINTA_CHECK(pinta_blob_read_byte(blob->reference, position, &value));
    PINTA_CHECK(pinta_lib_integer_alloc_value(core, value, result));

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_blob_in_place_to_string(PintaCore *core, PintaReference *blob, u32 string_length, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    u32 blob_length;
    u32 block_length;
    u32 remaining;
    PintaHeapObject *unused;
    wchar *blob_data;
    u32 actual_length;

    PINTA_UNUSED(core);

    pinta_assert(core != NULL);
    pinta_assert(blob != NULL);
    pinta_assert(result != NULL);

    if (blob->reference == NULL)
    {
        result->reference = NULL;
        PINTA_RETURN();
    }

    if (blob->reference->block_kind != PINTA_KIND_BLOB)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    blob_length = pinta_blob_ref_get_length(blob);
    if (blob_length < string_length)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

    block_length = pinta_blob_block_length(string_length);
    if (block_length < blob->reference->block_length)
    {
        remaining = blob->reference->block_length - block_length;
        blob->reference->block_length = block_length;

        unused = &blob->reference[block_length];
        unused->block_flags = PINTA_FLAG_NONE;
        unused->block_kind = PINTA_KIND_BLOB; // we cannot set it to be free because this would screwup the free list;
        unused->block_length = remaining;
        pinta_blob_set_data(unused, NULL);
        pinta_blob_set_length(unused, 0);
    }

    blob_data = (wchar*)pinta_blob_ref_get_data(blob);

    blob->reference->block_kind = PINTA_KIND_STRING;
    pinta_string_ref_set_data(blob, blob_data);

    actual_length = string_length / sizeof(wchar);
    pinta_string_ref_set_length(blob, actual_length);

    result->reference = blob->reference;

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_blob_debug_write(PintaCore *core, PintaReference *value, u32 max_depth, PintaJsonWriter *writer)
{
    PintaException exception = PINTA_OK;
    u8 *data;
    u32 length;

    PINTA_UNUSED(max_depth);

    if (value->reference == NULL)
        return pinta_json_write_null(core, writer);

    pinta_core_pin(value->reference);

    data = pinta_blob_ref_get_data(value);
    length = pinta_blob_ref_get_length(value);

    PINTA_CHECK(pinta_json_write_object_start(core, writer));

    PINTA_CHECK(pinta_json_write_property_string(core, writer, type_literal, blob_literal));
    PINTA_CHECK(pinta_json_write_property_u32(core, writer, length_literal, length));
    PINTA_CHECK(pinta_json_write_property_binary(core, writer, data_literal, data, length));

    PINTA_CHECK(pinta_json_write_end(core, writer));

PINTA_EXIT:
    pinta_core_unpin(value->reference);
    return exception;
}

/* BUFFER */

void pinta_buffer_init_type(PintaType *type)
{
    type->is_string = 0;

    type->gc_walk = pinta_buffer_walk;
    type->gc_relocate = pinta_buffer_relocate;

    type->get_member = pinta_lib_buffer_get_member;
    type->set_member = pinta_lib_buffer_set_member;

    type->set_item = pinta_lib_buffer_write_byte_at;
    type->get_item = pinta_lib_buffer_read_byte_at;

    type->get_length = pinta_lib_buffer_get_length;

#if PINTA_DEBUG
    type->debug_write = pinta_lib_buffer_debug_write;
#endif
}

u32 pinta_buffer_walk(PintaHeapObject *object, PintaHeapObjectWalkerState *state, PintaHeapObject **result)
{
    PintaHeapObject *item;

    pinta_assert(object != NULL);
    pinta_assert(state != NULL);
    pinta_assert(result != NULL);
    pinta_assert(object->block_kind == PINTA_KIND_BUFFER);

    *result = NULL;

    if (state->field != 0)
        return 0;

    item = pinta_buffer_get_blob(object);
    if (item != NULL)
    {
        *result = item;
        state->field = 1;
        return 1;
    }

    return 0;
}

void pinta_buffer_relocate(PintaHeapObject *object, PintaHeapReloc *reloc, u32 count)
{
    PintaHeapObject *blob;
    PintaHeapReloc *entry;

    pinta_gc_relocate_validate(PINTA_KIND_BUFFER, object, reloc, count);

    blob = pinta_buffer_get_blob(object);
    if (blob == NULL)
        return;

    entry = pinta_gc_relocate_find(blob, reloc, count);
    if (entry != NULL)
        pinta_buffer_set_blob(object, blob - entry->offset);
}

PintaHeapObject *pinta_buffer_alloc_object(PintaCore *core)
{
    u32 size;
    PintaHeapObject *result;

    pinta_assert(core != NULL);

    size = (sizeof(PintaHeapObject) + sizeof(PintaBufferBlob) + sizeof(PintaHeapObject) - 1) / sizeof(PintaHeapObject);
    result = pinta_core_alloc(core, PINTA_KIND_BUFFER, PINTA_FLAG_NONE, size);
    if (result == NULL)
        return NULL;

    pinta_buffer_set_length(result, 0);
    pinta_buffer_set_position(result, 0);
    pinta_buffer_set_blob(result, NULL);

    return result;
}

PintaException pinta_lib_buffer_alloc(PintaCore *core, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    PintaHeapObject *value;

    pinta_assert(core != NULL);
    pinta_assert(result != NULL);

    value = pinta_buffer_alloc_object(core);
    if (value == NULL)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_MEMORY);

    result->reference = value;

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_buffer_alloc_value(PintaCore *core, u32 capacity, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    PintaHeapObject *value;

    pinta_assert(core != NULL);
    pinta_assert(result != NULL);

    value = pinta_buffer_alloc_object(core);
    if (value == NULL)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_MEMORY);

    result->reference = value;

    value = pinta_blob_alloc_object(core, capacity);
    if (value == NULL)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_MEMORY);

    pinta_buffer_ref_set_blob_object(result, value);
PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_buffer_get_member(PintaCore *core, PintaReference *blob, PintaReference *name, u8 *is_accessor, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    i32 index;

    pinta_assert(core != NULL);
    pinta_assert(blob != NULL);
    pinta_assert(name != NULL);
    pinta_assert(result != NULL);

    if (name->reference != NULL && name->reference->block_kind == PINTA_KIND_INTEGER)
    {
        index = pinta_integer_ref_get_value(name);
        if (index >= 0)
        {
            if (is_accessor)
                *is_accessor = 0;

            PINTA_CHECK(pinta_lib_buffer_write_byte_at(core, blob, (u32)index, result));
            PINTA_RETURN();
        }
    }

    PINTA_THROW(PINTA_EXCEPTION_NOT_IMPLEMENTED);

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_buffer_set_member(PintaCore *core, PintaReference *blob, PintaReference *name, PintaReference *value, u8 *is_accessor, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    i32 index;

    PINTA_UNUSED(result);

    pinta_assert(core != NULL);
    pinta_assert(blob != NULL);
    pinta_assert(name != NULL);
    pinta_assert(result != NULL);

    if (name->reference != NULL && name->reference->block_kind == PINTA_KIND_INTEGER)
    {
        index = pinta_integer_ref_get_value(name);
        if (index >= 0)
        {
            if (is_accessor)
                *is_accessor = 0;

            PINTA_CHECK(pinta_lib_buffer_read_byte_at(core, blob, (u32)index, value));
            PINTA_RETURN();
        }
    }

    PINTA_THROW(PINTA_EXCEPTION_NOT_IMPLEMENTED);

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_buffer_get_length(PintaCore *core, PintaReference *buffer, u32 *result)
{
    PintaException exception = PINTA_OK;

    pinta_assert(core != NULL);
    pinta_assert(buffer != NULL);
    pinta_assert(result != NULL);

    if (buffer->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (buffer->reference->block_kind != PINTA_KIND_BUFFER)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    *result = pinta_buffer_ref_get_length(buffer);

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_buffer_get_position(PintaCore *core, PintaReference *buffer, PintaReference *result)
{
    PintaException exception = PINTA_OK;

    pinta_assert(core != NULL);
    pinta_assert(buffer != NULL);
    pinta_assert(result != NULL);

    if (buffer->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (buffer->reference->block_kind != PINTA_KIND_BUFFER)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    PINTA_CHECK(pinta_lib_integer_alloc_value(core, (i32)pinta_buffer_ref_get_position(buffer), result));

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_buffer_get_blob(PintaCore *core, PintaReference *buffer, PintaReference *result)
{
    PintaException exception = PINTA_OK;

    PINTA_UNUSED(core);

    pinta_assert(core != NULL);
    pinta_assert(buffer != NULL);
    pinta_assert(result != NULL);

    if (buffer->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (buffer->reference->block_kind != PINTA_KIND_BUFFER)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    result->reference = pinta_buffer_ref_get_blob(buffer);

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_buffer_set_length_integer(PintaCore *core, PintaReference *buffer, u32 length)
{
    PintaException exception = PINTA_OK;
    u32 current_length;
    u32 blob_length;
    u32 current_position;
    PintaReference blob;

    pinta_assert(core != NULL);
    pinta_assert(buffer != NULL);

    PINTA_GC_ENTER(core, blob);

    if (buffer->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (buffer->reference->block_kind != PINTA_KIND_BUFFER)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    current_length = pinta_buffer_ref_get_length(buffer);

    blob.reference = pinta_buffer_ref_get_blob(buffer);

    if (length < current_length && blob.reference != NULL)
    {
        pinta_buffer_ref_set_length(buffer, length);

        current_position = pinta_buffer_ref_get_position(buffer);
        if (current_position > length)
            pinta_buffer_ref_set_position(buffer, length);
    }
    else
    {
        if (blob.reference == NULL)
            blob_length = 0;
        else
            blob_length = pinta_blob_ref_get_length(&blob);

        if (blob_length < length || blob.reference == NULL)
        {
            blob_length = (length + sizeof(PintaHeapObject));
            if (blob.reference == NULL)
                PINTA_CHECK(pinta_lib_blob_alloc(core, blob_length, &blob));
            else
                PINTA_CHECK(pinta_lib_blob_resize(core, &blob, blob_length, &blob));

            pinta_buffer_ref_set_blob(buffer, &blob);
        }

        pinta_buffer_ref_set_length(buffer, length);
    }

    PINTA_GC_RETURN(core);
}

PintaException pinta_lib_buffer_set_length_position(PintaCore *core, PintaReference *buffer)
{
    PintaException exception = PINTA_OK;
    u32 position;
    u32 length;

    pinta_assert(core != NULL);
    pinta_assert(buffer != NULL);

    if (buffer->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (buffer->reference->block_kind != PINTA_KIND_BUFFER)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    position = pinta_buffer_ref_get_position(buffer);
    length = pinta_buffer_ref_get_length(buffer);

    if (position != length)
        PINTA_CHECK(pinta_lib_buffer_set_length_integer(core, buffer, position));

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_buffer_set_length(PintaCore *core, PintaReference *buffer, PintaReference *value)
{
    PintaException exception = PINTA_OK;
    i32 length;

    pinta_assert(core != NULL);
    pinta_assert(buffer != NULL);
    pinta_assert(value != NULL);

    if (buffer->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (buffer->reference->block_kind != PINTA_KIND_BUFFER)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    if (value->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (value->reference->block_kind != PINTA_KIND_INTEGER)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    length = pinta_integer_ref_get_value(value);
    if (length < 0)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

    PINTA_CHECK(pinta_lib_buffer_set_length_integer(core, buffer, (u32)length));

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_buffer_set_position(PintaCore *core, PintaReference *buffer, PintaReference *value)
{
    PintaException exception = PINTA_OK;
    i32 position;

    PINTA_UNUSED(core);

    pinta_assert(core != NULL);
    pinta_assert(buffer != NULL);
    pinta_assert(value != NULL);


    if (buffer->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (buffer->reference->block_kind != PINTA_KIND_BUFFER)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    if (value->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (value->reference->block_kind != PINTA_KIND_INTEGER)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    position = pinta_integer_ref_get_value(value);
    if (position < 0)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

    pinta_buffer_ref_set_position(buffer, (u32)position);

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_buffer_ensure_write(PintaCore *core, PintaReference *buffer, u32 write_length)
{
    PintaException exception = PINTA_OK;
    u32 length;
    u32 position;
    u32 new_length;
    PintaHeapObject *blob;

    pinta_assert(core != NULL);
    pinta_assert(buffer != NULL);
    pinta_assert(buffer->reference != NULL);
    pinta_assert(buffer->reference->block_kind == PINTA_KIND_BUFFER);

    length = pinta_buffer_ref_get_length(buffer);
    position = pinta_buffer_ref_get_position(buffer);
    blob = pinta_buffer_ref_get_blob(buffer);

    new_length = position + write_length;
    if (blob == NULL || new_length > length)
        PINTA_CHECK(pinta_lib_buffer_set_length_integer(core, buffer, new_length));

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_buffer_write_byte_value(PintaCore *core, PintaReference *buffer, u8 value)
{
    PintaException exception = PINTA_OK;
    u32 position;
    PintaHeapObject *blob;

    pinta_assert(core != NULL);
    pinta_assert(buffer != NULL);

    if (buffer->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (buffer->reference->block_kind != PINTA_KIND_BUFFER)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    PINTA_CHECK(pinta_lib_buffer_ensure_write(core, buffer, 1));

    position = pinta_buffer_ref_get_position(buffer);
    blob = pinta_buffer_ref_get_blob(buffer);

    pinta_assert(blob != NULL);

    PINTA_CHECK(pinta_blob_write_byte_value(blob, position, value));
    pinta_buffer_ref_set_position(buffer, position + 1);

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_buffer_write_byte_at(PintaCore *core, PintaReference *buffer, u32 position, PintaReference *value)
{
    PintaException exception = PINTA_OK;
    PintaHeapObject *blob;

    pinta_assert(core != NULL);
    pinta_assert(buffer != NULL);
    pinta_assert(value != NULL);

    if (buffer->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (buffer->reference->block_kind != PINTA_KIND_BUFFER)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    PINTA_CHECK(pinta_lib_buffer_ensure_write(core, buffer, 1));

    blob = pinta_buffer_ref_get_blob(buffer);

    pinta_assert(blob != NULL);

    PINTA_CHECK(pinta_blob_write_byte(blob, position, value));

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_buffer_write_byte(PintaCore *core, PintaReference *buffer, PintaReference *value)
{
    PintaException exception = PINTA_OK;
    u32 position;
    PintaHeapObject *blob;

    pinta_assert(core != NULL);
    pinta_assert(buffer != NULL);
    pinta_assert(value != NULL);

    if (buffer->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (buffer->reference->block_kind != PINTA_KIND_BUFFER)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    PINTA_CHECK(pinta_lib_buffer_ensure_write(core, buffer, 1));

    position = pinta_buffer_ref_get_position(buffer);
    blob = pinta_buffer_ref_get_blob(buffer);

    pinta_assert(blob != NULL);

    PINTA_CHECK(pinta_blob_write_byte(blob, position, value));
    pinta_buffer_ref_set_position(buffer, position + 1);

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_buffer_write_short_value(PintaCore *core, PintaReference *buffer, u16 value)
{
    PintaException exception = PINTA_OK;
    u32 position;
    PintaHeapObject *blob;

    pinta_assert(core != NULL);
    pinta_assert(buffer != NULL);

    if (buffer->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (buffer->reference->block_kind != PINTA_KIND_BUFFER)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    PINTA_CHECK(pinta_lib_buffer_ensure_write(core, buffer, 2));

    position = pinta_buffer_ref_get_position(buffer);
    blob = pinta_buffer_ref_get_blob(buffer);

    pinta_assert(blob != NULL);

    PINTA_CHECK(pinta_blob_write_short_value(blob, position, value));
    pinta_buffer_ref_set_position(buffer, position + 2);

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_buffer_write_short(PintaCore *core, PintaReference *buffer, PintaReference *value)
{
    PintaException exception = PINTA_OK;
    u32 position;
    PintaHeapObject *blob;

    pinta_assert(core != NULL);
    pinta_assert(buffer != NULL);
    pinta_assert(value != NULL);

    if (buffer->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (buffer->reference->block_kind != PINTA_KIND_BUFFER)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    PINTA_CHECK(pinta_lib_buffer_ensure_write(core, buffer, 2));

    position = pinta_buffer_ref_get_position(buffer);
    blob = pinta_buffer_ref_get_blob(buffer);

    pinta_assert(blob != NULL);

    PINTA_CHECK(pinta_blob_write_short(blob, position, value));
    pinta_buffer_ref_set_position(buffer, position + 2);

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_buffer_write_char_value(PintaCore *core, PintaReference *buffer, wchar value)
{
    PintaException exception = PINTA_OK;
    u32 position;
    PintaHeapObject *blob;

    pinta_assert(core != NULL);
    pinta_assert(buffer != NULL);

    if (buffer->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (buffer->reference->block_kind != PINTA_KIND_BUFFER)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    PINTA_CHECK(pinta_lib_buffer_ensure_write(core, buffer, sizeof(wchar)));

    position = pinta_buffer_ref_get_position(buffer);
    blob = pinta_buffer_ref_get_blob(buffer);

    pinta_assert(blob != NULL);

    PINTA_CHECK(pinta_blob_write_char_value(blob, position, value));
    pinta_buffer_ref_set_position(buffer, position + sizeof(wchar));

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_buffer_write_char(PintaCore *core, PintaReference *buffer, PintaReference *value)
{
    PintaException exception = PINTA_OK;
    u32 position;
    PintaHeapObject *blob;

    pinta_assert(core != NULL);
    pinta_assert(buffer != NULL);
    pinta_assert(value != NULL);

    if (buffer->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (buffer->reference->block_kind != PINTA_KIND_BUFFER)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    PINTA_CHECK(pinta_lib_buffer_ensure_write(core, buffer, sizeof(wchar)));

    position = pinta_buffer_ref_get_position(buffer);
    blob = pinta_buffer_ref_get_blob(buffer);

    pinta_assert(blob != NULL);

    PINTA_CHECK(pinta_blob_write_char(blob, position, value));
    pinta_buffer_ref_set_position(buffer, position + sizeof(wchar));

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_buffer_write_integer_value(PintaCore *core, PintaReference *buffer, u32 value)
{
    PintaException exception = PINTA_OK;
    u32 position;
    PintaHeapObject *blob;

    pinta_assert(core != NULL);
    pinta_assert(buffer != NULL);

    if (buffer->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (buffer->reference->block_kind != PINTA_KIND_BUFFER)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    PINTA_CHECK(pinta_lib_buffer_ensure_write(core, buffer, 2));

    position = pinta_buffer_ref_get_position(buffer);
    blob = pinta_buffer_ref_get_blob(buffer);

    pinta_assert(blob != NULL);

    PINTA_CHECK(pinta_blob_write_integer_value(blob, position, value));
    pinta_buffer_ref_set_position(buffer, position + 4);

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_buffer_write_string(PintaCore *core, PintaReference *buffer, u32 offset, u32 length, PintaReference *value)
{
    PintaException exception = PINTA_OK;
    u32 source_length;
    u32 position;
    u32 substring_offset;
    u32 item_length;
    u32 index;
    PintaType *type;
    struct
    {
        PintaReference blob;
        PintaReference item;
    } gc;

    pinta_assert(core != NULL);
    pinta_assert(buffer != NULL);
    pinta_assert(value != NULL);

    PINTA_GC_ENTER(core, gc);

    if (buffer->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (buffer->reference->block_kind != PINTA_KIND_BUFFER)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    if (value->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    type = pinta_core_get_type(core, value);
    if (!type->is_string)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    if (length == 0)
        PINTA_RETURN();

    switch (value->reference->block_kind)
    {
    case PINTA_KIND_STRING:

        source_length = pinta_string_ref_get_length(value);
        if (offset >= source_length)
            PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

        if (length > source_length)
            PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

        if ((offset + length) > source_length)
            PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

        PINTA_CHECK(pinta_lib_buffer_ensure_write(core, buffer, length * sizeof(wchar)));

        position = pinta_buffer_ref_get_position(buffer);
        gc.blob.reference = pinta_buffer_ref_get_blob(buffer);

        pinta_assert(gc.blob.reference != NULL);

        PINTA_CHECK(pinta_blob_write_string(gc.blob.reference, position, offset, length, value->reference));
        pinta_buffer_ref_set_position(buffer, position + length * sizeof(wchar));

        break;

    case PINTA_KIND_SUBSTRING:
        source_length = pinta_substring_ref_get_length(value);
        if (offset >= source_length)
            PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

        if (length > source_length)
            PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

        if ((offset + length) > source_length)
            PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

        PINTA_CHECK(pinta_lib_buffer_ensure_write(core, buffer, length * sizeof(wchar)));

        position = pinta_buffer_ref_get_position(buffer);
        gc.blob.reference = pinta_buffer_ref_get_blob(buffer);

        pinta_assert(gc.blob.reference != NULL);

        substring_offset = pinta_substring_ref_get_offset(value);
        gc.item.reference = pinta_substring_ref_get_value(value);

        PINTA_CHECK(pinta_blob_write_string(gc.blob.reference, position, substring_offset + offset, length, gc.item.reference));
        pinta_buffer_ref_set_position(buffer, position + length * sizeof(wchar));

        break;

    case PINTA_KIND_MULTISTRING:
        source_length = pinta_multistring_ref_get_length(value);
        if (offset >= source_length)
            PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

        if (length > source_length)
            PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

        if ((offset + length) > source_length)
            PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

        PINTA_CHECK(pinta_lib_buffer_ensure_write(core, buffer, length * sizeof(wchar)));

        for (index = 0; index < PINTA_MULTISTRING_COUNT; index++)
        {
            if (length == 0)
                break;

            gc.item.reference = pinta_multistring_ref_get_item(value, index);
            if (gc.item.reference == NULL)
                continue;

            if (gc.item.reference->block_kind == PINTA_KIND_STRING)
            {
                item_length = pinta_string_ref_get_length(&gc.item);
                if (item_length <= offset)
                {
                    offset -= item_length;
                }
                else
                {
                    item_length -= offset;
                    if (item_length > length)
                    {
                        item_length = length;
                        length = 0;
                    }
                    else
                    {
                        length -= item_length;
                    }

                    PINTA_CHECK(pinta_lib_buffer_write_string(core, buffer, offset, item_length, &gc.item));
                    offset = 0;
                }
            }
            else if (gc.item.reference->block_kind == PINTA_KIND_SUBSTRING)
            {
                item_length = pinta_substring_ref_get_length(&gc.item);

                if (item_length <= offset)
                {
                    offset -= item_length;
                }
                else
                {
                    item_length -= offset;
                    if (item_length > length)
                    {
                        item_length = length;
                        length = 0;
                    }
                    else
                    {
                        length -= item_length;
                    }

                    PINTA_CHECK(pinta_lib_buffer_write_string(core, buffer, offset, item_length, &gc.item));
                    offset = 0;
                }
            }
            else if (gc.item.reference->block_kind == PINTA_KIND_CHAR)
            {
                PINTA_CHECK(pinta_lib_buffer_write_char(core, buffer, &gc.item));
            }
            else
            {
                pinta_assert(0 && "Multistring supports only string and substring, when changed correct this code");
                PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);
            }
        }

        break;

    case PINTA_KIND_CHAR:

        PINTA_CHECK(pinta_lib_buffer_write_char(core, buffer, value));

        break;

    default:
        PINTA_THROW(PINTA_EXCEPTION_ENGINE);
    }

    PINTA_GC_RETURN(core);
}

PintaException pinta_lib_buffer_write_buffer(PintaCore *core, PintaReference *buffer, u32 offset, u32 length, PintaReference *value)
{
    PintaException exception = PINTA_OK;
    u32 destination_position;
    u32 source_length;
    PintaHeapObject *blob;
    PintaHeapObject *item;

    pinta_assert(core != NULL);
    pinta_assert(buffer != NULL);
    pinta_assert(value != NULL);

    if (buffer->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (buffer->reference->block_kind != PINTA_KIND_BUFFER)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    if (value->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (value->reference->block_kind != PINTA_KIND_BUFFER)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    if (length == 0)
        PINTA_RETURN();

    source_length = pinta_buffer_ref_get_length(value);

    if (offset >= source_length)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

    if (length > source_length)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

    if ((offset + length) > source_length)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

    PINTA_CHECK(pinta_lib_buffer_ensure_write(core, buffer, length));

    destination_position = pinta_buffer_ref_get_position(buffer);

    blob = pinta_buffer_ref_get_blob(buffer);
    item = pinta_buffer_ref_get_blob(value);

    if (item != NULL)
    {
        PINTA_CHECK(pinta_blob_write_blob(blob, destination_position, offset, length, item));
        pinta_buffer_ref_set_position(buffer, destination_position + length);
    }

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_buffer_write_blob(PintaCore *core, PintaReference *buffer, u32 offset, u32 length, PintaReference *value)
{
    PintaException exception = PINTA_OK;
    u32 destination_position;
    u32 source_length;
    PintaHeapObject *blob;
    PintaHeapObject *item;

    pinta_assert(core != NULL);
    pinta_assert(buffer != NULL);
    pinta_assert(value != NULL);

    if (buffer->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (buffer->reference->block_kind != PINTA_KIND_BUFFER)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    if (value->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (value->reference->block_kind != PINTA_KIND_BLOB)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    source_length = pinta_blob_ref_get_length(value);

    if (offset >= source_length)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

    if (length > source_length)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

    if ((offset + length) > source_length)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

    PINTA_CHECK(pinta_lib_buffer_ensure_write(core, buffer, length));

    destination_position = pinta_buffer_ref_get_position(buffer);

    blob = pinta_buffer_ref_get_blob(buffer);
    item = value->reference;

    if (item != NULL)
    {
        PINTA_CHECK(pinta_blob_write_blob(blob, destination_position, offset, length, item));
        pinta_buffer_ref_set_position(buffer, destination_position + length);
    }

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_buffer_write_data(PintaCore *core, PintaReference *buffer, u8 *data, u32 data_length)
{
    PintaException exception = PINTA_OK;
    u32 destination_position;
    PintaHeapObject *blob;

    pinta_assert(core != NULL);
    pinta_assert(buffer != NULL);

    if (buffer->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (buffer->reference->block_kind != PINTA_KIND_BUFFER)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    if (data == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    PINTA_CHECK(pinta_lib_buffer_ensure_write(core, buffer, data_length));

    destination_position = pinta_buffer_ref_get_position(buffer);

    blob = pinta_buffer_ref_get_blob(buffer);

    PINTA_CHECK(pinta_blob_write_data(blob, destination_position, data, data_length));
    pinta_buffer_ref_set_position(buffer, destination_position + data_length);

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_buffer_write_special(PintaCore *core, PintaReference *buffer, PintaReference *value)
{
    PintaException exception = PINTA_OK;
    u32 index;
    u32 length;
    PintaReference item;

    pinta_assert(core != NULL);
    pinta_assert(buffer != NULL);
    pinta_assert(value != NULL);

    if (buffer->reference == NULL)
        return (PINTA_EXCEPTION_NULL_REFERENCE);

    if (buffer->reference->block_kind != PINTA_KIND_BUFFER)
        return (PINTA_EXCEPTION_TYPE_MISMATCH);

    if (value->reference == NULL)
        return (PINTA_EXCEPTION_NULL_REFERENCE);

    PINTA_GC_ENTER(core, item);

    switch (value->reference->block_kind)
    {
    case PINTA_KIND_MULTISTRING:

        for (index = 0; index < PINTA_MULTISTRING_COUNT; index++)
        {
            item.reference = pinta_multistring_ref_get_item(value, index);
            if (item.reference == NULL)
                continue;

            PINTA_CHECK(pinta_lib_buffer_write(core, buffer, &item));
        }
        break;
    case PINTA_KIND_ARRAY:

        length = pinta_array_ref_get_length(value);

        for (index = 0; index < length; index++)
        {
            item.reference = pinta_array_ref_get_item(value, index);
            if (item.reference == NULL)
                continue;

            PINTA_CHECK(pinta_lib_buffer_write(core, buffer, &item));
        }
        break;
    case PINTA_KIND_WEAK:

        item.reference = pinta_weak_ref_get_target(value);
        if (item.reference != NULL)
            PINTA_CHECK(pinta_lib_buffer_write(core, buffer, &item));
        break;
    default:
        PINTA_THROW(PINTA_EXCEPTION_INVALID_ARGUMENTS);
    }

    PINTA_GC_RETURN(core);
}

PintaException pinta_lib_buffer_write(PintaCore *core, PintaReference *buffer, PintaReference *value)
{
    PintaException exception = PINTA_OK;
    u32 position;
    u32 write_length;
    u32 offset;
    PintaHeapObject *blob;
    PintaHeapObject *item;

    pinta_assert(core != NULL);
    pinta_assert(buffer != NULL);
    pinta_assert(value != NULL);

    if (buffer->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (buffer->reference->block_kind != PINTA_KIND_BUFFER)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    if (value->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    switch (value->reference->block_kind)
    {
    case PINTA_KIND_INTEGER:
        PINTA_CHECK(pinta_lib_buffer_ensure_write(core, buffer, 4));

        position = pinta_buffer_ref_get_position(buffer);
        blob = pinta_buffer_ref_get_blob(buffer);

        pinta_assert(blob != NULL);

        PINTA_CHECK(pinta_blob_write_integer(blob, position, value));
        pinta_buffer_ref_set_position(buffer, position + 4);

        break;

    case PINTA_KIND_STRING:

        write_length = pinta_string_ref_get_length(value);
        if (write_length > 0)
        {
            PINTA_CHECK(pinta_lib_buffer_ensure_write(core, buffer, write_length * sizeof(wchar)));

            position = pinta_buffer_ref_get_position(buffer);
            blob = pinta_buffer_ref_get_blob(buffer);

            pinta_assert(blob != NULL);

            PINTA_CHECK(pinta_blob_write_string(blob, position, 0, write_length, value->reference));
            pinta_buffer_ref_set_position(buffer, position + write_length * sizeof(wchar));
        }
        break;

    case PINTA_KIND_SUBSTRING:

        write_length = pinta_substring_ref_get_length(value);
        if (write_length)
        {
            PINTA_CHECK(pinta_lib_buffer_ensure_write(core, buffer, write_length * sizeof(wchar)));

            position = pinta_buffer_ref_get_position(buffer);
            blob = pinta_buffer_ref_get_blob(buffer);

            pinta_assert(blob != NULL);

            offset = pinta_substring_ref_get_offset(value);
            item = pinta_substring_ref_get_value(value);

            PINTA_CHECK(pinta_blob_write_string(blob, position, offset, write_length, item));
            pinta_buffer_ref_set_position(buffer, position + write_length * sizeof(wchar));
        }
        break;

    case PINTA_KIND_CHAR:

        PINTA_CHECK(pinta_lib_buffer_ensure_write(core, buffer, sizeof(wchar)));

        position = pinta_buffer_ref_get_position(buffer);
        blob = pinta_buffer_ref_get_blob(buffer);

        pinta_assert(blob != NULL);

        PINTA_CHECK(pinta_blob_write_char(blob, position, value));
        pinta_buffer_ref_set_position(buffer, position + sizeof(wchar));

        break;

    case PINTA_KIND_BUFFER:

        write_length = pinta_buffer_ref_get_length(value);
        if (write_length)
            PINTA_CHECK(pinta_lib_buffer_write_buffer(core, buffer, 0, write_length, value));

        break;

    case PINTA_KIND_BLOB:

        write_length = pinta_blob_ref_get_length(value);
        if (write_length > 0)
            PINTA_CHECK(pinta_lib_buffer_write_blob(core, buffer, 0, write_length, value));

        break;

    case PINTA_KIND_MULTISTRING:
    case PINTA_KIND_ARRAY:
    case PINTA_KIND_WEAK:

        PINTA_CHECK(pinta_lib_buffer_write_special(core, buffer, value));
        break;

    default:
        if (value->reference->block_kind < PINTA_KIND_LENGTH)
            PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

        PINTA_THROW(PINTA_EXCEPTION_ENGINE);
        break;
    }

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_buffer_read_byte_at(PintaCore *core, PintaReference *buffer, u32 position, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    u32 length;
    i32 value;
    PintaHeapObject *blob;

    pinta_assert(core != NULL);
    pinta_assert(buffer != NULL);
    pinta_assert(result != NULL);

    if (buffer->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (buffer->reference->block_kind != PINTA_KIND_BUFFER)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    blob = pinta_buffer_ref_get_blob(buffer);

    if (blob == NULL)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

    length = pinta_buffer_ref_get_length(buffer);

    if ((position + 1) > length)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

    PINTA_CHECK(pinta_blob_read_byte(blob, position, &value));
    PINTA_CHECK(pinta_lib_integer_alloc_value(core, value, result));

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_buffer_read_byte(PintaCore *core, PintaReference *buffer, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    u32 position;
    u32 length;
    i32 value;
    PintaHeapObject *blob;

    pinta_assert(core != NULL);
    pinta_assert(buffer != NULL);
    pinta_assert(result != NULL);

    if (buffer->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (buffer->reference->block_kind != PINTA_KIND_BUFFER)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    blob = pinta_buffer_ref_get_blob(buffer);

    if (blob == NULL)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

    position = pinta_buffer_ref_get_position(buffer);
    length = pinta_buffer_ref_get_length(buffer);

    if ((position + 1) > length)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

    PINTA_CHECK(pinta_blob_read_byte(blob, position, &value));

    pinta_buffer_ref_set_position(buffer, position + 1);

    PINTA_CHECK(pinta_lib_integer_alloc_value(core, value, result));
PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_buffer_read_short(PintaCore *core, PintaReference *buffer, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    u32 position;
    u32 length;
    i32 value;
    PintaHeapObject *blob;

    pinta_assert(core != NULL);
    pinta_assert(buffer != NULL);
    pinta_assert(result != NULL);

    if (buffer->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (buffer->reference->block_kind != PINTA_KIND_BUFFER)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    blob = pinta_buffer_ref_get_blob(buffer);

    if (blob == NULL)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

    position = pinta_buffer_ref_get_position(buffer);
    length = pinta_buffer_ref_get_length(buffer);

    if ((position + 2) > length)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

    PINTA_CHECK(pinta_blob_read_short(blob, position, &value));

    pinta_buffer_ref_set_position(buffer, position + 2);

    PINTA_CHECK(pinta_lib_integer_alloc_value(core, value, result));
PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_buffer_read_integer(PintaCore *core, PintaReference *buffer, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    u32 position;
    u32 length;
    i32 value;
    PintaHeapObject *blob;

    pinta_assert(core != NULL);
    pinta_assert(buffer != NULL);
    pinta_assert(result != NULL);

    if (buffer->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (buffer->reference->block_kind != PINTA_KIND_BUFFER)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    blob = pinta_buffer_ref_get_blob(buffer);

    if (blob == NULL)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

    position = pinta_buffer_ref_get_position(buffer);
    length = pinta_buffer_ref_get_length(buffer);

    if ((position + 4) > length)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

    PINTA_CHECK(pinta_blob_read_integer(blob, position, &value));

    pinta_buffer_ref_set_position(buffer, position + 4);

    PINTA_CHECK(pinta_lib_integer_alloc_value(core, value, result));
PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_buffer_read_string(PintaCore *core, PintaReference *buffer, u32 string_length, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    u32 position;
    u32 buffer_length;
    PintaReference blob;

    pinta_assert(core != NULL);
    pinta_assert(buffer != NULL);
    pinta_assert(result != NULL);

    if (buffer->reference == NULL)
        return (PINTA_EXCEPTION_NULL_REFERENCE);

    if (buffer->reference->block_kind != PINTA_KIND_BUFFER)
        return (PINTA_EXCEPTION_TYPE_MISMATCH);

    PINTA_GC_ENTER(core, blob);

    blob.reference = pinta_buffer_ref_get_blob(buffer);

    if (blob.reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

    position = pinta_buffer_ref_get_position(buffer);
    buffer_length = pinta_buffer_ref_get_length(buffer);

    if ((position + string_length) > buffer_length)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

    PINTA_CHECK(pinta_lib_blob_read_string_integer(core, &blob, position, string_length, result));

    pinta_buffer_ref_set_position(buffer, position + string_length);

    PINTA_GC_RETURN(core);
}

PintaException pinta_lib_buffer_read_blob(PintaCore *core, PintaReference *buffer, u32 data_length, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    u32 position;
    u32 buffer_length;
    PintaReference blob;

    pinta_assert(core != NULL);
    pinta_assert(buffer != NULL);
    pinta_assert(result != NULL);

    if (buffer->reference == NULL)
        return (PINTA_EXCEPTION_NULL_REFERENCE);

    if (buffer->reference->block_kind != PINTA_KIND_BUFFER)
        return (PINTA_EXCEPTION_TYPE_MISMATCH);

    PINTA_GC_ENTER(core, blob);

    blob.reference = pinta_buffer_ref_get_blob(buffer);

    if (blob.reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

    position = pinta_buffer_ref_get_position(buffer);
    buffer_length = pinta_buffer_ref_get_length(buffer);

    if ((position + data_length) > buffer_length)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

    PINTA_CHECK(pinta_lib_blob_read_blob_integer(core, &blob, position, data_length, result));

    pinta_buffer_ref_set_position(buffer, position + data_length);

    PINTA_GC_RETURN(core);
}

PintaException pinta_lib_buffer_read_buffer(PintaCore *core, PintaReference *buffer, u32 data_length, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    struct
    {
        PintaReference buffer_result;
        PintaReference blob;
    } gc;

    pinta_assert(core != NULL);
    pinta_assert(buffer != NULL);

    if (buffer->reference == NULL)
        return (PINTA_EXCEPTION_NULL_REFERENCE);

    if (buffer->reference->block_kind != PINTA_KIND_BUFFER)
        return (PINTA_EXCEPTION_TYPE_MISMATCH);

    PINTA_GC_ENTER(core, gc);

    PINTA_CHECK(pinta_lib_buffer_alloc(core, &gc.buffer_result));
    PINTA_CHECK(pinta_lib_buffer_read_blob(core, buffer, data_length, &gc.blob));

    pinta_buffer_ref_set_length(&gc.buffer_result, data_length);
    pinta_buffer_ref_set_position(&gc.buffer_result, 0);
    pinta_buffer_ref_set_blob(&gc.buffer_result, &gc.blob);

    result->reference = gc.buffer_result.reference;
    PINTA_GC_RETURN(core);
}

PintaException pinta_lib_buffer_in_place_to_string(PintaCore *core, PintaReference *buffer, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    u32 length;
    struct
    {
        PintaReference blob;
        PintaReference value;
    } gc;

    pinta_assert(core != NULL);
    pinta_assert(buffer != NULL);
    pinta_assert(result != NULL);

    if (buffer->reference == NULL)
        return (PINTA_EXCEPTION_NULL_REFERENCE);

    if (buffer->reference->block_kind != PINTA_KIND_BUFFER)
        return (PINTA_EXCEPTION_TYPE_MISMATCH);

    PINTA_GC_ENTER(core, gc);

    length = pinta_buffer_ref_get_length(buffer);
    gc.blob.reference = pinta_buffer_ref_get_blob(buffer);

    if (gc.blob.reference != NULL)
        PINTA_CHECK(pinta_lib_blob_in_place_to_string(core, &gc.blob, length, &gc.value));
    else
        PINTA_CHECK(pinta_lib_string_alloc(core, 0, &gc.value));

    pinta_buffer_ref_set_length(buffer, 0);
    pinta_buffer_ref_set_position(buffer, 0);
    pinta_buffer_ref_set_blob_null(buffer);

    result->reference = gc.value.reference;

    PINTA_GC_RETURN(core);
}

PintaException pinta_lib_buffer_debug_write(PintaCore *core, PintaReference *value, u32 max_depth, PintaJsonWriter *writer)
{
    PintaException exception = PINTA_OK;
    u8 *data = NULL;
    u32 length, blob_length = 0;
    u32 position;
    PintaHeapObject *blob;

    PINTA_UNUSED(max_depth);

    if (value->reference == NULL)
        return pinta_json_write_null(core, writer);

    length = pinta_buffer_ref_get_length(value);
    position = pinta_buffer_ref_get_position(value);
    blob = pinta_buffer_ref_get_blob(value);

    pinta_core_pin(blob);

    if (blob != NULL)
    {
        data = pinta_blob_get_data(blob);
        blob_length = pinta_blob_get_length(blob);

        if (blob_length > length)
            blob_length = length;
    }

    PINTA_CHECK(pinta_json_write_object_start(core, writer));

    PINTA_CHECK(pinta_json_write_property_string(core, writer, type_literal, buffer_literal));
    PINTA_CHECK(pinta_json_write_property_u32(core, writer, length_literal, length));
    PINTA_CHECK(pinta_json_write_property_u32(core, writer, position_literal, position));

    if (data != NULL)
        PINTA_CHECK(pinta_json_write_property_binary(core, writer, data_literal, data, blob_length));
    else
        PINTA_CHECK(pinta_json_write_property_null(core, writer, data_literal));

    PINTA_CHECK(pinta_json_write_end(core, writer));

PINTA_EXIT:
    pinta_core_unpin(blob);
    return exception;
}