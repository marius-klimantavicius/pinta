#include "pinta_tests.h"

PINTA_TEST_BEGIN(buffer_write_to_empty, 3)
{
    PintaReference *value;
    PintaReference *buffer;
    PintaReference *blob;
    u32 length;
    u32 position;
    u8 *data;

    value = PINTA_GC_LOCAL(0);
    buffer = PINTA_GC_LOCAL(1);
    blob = PINTA_GC_LOCAL(2);

    PINTA_CHECK(pinta_lib_string_alloc_value(core, L"0123456789", 10, value));
    PINTA_CHECK(pinta_lib_buffer_alloc(core, buffer));

    PINTA_CHECK(pinta_lib_buffer_write(core, buffer, value));

    pinta_core_gc(core, 1);

    length = pinta_buffer_ref_get_length(buffer);
    sput_fail_if(length < 10 * sizeof(wchar), "buffer length >= 10 * sizeof(wchar)");

    position = pinta_buffer_ref_get_position(buffer);
    sput_fail_if(position != 10 * sizeof(wchar), "position == 10 * sizeof(wchar)");

    blob->reference = pinta_buffer_ref_get_blob(buffer);
    sput_fail_if(blob->reference == NULL, "blob != NULL");

    length = pinta_blob_ref_get_length(blob);
    sput_fail_if(length < 10 * sizeof(wchar), "blob length >= 10 * sizeof(wchar)");

    data = pinta_blob_ref_get_data(blob);
    sput_fail_if(data == NULL, "data != NULL");

    sput_fail_if(memcmp(data, L"0123456789", 10 * sizeof(wchar)) != 0, "data written correctly");
}
PINTA_TEST_END(PINTA_OK)

PINTA_TEST_BEGIN(buffer_write_resize, 3)
{
    PintaReference *value;
    PintaReference *buffer;
    PintaReference *blob;
    u32 length;
    u32 position;
    u8 *data;

    value = PINTA_GC_LOCAL(0);
    buffer = PINTA_GC_LOCAL(1);
    blob = PINTA_GC_LOCAL(2);

    PINTA_CHECK(pinta_lib_string_alloc_value(core, L"0123456789", 10, value));
    PINTA_CHECK(pinta_lib_buffer_alloc(core, buffer));

    PINTA_CHECK(pinta_lib_buffer_write(core, buffer, value));
    PINTA_CHECK(pinta_lib_buffer_write(core, buffer, value));
    pinta_core_gc(core, 1);

    PINTA_CHECK(pinta_lib_buffer_write(core, buffer, value));
    PINTA_CHECK(pinta_lib_buffer_write(core, buffer, value));
    pinta_core_gc(core, 1);

    length = pinta_buffer_ref_get_length(buffer);
    sput_fail_if(length < 40 * sizeof(wchar), "buffer length >= 40 * sizeof(wchar)");

    position = pinta_buffer_ref_get_position(buffer);
    sput_fail_if(position != 40 * sizeof(wchar), "position == 40 * sizeof(wchar)");

    blob->reference = pinta_buffer_ref_get_blob(buffer);
    sput_fail_if(blob->reference == NULL, "blob != NULL");

    length = pinta_blob_ref_get_length(blob);
    sput_fail_if(length < 40 * sizeof(wchar), "blob length >= 40 * sizeof(wchar)");

    data = pinta_blob_ref_get_data(blob);
    sput_fail_if(data == NULL, "data != NULL");

    sput_fail_if(memcmp(data, L"0123456789012345678901234567890123456789", 40 * sizeof(wchar)) != 0, "data written correctly");
}
PINTA_TEST_END(PINTA_OK)

PINTA_TEST_BEGIN(buffer_write_types, 3)
{
    PintaReference *value;
    PintaReference *buffer;
    PintaReference *blob;
    u32 length;
    u32 position;
    u8 *data;

    value = PINTA_GC_LOCAL(0);
    buffer = PINTA_GC_LOCAL(1);
    blob = PINTA_GC_LOCAL(2);

    PINTA_CHECK(pinta_lib_buffer_alloc(core, buffer));

    PINTA_CHECK(pinta_lib_string_alloc_value(core, L"abcdefghijklmnopqrstuvwxyz", 26, value));
    PINTA_CHECK(pinta_lib_buffer_write(core, buffer, value));
    
    PINTA_CHECK(pinta_lib_integer_alloc_value(core, 0x33, value));
    PINTA_CHECK(pinta_lib_buffer_write_byte(core, buffer, value));
    
    PINTA_CHECK(pinta_lib_integer_alloc_value(core, 0x00, value));
    PINTA_CHECK(pinta_lib_buffer_write_byte(core, buffer, value));

    PINTA_CHECK(pinta_lib_integer_alloc_value(core, 0x0033, value));
    PINTA_CHECK(pinta_lib_buffer_write_short(core, buffer, value));

    PINTA_CHECK(pinta_lib_integer_alloc_value(core, 0x0032, value));
    PINTA_CHECK(pinta_lib_buffer_write_short(core, buffer, value));

    PINTA_CHECK(pinta_lib_integer_alloc_value(core, 0x00320033, value));
    PINTA_CHECK(pinta_lib_buffer_write(core, buffer, value));

    PINTA_CHECK(pinta_lib_integer_alloc_value(core, 0x00300031, value));
    PINTA_CHECK(pinta_lib_buffer_write(core, buffer, value));

    PINTA_CHECK(pinta_lib_string_alloc_value(core, L"abcdefghijklmnopqrstuvwxyz", 26, value));
    PINTA_CHECK(pinta_lib_substring_alloc(core, value, 3, 5, value));
    PINTA_CHECK(pinta_lib_buffer_write(core, buffer, value));

    PINTA_CHECK(pinta_lib_blob_alloc(core, 12 * sizeof(wchar), value));
    data = pinta_blob_ref_get_data(value);
    memcpy(data, L"-9988877766-", 12 * sizeof(wchar));
    PINTA_CHECK(pinta_lib_buffer_write_blob(core, buffer, 2, 20, value));

    length = pinta_buffer_ref_get_length(buffer);
    sput_fail_if(length < 96, "buffer length >= 96");

    position = pinta_buffer_ref_get_position(buffer);
    sput_fail_if(position != 96, "position == 96");

    blob->reference = pinta_buffer_ref_get_blob(buffer);
    sput_fail_if(blob->reference == NULL, "blob != NULL");

    length = pinta_blob_ref_get_length(blob);
    sput_fail_if(length < 96, "blob length >= 96");

    data = pinta_blob_ref_get_data(blob);
    sput_fail_if(data == NULL, "data != NULL");

    sput_fail_if(memcmp(data, L"abcdefghijklmnopqrstuvwxyz3323210defgh9988877766", 96) != 0, "data written correctly");

    PINTA_CHECK(pinta_lib_buffer_in_place_to_string(core, buffer, value));

    sput_fail_if(value->reference == NULL, "string != NULL");
    sput_fail_if(value->reference->block_kind != PINTA_KIND_STRING, "string correct type");
    sput_fail_if(pinta_string_ref_get_length(value) < 48, "string length >= 48");
    sput_fail_if(memcmp(pinta_string_ref_get_data(value), L"abcdefghijklmnopqrstuvwxyz3323210defgh9988877766", pinta_string_ref_get_length(value) * sizeof(wchar)) != 0, "string data correct");
}
PINTA_TEST_END(PINTA_OK)

PINTA_TEST_BEGIN(buffer_sub_buffer, 5)
{
    PintaReference *value;
    PintaReference *buffer;
    PintaReference *blob;
    PintaReference *item;
    PintaReference *sub;
    u32 length;
    u32 position;
    u8 *data;

    value = PINTA_GC_LOCAL(0);
    buffer = PINTA_GC_LOCAL(1);
    blob = PINTA_GC_LOCAL(2);
    item = PINTA_GC_LOCAL(3);
    sub = PINTA_GC_LOCAL(4);

    PINTA_CHECK(pinta_lib_buffer_alloc(core, buffer));
    PINTA_CHECK(pinta_lib_string_alloc_value(core, L"AID:000000", 10, value));

    PINTA_CHECK(pinta_lib_buffer_set_length_integer(core, buffer, 10 * sizeof(wchar)));

    PINTA_CHECK(pinta_lib_integer_alloc_value(core, 4 * sizeof(wchar), item));
    PINTA_CHECK(pinta_lib_buffer_set_position(core, buffer, item));
    PINTA_CHECK(pinta_lib_buffer_write_string(core, buffer, 4, 6, value));

    PINTA_CHECK(pinta_lib_integer_alloc_value(core, 0, item));
    PINTA_CHECK(pinta_lib_buffer_set_position(core, buffer, item));
    PINTA_CHECK(pinta_lib_buffer_write_string(core, buffer, 0, 4, value));

    length = pinta_buffer_ref_get_length(buffer);
    sput_fail_if(length != 10 * sizeof(wchar), "buffer length == 10 * sizeof(wchar)");

    position = pinta_buffer_ref_get_position(buffer);
    sput_fail_if(position != 4 * sizeof(wchar), "position == 4 * sizeof(wchar)");

    blob->reference = pinta_buffer_ref_get_blob(buffer);
    sput_fail_if(blob->reference == NULL, "blob != NULL");

    data = pinta_blob_ref_get_data(blob);
    sput_fail_if(data == NULL, "data != NULL");

    sput_fail_if(memcmp(data, L"AID:000000", 10 * sizeof(wchar)) != 0, "data written correctly");

    PINTA_CHECK(pinta_lib_integer_alloc_value(core, 0, item));
    PINTA_CHECK(pinta_lib_buffer_set_position(core, buffer, item));
    PINTA_CHECK(pinta_lib_buffer_read_buffer(core, buffer, 4 * sizeof(wchar), sub));

    PINTA_CHECK(pinta_lib_integer_alloc_value(core, 4 * sizeof(wchar), item));
    PINTA_CHECK(pinta_lib_buffer_set_position(core, buffer, item));
    PINTA_CHECK(pinta_lib_buffer_read_buffer(core, buffer, 6 * sizeof(wchar), item));

    PINTA_CHECK(pinta_lib_buffer_alloc(core, buffer));
    PINTA_CHECK(pinta_lib_buffer_write(core, buffer, sub));
    PINTA_CHECK(pinta_lib_buffer_write(core, buffer, item));

    length = pinta_buffer_ref_get_length(buffer);
    sput_fail_if(length != 10 * sizeof(wchar), "buffer length == 10 * sizeof(wchar)");

    position = pinta_buffer_ref_get_position(buffer);
    sput_fail_if(position != 10 * sizeof(wchar), "position == 10 * sizeof(wchar)");

    blob->reference = pinta_buffer_ref_get_blob(buffer);
    sput_fail_if(blob->reference == NULL, "blob != NULL");

    data = pinta_blob_ref_get_data(blob);
    sput_fail_if(data == NULL, "data != NULL");

    sput_fail_if(memcmp(data, L"AID:000000", 10 * sizeof(wchar)) != 0, "data written correctly");
}
PINTA_TEST_END(PINTA_OK)

PINTA_TEST_BEGIN(buffer_out_of_memory, 2)
{
    PintaReference *buffer;
    PintaReference *string;

    buffer = PINTA_GC_LOCAL(0);
    string = PINTA_GC_LOCAL(1);

    PINTA_CHECK(pinta_lib_buffer_alloc(core, buffer));
    PINTA_CHECK(pinta_lib_string_alloc_value(core, L"Some data to fill buffer", 24, string));

    while (1)
    {
        PINTA_CHECK(pinta_lib_buffer_write(core, buffer, string));
    }
}
PINTA_TEST_END(PINTA_EXCEPTION_OUT_OF_MEMORY)

void pinta_tests_buffer()
{
    sput_enter_suite("Buffer tests");
    sput_run_test(buffer_write_to_empty);
    sput_run_test(buffer_write_resize);
    sput_run_test(buffer_write_types);
    sput_run_test(buffer_sub_buffer);
    sput_run_test(buffer_out_of_memory);
    sput_leave_suite();
}
