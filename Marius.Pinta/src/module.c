#include "pinta.h"

static void pinta_swap_endian(u32* value)
{
    u32 result = *value;
    result = (result >> 24)
        | ((result >> 8) & 0xFF00)
        | ((result << 8) & 0xFF0000)
        | (result << 24);

    *value = result;
}

PintaModule *pinta_module_init(void *module_data, u32 module_length)
{
    PintaException exception = PINTA_OK;
    PintaModule *result = (PintaModule*)module_data;
    u8 *start = (u8*)module_data;
    u8 *end = &start[module_length];

    u8 *data_start;
    u8 *data_end;

    PintaModuleString *strings_start;
    PintaModuleString *strings_current;
    PintaModuleString *strings_end;

    PintaModuleBlob *blobs_start;
    PintaModuleBlob *blobs_current;
    PintaModuleBlob *blobs_end;

    PintaModuleGlobal *globals_start;
    PintaModuleGlobal *globals_current;
    PintaModuleGlobal *globals_end;

    PintaModuleFunction *functions_start;
    PintaModuleFunction *functions_current;
    PintaModuleFunction *functions_end;

    PintaReader reader_value, *reader = &reader_value;
    u8 is_big_endian = 0;

    if (module_length < sizeof(PintaModule))
        return NULL;

    if (result->magic != PINTA_CODE_MODULE_MAGIC)
    {
        u32 big_endian_magic = result->magic;
        pinta_swap_endian(&big_endian_magic);
        if (big_endian_magic != PINTA_CODE_MODULE_MAGIC)
            return NULL;

        is_big_endian = 1;
    }

    if (is_big_endian)
    {
        pinta_swap_endian(&result->magic);
        pinta_swap_endian(&result->flags);

        pinta_swap_endian(&result->strings_length);
        pinta_swap_endian(&result->strings_offset);

        pinta_swap_endian(&result->blobs_length);
        pinta_swap_endian(&result->blobs_offset);

        pinta_swap_endian(&result->globals_length);
        pinta_swap_endian(&result->globals_offset);

        pinta_swap_endian(&result->functions_length);
        pinta_swap_endian(&result->functions_offset);

        pinta_swap_endian(&result->start_function_token);

        pinta_swap_endian(&result->data_length);
        pinta_swap_endian(&result->data_offset);
    }

    data_start = &start[result->data_offset];
    data_end = &data_start[result->data_length];

    if (data_end < data_start)
        return NULL;

    if (data_start < start || data_start > end)
        return NULL;

    if (data_end < start || data_end > end)
        return NULL;

    /* Strings */
    strings_start = (PintaModuleString*)&start[result->strings_offset];
    strings_end = &strings_start[result->strings_length];

    if ((u8*)strings_start < data_start || (u8*)strings_start > data_end)
        return NULL;

    if ((u8*)strings_end < data_start || (u8*)strings_end > data_end)
        return NULL;

    reader->start = data_start;
    reader->end = data_end;
    for (strings_current = strings_start; strings_current < strings_end; strings_current++)
    {
        u32 string_length;
        u8 *string_start;
        u8 *string_end;

        if (is_big_endian)
            pinta_swap_endian(&strings_current->string_offset);

        reader->current = &start[strings_current->string_offset];

        exception = pinta_binary_read_uleb128(reader, NULL, &string_length);
        if (exception != PINTA_OK)
            return NULL;

        string_start = reader->current;
        string_end = &string_start[string_length];

        if ((u8*)string_start < data_start || (u8*)string_start > data_end)
            return NULL;

        if ((u8*)string_end < data_start || (u8*)string_end > data_end)
            return NULL;
    }

    /* Blobs */
    blobs_start = (PintaModuleBlob*)&start[result->blobs_offset];
    blobs_end = &blobs_start[result->blobs_length];

    if ((u8*)blobs_start < data_start || (u8*)blobs_start > data_end)
        return NULL;

    if ((u8*)blobs_end < data_start || (u8*)blobs_end > data_end)
        return NULL;

    for (blobs_current = blobs_start; blobs_current < blobs_end; blobs_current++)
    {
        u32 blob_length;
        u8 *blob_start;
        u8 *blob_end;

        if (is_big_endian)
            pinta_swap_endian(&blobs_current->blob_offset);

        reader->current = &start[blobs_current->blob_offset];

        exception = pinta_binary_read_uleb128(reader, NULL, &blob_length);
        if (exception != PINTA_OK)
            return NULL;

        if (!(result->flags & 0x00010000))
        {
            blob_start = reader->current;
            blob_end = &blob_start[blob_length];

            if ((u8*)blob_start < data_start || (u8*)blob_start > data_end)
                return NULL;

            if ((u8*)blob_end < data_start || (u8*)blob_end > data_end)
                return NULL;
        }
    }

    /* Globals */
    globals_start = (PintaModuleGlobal*)&start[result->globals_offset];
    globals_end = &globals_start[result->globals_length];

    if ((u8*)globals_start < data_start || (u8*)globals_start > data_end)
        return NULL;

    if ((u8*)globals_end < data_start || (u8*)globals_end > data_end)
        return NULL;

    for (globals_current = globals_start; globals_current < globals_end; globals_current++)
    {
        if (is_big_endian)
            pinta_swap_endian(&globals_current->string_token);

        if (globals_current->string_token >= result->strings_length)
            return NULL;
    }

    /* Functions */
    functions_start = (PintaModuleFunction*)&start[result->functions_offset];
    functions_end = &functions_start[result->functions_length];

    if ((u8*)functions_start < data_start || (u8*)functions_start > data_end)
        return NULL;

    if ((u8*)functions_end < data_start || (u8*)functions_end > data_end)
        return NULL;

    for (functions_current = functions_start; functions_current < functions_end; functions_current++)
    {
        u8 *code_start;
        u8 *code_end;

        if (functions_current->string_name_token != PINTA_CODE_TOKEN_EMPTY)
        {
            if (is_big_endian)
                pinta_swap_endian(&functions_current->string_name_token);

            if (functions_current->string_name_token >= result->strings_length)
                return NULL;
        }

        if (is_big_endian)
        {
            pinta_swap_endian(&functions_current->code_offset);
            pinta_swap_endian(&functions_current->code_length);
            pinta_swap_endian(&functions_current->arguments_count);
            pinta_swap_endian(&functions_current->locals_count);
        }

        code_start = &start[functions_current->code_offset];
        code_end = &code_start[functions_current->code_length];

        if ((u8*)code_start < data_start || (u8*)code_start > data_end)
            return NULL;

        if ((u8*)code_end < data_start || (u8*)code_end > data_end)
            return NULL;
    }

    return result;
}

u8 pinta_module_is_compressed_data(PintaModuleDomain *domain)
{
    PintaModule *module;

    module = domain->module;
    if (module->flags & 0x00010000)
        return 1;

    return 0;
}

PintaException pinta_module_get_string(PintaModuleDomain *domain, u32 token, PintaCodeString *result)
{
    PintaException exception = PINTA_OK;
    PintaModule *module;
    PintaModuleString *strings;
    PintaModuleString *current;
    PintaReader reader;
    u32 string_length;
    u8 *start;

    pinta_assert(result != NULL);
    pinta_assert(domain != NULL);
    pinta_assert(domain->module != NULL);

    module = domain->module;
    if (token >= module->strings_length)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

    start = (u8*)module;
    strings = (PintaModuleString*)&start[module->strings_offset];

    current = &strings[token];

    reader.start = &start[module->data_offset];
    reader.end = &reader.start[module->data_length];
    reader.current = &start[current->string_offset];

    PINTA_CHECK(pinta_binary_read_uleb128(&reader, NULL, &string_length));
    result->string_length = string_length;
    result->string_data = reader.current;

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_module_get_blob(PintaModuleDomain *domain, u32 token, PintaCodeBlob *result)
{
    PintaException exception = PINTA_OK;
    PintaModule *module;
    PintaModuleBlob *blobs;
    PintaModuleBlob *current;
    PintaReader reader;
    u32 blob_length;
    u32 blob_offset;
    u8 *start;

    pinta_assert(result != NULL);
    pinta_assert(domain != NULL);
    pinta_assert(domain->module != NULL);

    module = domain->module;
    if (token >= module->blobs_length)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

    start = (u8*)module;
    blobs = (PintaModuleBlob*)&start[module->blobs_offset];

    current = &blobs[token];

    reader.start = &start[module->data_offset];
    reader.end = &reader.start[module->data_length];
    reader.current = &start[current->blob_offset];

    PINTA_CHECK(pinta_binary_read_uleb128(&reader, NULL, &blob_length));
    if (module->flags & 0x00010000)
    {
        PINTA_CHECK(pinta_binary_read_uleb128(&reader, NULL, &blob_offset));

        result->blob_length = blob_length;
        result->blob_data = &start[blob_offset];
    }
    else
    {
        result->blob_length = blob_length;
        result->blob_data = reader.current;
    }

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_module_get_function(PintaModuleDomain *domain, u32 token, PintaCodeFunction *result)
{
    PintaException exception = PINTA_OK;
    PintaModule *module;
    PintaModuleFunction *functions;
    PintaModuleFunction *current;
    u8 *start;

    pinta_assert(result != NULL);
    pinta_assert(domain != NULL);
    pinta_assert(domain->module != NULL);

    module = domain->module;
    if (token >= module->functions_length)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

    start = (u8*)module;
    functions = (PintaModuleFunction*)&start[module->functions_offset];

    current = &functions[token];

    result->string_name_token = current->string_name_token;
    result->arguments_count = current->arguments_count;
    result->locals_count = current->locals_count;

    result->code_start = &start[current->code_offset];
    result->code_end = &result->code_start[current->code_length];

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_module_get_global_name_token(PintaModuleDomain *domain, u32 global_token, u32 *name_token)
{
    PintaException exception = PINTA_OK;
    PintaModule *module;
    PintaModuleGlobal *globals;
    PintaModuleGlobal *current;
    u8 *start;

    pinta_assert(name_token != NULL);
    pinta_assert(domain != NULL);
    pinta_assert(domain->module != NULL);

    module = domain->module;
    if (global_token >= module->globals_length)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

    start = (u8*)module;
    globals = (PintaModuleGlobal*)&start[module->globals_offset];

    current = &globals[global_token];
    *name_token = current->string_token;

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_module_get_global_token(PintaCore *core, PintaModuleDomain *domain, PintaReference *name, u32 *token)
{
    PintaException exception = PINTA_OK;
    u32 globals_length;
    u32 index = 0;
    u32 global_name_token;
    u32 name_length;
    u32 string_length;
    PintaModule *module;
    PintaCodeString global_name;
    PintaType *name_type;
    PintaReference string_value;

    pinta_assert(core != NULL);
    pinta_assert(domain != NULL);
    pinta_assert(domain->module != NULL);
    pinta_assert(name != NULL);
    pinta_assert(name->reference != NULL);

    PINTA_GC_ENTER(core, string_value);

    module = domain->module;

    name_type = pinta_core_get_type(core, name);
    if (!name_type->is_string)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    PINTA_CHECK(pinta_lib_string_to_string(core, name, name));

    name_length = pinta_string_ref_get_length(name);

    if (domain->globals.reference == NULL)
    {
        *token = PINTA_CODE_TOKEN_EMPTY;
        PINTA_RETURN();
    }

    globals_length = pinta_array_ref_get_length(&domain->globals);
    pinta_assert(globals_length == module->globals_length);

    if (module->globals_length < globals_length)
        globals_length = module->globals_length;

    for (index = 0; index < globals_length; index++)
    {
        wchar *name_data;
        wchar *value_data;

        string_value.reference = NULL;

        PINTA_CHECK(pinta_module_get_global_name_token(domain, index, &global_name_token));
        if (domain->strings.reference != NULL)
            PINTA_CHECK(pinta_lib_array_get_item(core, &domain->strings, global_name_token, &string_value));

        if (string_value.reference == NULL)
        {
            PINTA_CHECK(pinta_module_get_string(domain, global_name_token, &global_name));
            if (global_name.string_length != name_length)
                continue;

            PINTA_CHECK(pinta_lib_decode_mutf8(core, global_name.string_data, global_name.string_length, &string_value));
        }
        else
        {
            string_length = pinta_string_ref_get_length(&string_value);
            if (string_length != name_length)
                continue;
        }

        if (domain->strings.reference != NULL)
            PINTA_CHECK(pinta_lib_array_set_item(core, &domain->strings, global_name_token, &string_value));

        name_data = pinta_string_ref_get_data(name);
        value_data = pinta_string_ref_get_data(&string_value);
        if (memcmp(name_data, value_data, name_length * sizeof(wchar)) == 0)
        {
            *token = index;
            break;
        }
    }

    PINTA_GC_RETURN(core);
}

PintaException pinta_lib_module_get_global_value(PintaCore *core, PintaModuleDomain *domain, PintaReference *name, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    u32 token;
    pinta_assert(result != NULL);

    token = PINTA_CODE_TOKEN_EMPTY;
    PINTA_CHECK(pinta_lib_module_get_global_token(core, domain, name, &token));

    if (token != PINTA_CODE_TOKEN_EMPTY)
        PINTA_CHECK(pinta_lib_array_get_item(core, &domain->globals, token, result));
    else
        result->reference = NULL;

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_module_set_global_value(PintaCore *core, PintaModuleDomain *domain, PintaReference *name, PintaReference *value)
{
    PintaException exception = PINTA_OK;
    u32 token;
    pinta_assert(value != NULL);

    token = PINTA_CODE_TOKEN_EMPTY;
    PINTA_CHECK(pinta_lib_module_get_global_token(core, domain, name, &token));

    if (token != PINTA_CODE_TOKEN_EMPTY)
        PINTA_CHECK(pinta_lib_array_set_item(core, &domain->globals, token, value));

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_module_get_string(PintaCore *core, PintaModuleDomain *domain, u32 token, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    PintaCodeString string_value, *string = &string_value;

    pinta_assert(token != PINTA_CODE_TOKEN_EMPTY);

    result->reference = NULL;
    if (domain->strings.reference != NULL)
        PINTA_CHECK(pinta_lib_array_get_item(core, &domain->strings, token, result));

    if (result->reference == NULL)
    {
        PINTA_CHECK(pinta_module_get_string(domain, token, string));
        PINTA_CHECK(pinta_lib_decode_mutf8(core, string->string_data, string->string_length, result));

        if (domain->strings.reference != NULL)
            PINTA_CHECK(pinta_lib_array_set_item(core, &domain->strings, token, result));
    }

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}