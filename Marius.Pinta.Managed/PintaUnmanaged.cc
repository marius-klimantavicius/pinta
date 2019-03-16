#include "Stdafx.h"
#include "PintaUnmanaged.h"
#include "PintaEngineException.h"
#include "PintaEnvironment.h"

PintaException pinta_managed_init(PintaManagedEngineState *state, wchar *filename)
{
    PintaException exception = PINTA_OK;
    PintaApi *api;
    PintaApiString name;
    void *domain;
    PintaCore *core;
    PintaApiEnvironment environment = { 0 };

    name.string_data = filename;
    name.string_length = string_get_length(filename);
    name.string_encoding = PINTA_API_ENCODING_UTF16;

    environment.environment_context = state->engine;
    environment.memory = state->native_memory;
    environment.memory_length = state->native_length;
    environment.file_open = pinta_managed_file_open;
    environment.file_size = pinta_managed_file_size;
    environment.file_read = pinta_managed_file_read;
    environment.file_close = pinta_managed_file_close;
    environment.stack_length = state->stack_length;
    environment.heap_length = state->heap_length;
    environment.platform_encoding = PINTA_API_ENCODING_UTF16;

    state->api = api = pinta_api_create(&environment);
    if (api == NULL)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_OUT_OF_MEMORY);
    state->core = core = (PintaCore*)api->core;
    if (core == NULL)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_OUT_OF_MEMORY);

    domain = api->load_module(api, &name);
    if (domain == NULL)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_INVALID_MODULE);

    state->domain = (PintaModuleDomain*)domain;
    return PINTA_OK;
}

PintaException pinta_managed_set_global(PintaManagedEngineState *state, void *name, i32 nameLength, void *value, i32 valueLength)
{
    PintaApi *api = state->api;

    PintaApiString name_string, value_string;

    name_string.string_data = name;
    name_string.string_length = nameLength;
    name_string.string_encoding = PINTA_API_ENCODING_UTF16;

    value_string.string_data = value;
    value_string.string_length = valueLength;
    value_string.string_encoding = PINTA_API_ENCODING_UTF16;

    return (PintaException)api->set_string(api, state->domain, &name_string, &value_string);
}

PintaException pinta_managed_set_global_null(PintaManagedEngineState *state, void *name, i32 nameLength)
{
    PintaApi *api = state->api;
    PintaApiString name_string;

    name_string.string_data = name;
    name_string.string_length = nameLength;
    name_string.string_encoding = PINTA_API_ENCODING_UTF16;

    return (PintaException)api->set_null(api, state->domain, &name_string);
}

PintaException pinta_managed_get_global(PintaManagedEngineState *state, void *name, i32 nameLength, void **valueData, i32 *valueLength)
{
    PintaException exception = PINTA_OK;
    PintaApi *api = state->api;
    PintaApiString name_string;
    uint32_t length;

    name_string.string_data = name;
    name_string.string_length = nameLength;
    name_string.string_encoding = PINTA_API_ENCODING_UTF16;

    exception = (PintaException)api->unsafe_get_string(api, state->domain, &name_string, PINTA_API_ENCODING_UTF16, &length, valueData);
    *valueLength = (i32)length;

    return PINTA_EXCEPTION(exception);
}

static const u8 empty_bytes[1];

PintaException pinta_managed_execute(PintaManagedEngineState *state, uint32_t *bufferLength, void **bufferData)
{
    PintaException exception = PINTA_OK;
    PintaApi *api = state->api;

    PINTA_CHECK((PintaException)api->execute(api, state->domain));
    PINTA_CHECK((PintaException)api->unsafe_get_output_buffer(api, bufferLength, bufferData));

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}
