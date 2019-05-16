#include "pinta_tests.h"

u8 buffer[8192];
u32 buffer_offset;

PintaException json_write(PintaCore *core, PintaJsonWriter *writer, wchar *value, u32 string_length)
{
    PintaException exception = PINTA_OK;
    u32 index;
    u32 current;
    u32 next;
    u8 *data = &buffer[buffer_offset];
    u8 *end = &buffer[sizeof(buffer)];

    for (index = 0; index < string_length; index++)
    {
        current = (u32)value[index];

        if (current >= 0xD800 && current < 0xDC00)
        {
            if (index + 1 < string_length)
            {
                next = (u32)value[index + 1];
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
            if (data + 1 > end)
                return PINTA_EXCEPTION_INDEX_OUT_OF_BOUNDS;

            *data++ = (u8)current;
        }
        else if (current < 0x0800)
        {
            if (data + 2 > end)
                return PINTA_EXCEPTION_INDEX_OUT_OF_BOUNDS;

            *data++ = (u8)(((current >> 6) & 0x1F) | 0xC0);
            *data++ = (u8)((current & 0x3F) | 0x80);
        }
        else if (current < 0x10000)
        {
            if (data + 3 > end)
                return PINTA_EXCEPTION_INDEX_OUT_OF_BOUNDS;

            *data++ = (u8)(((current >> 12) & 0x0F) | 0xE0);
            *data++ = (u8)(((current >> 6) & 0x3F) | 0x80);
            *data++ = (u8)((current & 0x3F) | 0x80);
        }
        else
        {
            if (data + 4 > end)
                return PINTA_EXCEPTION_INDEX_OUT_OF_BOUNDS;

            *data++ = (u8)(((current >> 18) & 0x07) | 0xF0);
            *data++ = (u8)(((current >> 12) & 0x3F) | 0x80);
            *data++ = (u8)(((current >> 6) & 0x3F) | 0x80);
            *data++ = (u8)((current & 0x3F) | 0x80);
        }
    }

    buffer_offset = (u32)(data - &buffer[0]);

    return exception;
}

PINTA_TEST_BEGIN(json_write_object, 2)
{
    PintaJsonWriter writer_;
    PintaJsonWriter *writer = &writer_;
    PintaReference *local = PINTA_GC_LOCAL(0);

    writer->data = NULL;
    writer->stack = 0;
    writer->write = json_write;

    memset(buffer, 0, sizeof(buffer));
    buffer_offset = 0;

    PINTA_CHECK(pinta_json_write_object_start(core, writer));

    PINTA_CHECK(pinta_json_write_property_i32(core, writer, L"key", 34));
    
    PINTA_CHECK(pinta_json_write_property_name(core, writer, L"value"));
    PINTA_CHECK(pinta_json_write_array_start(core, writer));

    PINTA_CHECK(pinta_json_write_null(core, writer));
    PINTA_CHECK(pinta_json_write_bool(core, writer, 0));
    PINTA_CHECK(pinta_json_write_bool(core, writer, 1));
    PINTA_CHECK(pinta_json_write_string(core, writer, L"string", 6));

    PINTA_CHECK(pinta_json_write_end(core, writer));

    PINTA_CHECK(pinta_json_write_end(core, writer));

    sput_fail_if(memcmp(buffer, "{\"key\": 34, \"value\": [null, false, true, \"string\"]}", buffer_offset), "Incorrect json");
}
PINTA_TEST_END(PINTA_OK)

void pinta_tests_json()
{
    sput_enter_suite("JSON tests");
    sput_run_test(json_write_object);
    sput_leave_suite();
}
