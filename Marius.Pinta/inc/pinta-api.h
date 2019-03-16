#ifndef PINTA_API_H
#define PINTA_API_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define PINTA_API_OK                    0
#define PINTA_API_VERSION               "2.0.2"

typedef struct PintaApiEnvironment PintaApiEnvironment;
typedef struct PintaApi PintaApi;
typedef struct PintaApiString PintaApiString;

typedef enum PintaApiEncoding
{
    PINTA_API_ENCODING_C = 0,
    PINTA_API_ENCODING_UTF8 = 1,
    PINTA_API_ENCODING_UTF16 = 2
} PintaApiEncoding;

typedef void*(*pinta_api_load_module_fn)(PintaApi* api, PintaApiString *name);
typedef uint32_t(*pinta_api_set_global_string_fn)(PintaApi *api, void *module, PintaApiString *name, PintaApiString *value);
typedef uint32_t(*pinta_api_set_global_integer_fn)(PintaApi *api, void *module, PintaApiString *name, int32_t value);
typedef uint32_t(*pinta_api_set_global_null_fn)(PintaApi *api, void *module, PintaApiString *name);
typedef uint32_t(*pinta_api_execute_module_fn)(PintaApi *api, void *module);
typedef uint32_t(*pinta_api_get_global_string_fn)(PintaApi *api, void *module, PintaApiString *name, PintaApiEncoding encoding, uint32_t *string_length, void **string_data);
typedef uint32_t(*pinta_api_get_output_buffer_fn)(PintaApi *api, uint32_t *buffer_length, void **buffer_data);
typedef uint32_t(*pinta_api_get_output_string_fn)(PintaApi *api, PintaApiEncoding encoding, uint32_t *string_length, void **string_data);

struct PintaApi
{
    void *core;

    pinta_api_load_module_fn load_module;
    pinta_api_set_global_string_fn set_string;
    pinta_api_set_global_integer_fn set_integer;
    pinta_api_set_global_null_fn set_null;

    pinta_api_execute_module_fn execute;

    pinta_api_get_global_string_fn unsafe_get_string;
    pinta_api_get_output_buffer_fn unsafe_get_output_buffer;
    pinta_api_get_output_string_fn unsafe_get_output_string;
};

struct PintaApiString
{
    void *string_data;
    uint32_t string_length;
    PintaApiEncoding string_encoding;
};

struct PintaApiEnvironment
{
    /* anything from the env, passed to file_* functions */
    void *environment_context;

    void *memory;
    uint32_t memory_length;
    uint32_t heap_length;
    uint32_t stack_length;

    /* 
     * 0 - use default initial capacity of 2816, non-zero - use provided initial capacity.
     * The output buffer resizes automatically, use sufficiently large initial capacity to limit the number of resizes. 
     */
    uint32_t initial_output_capacity; 

    enum PintaApiEncoding platform_encoding;

    void *(*file_open)(void *environment_context, void *name, uint32_t name_length);
    uint32_t(*file_size)(void *environment_context, void *handle);
    uint32_t(*file_read)(void *environment_context, void *handle, void *buffer, uint32_t length);
    void(*file_close)(void *environment_context, void *handle);
};

PintaApi *pinta_api_create(PintaApiEnvironment *environment);

uint32_t pinta_api_util_set_string_c(PintaApi *api, void *module, char *name, char *value);
uint32_t pinta_api_util_set_string_utf8(PintaApi *api, void *module, char *name, char *value);

#ifdef __cplusplus
}
#endif

#endif // PINTA_API_H
