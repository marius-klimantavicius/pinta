#include "pinta_tests.h"

void *heap;

PintaApiEnvironment environment;
PintaApi *api;
PintaCore *core;

__sput_struct __sput;

void pinta_tests()
{
    sput_start_testing();

    pinta_tests_gc();
    pinta_tests_array();
    pinta_tests_memory();
    pinta_tests_code();
    pinta_tests_code_v2();
    pinta_tests_buffer();
    pinta_tests_format();
    pinta_tests_integer();
    pinta_tests_decimal();
    pinta_tests_weak();
    pinta_tests_encoding();
    pinta_tests_property();
    pinta_tests_object();
    pinta_tests_function();
    pinta_tests_pattern();
    pinta_tests_json();

    sput_finish_testing();
}

void pinta_init_tests(int heap_size, int stack_size)
{
    if (heap == NULL)
        heap = malloc(64 * 1024);

    pinta_assert(heap != NULL);
    pinta_assert(heap_size + stack_size <= 60 * 1024);

    memset(heap, 0xCD, 64 * 1024);
    
    environment.environment_context = NULL;
    environment.memory = heap;
    environment.memory_length = 64 * 1024;
    environment.file_open = pinta_platform_file_open;
    environment.file_size = pinta_platform_file_size;
    environment.file_read = pinta_platform_file_read;
    environment.file_close = pinta_platform_file_close;
    environment.stack_length = stack_size;
    environment.heap_length = heap_size;
    environment.platform_encoding = PINTA_API_ENCODING_UTF16;

    api = pinta_api_create(&environment);
    core = (PintaCore*)api->core;
}

void pinta_dispose_tests()
{
}
