#include "pinta.h"

#define PINTA_API_INITIAL_OUTPUT_CAPACITY   2816

PintaApi *pinta_api_create(PintaApiEnvironment *environment)
{
    PintaEnvironment *core_environment;
    PintaApiEnvironment *native_environment;
    PintaNativeMemory *memory;
    PintaApi *api;
    PintaCore *core;

    if (environment == NULL)
        return NULL;

    memory = pinta_memory_init(environment->memory, environment->memory_length);
    if (memory == NULL)
        return NULL;

    api = (PintaApi*)pinta_memory_alloc(memory, sizeof(PintaApi));
    if (api == NULL)
        return NULL;

    native_environment = (PintaApiEnvironment*)pinta_memory_alloc(memory, sizeof(PintaApiEnvironment));
    if (native_environment == NULL)
        return NULL;

    *native_environment = *environment;

    core_environment = (PintaEnvironment*)pinta_memory_alloc(memory, sizeof(PintaEnvironment));
    if (core_environment == NULL)
        return NULL;

    core_environment->native_environment = native_environment;

    core_environment->file_open = pinta_api_core_file_open;
    core_environment->file_size = pinta_api_core_file_size;
    core_environment->file_read = pinta_api_core_file_read;
    core_environment->file_close = pinta_api_core_file_close;
    core_environment->load_module = pinta_api_core_load_module;

    api->core = core = pinta_core_init(memory, environment->heap_length, environment->stack_length);
    if (api->core == NULL)
        return NULL;

    core->environment = core_environment;

    core->internal_functions = (PintaCoreInternalFunction*)pinta_memory_alloc(memory, 1 * sizeof(PintaCoreInternalFunction));
    core->internal_functions_length = 1;

    core->internal_functions[0] = pinta_api_internal_out;

    api->load_module = pinta_api_load_module;
    api->set_string = pinta_api_set_global_string;
    api->set_integer = pinta_api_set_global_integer;
    api->set_null = pinta_api_set_global_null;
    api->execute = pinta_api_execute_module;

    api->unsafe_get_string = pinta_api_get_global_string;
    api->unsafe_get_output_buffer = pinta_api_get_output_buffer;
    api->unsafe_get_output_string = pinta_api_get_output_string;

    return api;
}

uint32_t pinta_api_util_set_string_c(PintaApi *api, void *module, char *name, char *value)
{
    PintaApiString name_string, value_string;

    name_string.string_data = name;
    if (name != NULL)
        name_string.string_length = strlen(name);
    else
        name_string.string_length = 0;

    name_string.string_encoding = PINTA_API_ENCODING_C;

    value_string.string_data = value;
    if (value != NULL)
        value_string.string_length = strlen(value);
    else
        value_string.string_length = 0;

    value_string.string_encoding = PINTA_API_ENCODING_C;

    return api->set_string(api, module, &name_string, &value_string);
}

uint32_t pinta_api_util_set_string_utf8(PintaApi *api, void *module, char *name, char *value)
{
    PintaApiString name_string, value_string;

    name_string.string_data = name;
    if (name != NULL)
        name_string.string_length = strlen(name);
    else
        name_string.string_length = 0;

    name_string.string_encoding = PINTA_API_ENCODING_UTF8;

    value_string.string_data = value;
    if (value != NULL)
        value_string.string_length = strlen(value);
    else
        value_string.string_length = 0;

    value_string.string_encoding = PINTA_API_ENCODING_UTF8;

    return api->set_string(api, module, &name_string, &value_string);
}

void *pinta_api_load_module(PintaApi *api, PintaApiString *name)
{
    PintaException exception = PINTA_OK;
    PintaCore *core;
    PintaEnvironment *core_environment;
    PintaApiEnvironment *environment;
    PintaModuleDomain *domain = NULL;
    void *handle = NULL;
    void *name_data;
    uint32_t name_length;
    PintaReference pinta_value;

    if (api == NULL)
        return NULL;

    core = api->core;
    if (core == NULL)
        return NULL;

    core_environment = core->environment;
    if (core_environment == NULL)
        return NULL;

    environment = (PintaApiEnvironment *)core_environment->native_environment;
    if (environment == NULL)
        return NULL;

    PINTA_GC_ENTER(core, pinta_value);

    if (name->string_encoding != environment->platform_encoding)
    {
        PINTA_CHECK(pinta_api_string_decode(core, name, &pinta_value));
        PINTA_CHECK(pinta_api_string_encode(core, &pinta_value, environment->platform_encoding, &name_length, &name_data));
    }
    else
    {
        name_data = name->string_data;
        name_length = name->string_length;
    }

    handle = environment->file_open(environment->environment_context, name_data, name_length);
    if (handle == NULL)
        PINTA_THROW(PINTA_EXCEPTION_FILE_NOT_FOUND);

    PINTA_CHECK(pinta_api_load_module_handle(core, handle, &domain));

PINTA_EXIT:
    PINTA_GC_EXIT(core);

    if (handle != NULL)
        environment->file_close(environment->environment_context, handle);

    return (void*)domain;
}

uint32_t pinta_api_set_global_string(PintaApi *api, void *module, PintaApiString *name, PintaApiString *value)
{
    PintaException exception = PINTA_OK;
    PintaCore *core;

    struct
    {
        PintaReference name;
        PintaReference value;
    } gc;

    if (api == NULL)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_NULL_REFERENCE);

    if (module == NULL)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_NULL_REFERENCE);

    if (name->string_data == NULL || name->string_length == 0)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_INVALID_ARGUMENTS);

    core = api->core;
    if (core == NULL)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_INVALID_OPERATION);

    PINTA_GC_ENTER(core, gc);

    PINTA_CHECK(pinta_api_string_decode(core, name, &gc.name));
    PINTA_CHECK(pinta_api_string_decode(core, value, &gc.value));

    PINTA_CHECK(pinta_lib_module_set_global_value(core, (PintaModuleDomain*)module, &gc.name, &gc.value));

PINTA_EXIT:
    PINTA_GC_EXIT(core);
    return (uint32_t)exception;
}

uint32_t pinta_api_set_global_integer(PintaApi *api, void *module, PintaApiString *name, int32_t value)
{
    PintaException exception = PINTA_OK;
    PintaCore *core;

    struct
    {
        PintaReference name;
        PintaReference value;
    } gc;

    if (api == NULL)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_NULL_REFERENCE);

    if (module == NULL)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_NULL_REFERENCE);

    if (name->string_data == NULL || name->string_length == 0)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_INVALID_ARGUMENTS);

    core = api->core;
    if (core == NULL)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_INVALID_OPERATION);

    PINTA_GC_ENTER(core, gc);

    PINTA_CHECK(pinta_api_string_decode(core, name, &gc.name));
    PINTA_CHECK(pinta_lib_integer_alloc_value(core, value, &gc.value));

    PINTA_CHECK(pinta_lib_module_set_global_value(core, (PintaModuleDomain*)module, &gc.name, &gc.value));

PINTA_EXIT:
    PINTA_GC_EXIT(core);
    return (uint32_t)exception;
}

uint32_t pinta_api_set_global_null(PintaApi *api, void *module, PintaApiString *name)
{
    PintaException exception = PINTA_OK;
    PintaCore *core;

    struct
    {
        PintaReference name;
        PintaReference value;
    } gc;

    if (api == NULL)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_NULL_REFERENCE);

    if (module == NULL)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_NULL_REFERENCE);

    if (name->string_data == NULL || name->string_length == 0)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_INVALID_ARGUMENTS);

    core = api->core;
    if (core == NULL)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_INVALID_OPERATION);

    PINTA_GC_ENTER(core, gc);

    PINTA_CHECK(pinta_api_string_decode(core, name, &gc.name));
    gc.value.reference = NULL;

    PINTA_CHECK(pinta_lib_module_set_global_value(core, (PintaModuleDomain*)module, &gc.name, &gc.value));

PINTA_EXIT:
    PINTA_GC_EXIT(core);
    return (uint32_t)exception;
}

uint32_t pinta_api_execute_module(PintaApi *api, void *module)
{
    PintaException exception = PINTA_OK;
    PintaCore *core;
    PintaEnvironment *core_environment;
    PintaApiEnvironment *environment;
    u32 initial_output_capacity = PINTA_API_INITIAL_OUTPUT_CAPACITY;

    if (api == NULL)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_NULL_REFERENCE);

    if (module == NULL)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_NULL_REFERENCE);

    core = api->core;
    if (core == NULL)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_INVALID_OPERATION);

    core_environment = core->environment;
    if (core_environment != NULL)
    {
        environment = (PintaApiEnvironment *)core_environment->native_environment;
        if (environment != NULL && environment->initial_output_capacity > 0)
            initial_output_capacity = environment->initial_output_capacity;
    }

    PINTA_CHECK(pinta_lib_buffer_alloc_value(core, initial_output_capacity, &core->output));
    PINTA_CHECK(pinta_code_execute_module(core, (PintaModuleDomain*)module));

PINTA_EXIT:
    return (uint32_t)exception;
}

uint32_t pinta_api_get_global_string(PintaApi *api, void *module, PintaApiString *name, PintaApiEncoding encoding, uint32_t *string_length, void **string_data)
{
    PintaException exception = PINTA_OK;
    PintaCore *core;
    PintaType *type;

    struct
    {
        PintaReference name;
        PintaReference value;
    } gc;

    if (api == NULL)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_NULL_REFERENCE);

    if (module == NULL)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_NULL_REFERENCE);

    if (name->string_data == NULL || name->string_length == 0)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_INVALID_ARGUMENTS);

    core = api->core;
    if (core == NULL)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_INVALID_OPERATION);

    PINTA_GC_ENTER(core, gc);

    PINTA_CHECK(pinta_api_string_decode(core, name, &gc.name));
    PINTA_CHECK(pinta_lib_module_get_global_value(core, (PintaModuleDomain*)module, &gc.name, &gc.value));

    if (gc.value.reference == NULL)
    {
        *string_length = 0;
        *string_data = NULL;
        PINTA_RETURN();
    }

    type = pinta_core_get_type(core, &gc.value);
    PINTA_CHECK(type->to_string(core, &gc.value, &gc.value));
    PINTA_CHECK(pinta_lib_string_to_string(core, &gc.value, &gc.value));
    PINTA_CHECK(pinta_api_string_encode(core, &gc.value, encoding, string_length, string_data));

PINTA_EXIT:
    PINTA_GC_EXIT(core);
    return (uint32_t)exception;
}

uint32_t pinta_api_get_output_buffer(PintaApi *api, uint32_t *buffer_length, void **buffer_data)
{
    PintaException exception = PINTA_OK;
    PintaCore *core;
    PintaHeapObject *blob;

    if (api == NULL)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_NULL_REFERENCE);

    if (buffer_length == NULL)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_NULL_REFERENCE);

    if (buffer_data == NULL)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_NULL_REFERENCE);

    core = api->core;
    if (core == NULL)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_INVALID_OPERATION);

    if (core->output.reference == NULL)
    {
        *buffer_length = 0;
        *buffer_data = NULL;
        PINTA_RETURN();
    }

    if (core->output.reference->block_kind != PINTA_KIND_BUFFER)
        PINTA_THROW(PINTA_EXCEPTION_ENGINE);

    blob = pinta_buffer_ref_get_blob(&core->output);
    *buffer_length = pinta_buffer_ref_get_length(&core->output);
    *buffer_data = (void*)pinta_blob_get_data(blob);

PINTA_EXIT:
    return (uint32_t)exception;
}

uint32_t pinta_api_get_output_string(PintaApi *api, PintaApiEncoding encoding, uint32_t *string_length, void **string_data)
{
    PintaException exception = PINTA_OK;
    PintaCore *core;
    struct
    {
        PintaReference buffer;
        PintaReference string;
    } gc;

    if (api == NULL)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_NULL_REFERENCE);

    if (string_length == NULL)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_NULL_REFERENCE);

    if (string_data == NULL)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_NULL_REFERENCE);

    core = api->core;
    if (core == NULL)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_INVALID_OPERATION);

    if (core->output.reference == NULL)
    {
        *string_length = 0;
        *string_data = NULL;
        return PINTA_OK;
    }

    if (core->output.reference->block_kind != PINTA_KIND_BUFFER)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_ENGINE);

    PINTA_GC_ENTER(core, gc);

    PINTA_CHECK(pinta_lib_buffer_alloc(core, &gc.buffer));
    PINTA_CHECK(pinta_lib_buffer_write(core, &gc.buffer, &core->output));
    PINTA_CHECK(pinta_lib_buffer_in_place_to_string(core, &gc.buffer, &gc.string));

    PINTA_CHECK(pinta_api_string_encode(core, &gc.string, encoding, string_length, string_data));

PINTA_EXIT:
    PINTA_GC_EXIT(core);
    return (uint32_t)exception;
}

PintaException pinta_api_read_file(PintaCore *core, void *handle, u32 length, void *result)
{
    PintaException exception = PINTA_OK;
    PintaEnvironment *core_environment;
    PintaApiEnvironment *environment;
    uint32_t actual_read, read_remaining;
    void *buffer;

    if (core == NULL)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_INVALID_OPERATION);

    if (result == NULL)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_INVALID_ARGUMENTS);

    core_environment = core->environment;
    if (core_environment == NULL)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_INVALID_OPERATION);

    environment = (PintaApiEnvironment *)core_environment->native_environment;
    if (environment == NULL)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_INVALID_OPERATION);

    if (environment->file_open == NULL
        || environment->file_size == NULL
        || environment->file_read == NULL
        || environment->file_close == NULL)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_INVALID_OPERATION);

    buffer = result;
    read_remaining = length;
    while (read_remaining > 0)
    {
        actual_read = environment->file_read(environment->environment_context, handle, buffer, read_remaining);
        if (actual_read == 0)
            PINTA_THROW(PINTA_EXCEPTION_PLATFORM); // we were unable to read the full file

        if (actual_read > read_remaining)
            PINTA_THROW(PINTA_EXCEPTION_PLATFORM);

        read_remaining -= actual_read;
        buffer = &((u8*)buffer)[actual_read];
    }

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_api_string_decode(PintaCore *core, PintaApiString *string, PintaReference *result)
{
    PintaException exception = PINTA_OK;

    if (string->string_data == NULL || string->string_length == 0)
        PINTA_CHECK(pinta_lib_string_alloc(core, 0, result));
    else if (string->string_encoding == PINTA_API_ENCODING_C)
        PINTA_CHECK(pinta_lib_decode_string_c(core, string->string_data, string->string_length, result));
    else if (string->string_encoding == PINTA_API_ENCODING_UTF8)
        PINTA_CHECK(pinta_lib_decode_utf8(core, string->string_data, string->string_length, result));
    else if (string->string_encoding == PINTA_API_ENCODING_UTF16)
        PINTA_CHECK(pinta_lib_string_alloc_copy(core, (wchar*)string->string_data, string->string_length, result));
    else
        PINTA_THROW(PINTA_EXCEPTION_INVALID_ARGUMENTS);

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_api_string_encode(PintaCore *core, PintaReference *string, PintaApiEncoding encoding, uint32_t *string_length, void **string_data)
{
    PintaException exception = PINTA_OK;
    PintaReference buffer;
    PintaHeapObject *blob;
    u32 length;

    pinta_assert(core != NULL);
    pinta_assert(string != NULL);

    if (core == NULL)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_NULL_REFERENCE);

    if (string_length == NULL)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_NULL_REFERENCE);

    if (string_data == NULL)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_NULL_REFERENCE);

    if (string->reference == NULL)
    {
        *string_length = 0;
        *string_data = NULL;
        return PINTA_OK;
    }

    if (string->reference->block_kind != PINTA_KIND_STRING)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_TYPE_MISMATCH);

    PINTA_GC_ENTER(core, buffer);

    length = pinta_string_ref_get_length(string);
    if (encoding == PINTA_API_ENCODING_C || encoding == PINTA_API_ENCODING_UTF8)
    {
        PINTA_CHECK(pinta_lib_buffer_alloc_value(core, length, &buffer));
        PINTA_CHECK(pinta_lib_buffer_write_string_utf8(core, &buffer, string));

        blob = pinta_buffer_ref_get_blob(&buffer);
        if (blob == NULL)
            PINTA_THROW(PINTA_EXCEPTION_ENGINE);

        *string_length = pinta_buffer_ref_get_length(&buffer);
        *string_data = (void*)pinta_blob_get_data(blob);
    }
    else if (encoding == PINTA_API_ENCODING_UTF16)
    {
        *string_length = pinta_string_ref_get_length(string);
        *string_data = pinta_string_ref_get_data(string);
    }
    else
    {
        PINTA_THROW(PINTA_EXCEPTION_INVALID_ARGUMENTS);
    }

    PINTA_GC_RETURN(core);
}

PintaException pinta_api_internal_out(PintaCore *core, PintaReference *arguments, PintaReference *return_value)
{
    PintaException exception = PINTA_OK;
    u32 arguments_length;
    i32 provided_offset;
    i32 provided_length;
    u32 actual_length;
    PintaType *offset_type, *length_type;
    struct
    {
        PintaReference string;
        PintaReference offset;
        PintaReference length;
    } gc;

    PINTA_GC_ENTER(core, gc);

    if (arguments->reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    if (arguments->reference->block_kind != PINTA_KIND_ARRAY)
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);

    arguments_length = pinta_array_ref_get_length(arguments);

    if (arguments_length == 0)
        PINTA_RETURN();

    PINTA_CHECK(pinta_lib_array_get_item(core, arguments, 0, &gc.string));

    if (gc.string.reference == NULL)
        PINTA_RETURN();

    if (arguments_length > 1)
    {
        if (gc.string.reference->block_kind == PINTA_KIND_BLOB)
        {
            actual_length = pinta_blob_ref_get_length(&gc.string);

            PINTA_CHECK(pinta_lib_array_get_item(core, arguments, 1, &gc.offset));

            if (arguments_length > 2)
                PINTA_CHECK(pinta_lib_array_get_item(core, arguments, 2, &gc.length));

            offset_type = pinta_core_get_type(core, &gc.offset);
            PINTA_CHECK(offset_type->to_integer_value(core, &gc.offset, &provided_offset));

            length_type = pinta_core_get_type(core, &gc.length);
            PINTA_CHECK(length_type->to_integer_value(core, &gc.length, &provided_length));

            if (provided_offset < 0 || provided_length < 0)
                PINTA_THROW(PINTA_EXCEPTION_INVALID_ARGUMENTS);

            if ((u32)provided_offset > actual_length)
                provided_offset = (i32)actual_length;

            if (gc.length.reference == NULL)
                provided_length = (i32)(actual_length - (u32)provided_offset);

            if (core->output.reference == NULL)
                PINTA_CHECK(pinta_lib_buffer_alloc_value(core, (u32)provided_length, &core->output));

            PINTA_CHECK(pinta_lib_buffer_write_blob(core, &core->output, (u32)provided_offset, (u32)provided_length, &gc.string));
        }
        else
        {
            if (core->output.reference == NULL)
                PINTA_CHECK(pinta_lib_buffer_alloc_value(core, 240, &core->output));

            PINTA_CHECK(pinta_lib_format_buffer(core, &core->output, NULL, &gc.string, arguments, 1));
        }
    }
    else
    {
        if (core->output.reference == NULL)
            PINTA_CHECK(pinta_lib_buffer_alloc(core, &core->output));

        PINTA_CHECK(pinta_lib_buffer_write(core, &core->output, &gc.string));
    }

    return_value->reference = NULL;

    PINTA_GC_RETURN(core);
}

PintaException pinta_api_core_file_open(PintaCore *core, PintaReference *file_name, void **file_handle)
{
    PintaException exception = PINTA_OK;
    PintaEnvironment *core_environment;
    PintaApiEnvironment *environment;
    void *name_data;
    uint32_t name_length;
    void *handle;

    pinta_assert(core != NULL);
    pinta_assert(file_name != NULL);

    if (file_handle == NULL)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_NULL_REFERENCE);

    core_environment = core->environment;
    if (core_environment == NULL)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_INVALID_OPERATION);

    environment = (PintaApiEnvironment*)core_environment->native_environment;
    if (environment == NULL)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_INVALID_OPERATION);

    if (environment->file_open == NULL
        || environment->file_size == NULL
        || environment->file_read == NULL
        || environment->file_close == NULL)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_INVALID_OPERATION);

    if (environment->platform_encoding != PINTA_API_ENCODING_UTF16)
    {
        PINTA_CHECK(pinta_api_string_encode(core, file_name, environment->platform_encoding, &name_length, &name_data));
    }
    else
    {
        PINTA_CHECK(pinta_lib_string_to_string(core, file_name, file_name));

        name_length = pinta_string_ref_get_length(file_name);
        name_data = pinta_string_ref_get_data(file_name);
    }

    handle = environment->file_open(environment->environment_context, name_data, name_length);
    if (handle == NULL)
        PINTA_THROW(PINTA_EXCEPTION_FILE_NOT_FOUND);

    *file_handle = handle;

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_api_core_file_size(PintaCore *core, void *file_handle, u32 *file_size)
{
    PintaEnvironment *core_environment;
    PintaApiEnvironment *environment;
    uint32_t size;

    pinta_assert(core != NULL);

    if (file_handle == NULL)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_NULL_REFERENCE);

    core_environment = core->environment;
    if (core_environment == NULL)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_INVALID_OPERATION);

    environment = (PintaApiEnvironment*)core_environment->native_environment;
    if (environment == NULL)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_INVALID_OPERATION);

    if (environment->file_open == NULL
        || environment->file_size == NULL
        || environment->file_read == NULL
        || environment->file_close == NULL)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_INVALID_OPERATION);

    size = environment->file_size(environment->environment_context, file_handle);
    if (file_size)
        *file_size = (u32)size;

    return PINTA_OK;
}

PintaException pinta_api_core_file_read(PintaCore *core, void *file_handle, PintaReference *buffer, u32 length, u32 *actual_read)
{
    PintaException exception = PINTA_OK;
    PintaEnvironment *core_environment;
    PintaApiEnvironment *environment;
    u32 blob_length;
    u32 blob_position;
    PintaReference blob;
    u8 *blob_data;
    void *output;
    u32 read;

    pinta_assert(core != NULL);
    pinta_assert(buffer != NULL);

    if (file_handle == NULL)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_NULL_REFERENCE);

    core_environment = core->environment;
    if (core_environment == NULL)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_INVALID_OPERATION);

    environment = (PintaApiEnvironment*)core_environment->native_environment;
    if (environment == NULL)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_INVALID_OPERATION);

    if (environment->file_open == NULL
        || environment->file_size == NULL
        || environment->file_read == NULL
        || environment->file_close == NULL)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_INVALID_OPERATION);

    PINTA_GC_ENTER(core, blob);

    if (buffer->reference == NULL)
    {
        PINTA_CHECK(pinta_lib_blob_alloc(core, length, buffer));

        blob_position = 0;
        blob.reference = buffer->reference;
    }
    else if (buffer->reference->block_kind == PINTA_KIND_BUFFER)
    {
        PINTA_CHECK(pinta_lib_buffer_ensure_write(core, buffer, length));

        blob_position = pinta_buffer_ref_get_position(buffer);
        blob.reference = pinta_buffer_ref_get_blob(buffer);
    }
    else if (buffer->reference->block_kind == PINTA_KIND_BLOB)
    {
        blob_length = pinta_blob_ref_get_length(buffer);
        if (blob_length < length)
            PINTA_THROW(PINTA_EXCEPTION_OUT_OF_RANGE);

        blob_position = 0;
        blob.reference = buffer->reference;
    }
    else
    {
        PINTA_THROW(PINTA_EXCEPTION_TYPE_MISMATCH);
    }

    blob_data = pinta_blob_ref_get_data(&blob);
    output = (void*)&blob_data[blob_position];

    read = environment->file_read(environment->environment_context, file_handle, output, (uint32_t)length);
    if (actual_read)
        *actual_read = read;

    PINTA_GC_RETURN(core);
}

PintaException pinta_api_core_file_close(PintaCore *core, void *file_handle)
{
    PintaEnvironment *core_environment;
    PintaApiEnvironment *environment;

    pinta_assert(core != NULL);

    if (file_handle == NULL)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_NULL_REFERENCE);

    core_environment = core->environment;
    if (core_environment == NULL)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_INVALID_OPERATION);

    environment = (PintaApiEnvironment*)core_environment->native_environment;
    if (environment == NULL)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_INVALID_OPERATION);

    if (environment->file_open == NULL
        || environment->file_size == NULL
        || environment->file_read == NULL
        || environment->file_close == NULL)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_INVALID_OPERATION);

    environment->file_close(environment->environment_context, file_handle);

    return PINTA_OK;
}

PintaException pinta_api_set_builtin_objects(PintaCore *core, PintaModuleDomain *domain)
{
    PintaException exception = PINTA_OK;

    struct
    {
        PintaReference name;
        PintaReference value;
    } gc;

    PINTA_GC_ENTER(core, gc);

    PINTA_CHECK(pinta_lib_decode_string_c(core, "require", 7, &gc.name));
    PINTA_CHECK(pinta_lib_function_native_alloc(core, pinta_code_rt_require, 0, NULL, NULL, NULL, NULL, &gc.value));
    PINTA_CHECK(pinta_lib_module_set_global_value(core, domain, &gc.name, &gc.value));

    PINTA_GC_RETURN(core);
}

PintaException pinta_api_core_load_module(PintaCore *core, PintaReference *module_name, PintaModuleDomain **domain)
{
    PintaException exception = PINTA_OK;
    void *handle = NULL;

    pinta_assert(core != NULL);
    pinta_assert(module_name != NULL);

    if (module_name->reference == NULL)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_NULL_REFERENCE);

    if (domain == NULL)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_NULL_REFERENCE);

    PINTA_CHECK(pinta_api_core_file_open(core, module_name, &handle));
    PINTA_CHECK(pinta_api_load_module_handle(core, handle, domain));

    if (*domain)
        PINTA_CHECK(pinta_api_set_builtin_objects(core, *domain));

PINTA_EXIT:
    if (handle != NULL)
        pinta_api_core_file_close(core, handle);

    return PINTA_EXCEPTION(exception);
}

PintaException pinta_api_load_module_handle(PintaCore *core, void *handle, PintaModuleDomain **domain)
{
    PintaException exception = PINTA_OK;
    PintaNativeMemory *memory;
    u32 module_length;
    void *module_data;
    PintaModule *module;

    memory = core->memory;
    if (memory == NULL)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_INVALID_OPERATION);

    PINTA_CHECK(pinta_api_core_file_size(core, handle, &module_length));

    module_data = pinta_memory_alloc(memory, module_length);
    if (module_data == NULL)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_MEMORY);

    PINTA_CHECK(pinta_api_read_file(core, handle, module_length, module_data));

    module = pinta_module_init(module_data, module_length);
    if (module == NULL)
        PINTA_THROW(PINTA_EXCEPTION_INVALID_MODULE);

    PINTA_CHECK(pinta_core_init_domain(core, module, domain));

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}