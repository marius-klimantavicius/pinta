#include <string.h>
#include <stddef.h>

#define PINTA_WITH_DEBUGGER 1

#if PINTA_WITH_DEBUGGER // ignore - debugger is (really slowly) in progress and is not usable
#include "debugger.h"
#endif
#include "pinta-api.h"

static uint8_t data[128 * 1024];

void *pinta_platform_file_open(void *context, void *name_data, uint32_t name_length);
uint32_t pinta_platform_file_size(void *context, void *handle);
uint32_t pinta_platform_file_read(void *context, void *handle, void *buffer, uint32_t length);
void pinta_platform_file_close(void *context, void *handle);
uint32_t pinta_platform_file_write(void *context, void *handle, void *data, uint32_t data_length);
uint32_t pinta_platform_file_set_end_of_file(void *context, void *handle);

#define CHECK(x) do { err = (x); if (err != 0) { printf("%i", err); return err; } } while (0, 0)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

uint32_t run(void *block, uint32_t block_length)
{
    struct PintaApiEnvironment environment = { 0 };
    struct PintaApi *api;
    struct PintaApiString string;
    void *module;
    void *buffer_data;
    uint32_t buffer_length;
    uint32_t err;

    environment.memory = block;
    environment.memory_length = block_length;

    // we use the functions to read executable from the disk (or whatever)

    /*
    void* file_open(void *environment_context, void *name, uint32_t name_length);
            Opens a file
                environment_context - passed environment.environment_context
                name - file name, converted to environment.platform_encoding
                name_length - length of the received file name

                returns handle to the opened file

    uint32_t file_size(void *environment_context, void *handle)
            Gets the file size
                environment_context - passed environment.environment_context
                handle - file handle returned from file_open

                returns file size in bytes

    uint32_t file_read(void *environment_context, void *handle, void *buffer, uint32_t length)
            Read from the file
                environment_context - passed environment.environment_context
                handle - file handle returned from file_open
                buffer - output buffer to read data to
                buffer_length - output buffer capacity

                returns number of actual bytes read

    void file_close(void *environment_context, void *handle)
            Closes an opened file
                environment_context - passed environment.environment_context
                handle - file handle returned from file_open
    */
    environment.environment_context = NULL;
    environment.file_open = pinta_platform_file_open;
    environment.file_size = pinta_platform_file_size;
    environment.file_read = pinta_platform_file_read;
    environment.file_close = pinta_platform_file_close;

    environment.stack_length = 1024;
    environment.heap_length = 32 * 1024;
    environment.platform_encoding = PINTA_API_ENCODING_UTF16;
    environment.initial_output_capacity = 2048;

    api = pinta_api_create(&environment);

#if PINTA_WITH_DEBUGGER
    pinta_api_debugger_start(api, "127.0.0.1", 34001);
    pinta_api_debugger_wait(api);
#endif

    string.string_data = L"..\\Marius.Pinta.Test.Files\\sample.pint";
    string.string_length = wcslen((wchar_t*)string.string_data);
    string.string_encoding = PINTA_API_ENCODING_UTF16;
    module = api->load_module(api, &string);
    if (module == NULL)
        return 500;

    //CHECK(pinta_api_util_set_string_c(api, module, "OP", "SALE"));

    CHECK(api->execute(api, module));

    /*
     * api->unsafe_* - these functions return data to pinta managed heap, any call to api function might invalidate returned pointers
     * if multiple unsafe calls must be made, make sure that previously returned pointers are no longer used
     */

    CHECK(api->unsafe_get_output_buffer(api, &buffer_length, &buffer_data));

#if PINTA_WITH_DEBUGGER
    Sleep(10000);
    pinta_api_debugger_stop(api);
#endif

#if _DEBUG
    if (buffer_data && buffer_length > 0)
    {
        void *handle;
        wchar_t *name = L"..\\Marius.Pinta.Test.Files\\sample.txt";
        uint32_t written;

        handle = pinta_platform_file_open(NULL, name, wcslen(name));
        if (!handle)
            return (int)GetLastError();

        written = pinta_platform_file_write(NULL, handle, buffer_data, buffer_length);
        pinta_platform_file_set_end_of_file(NULL, handle);
        pinta_platform_file_close(NULL, handle);
        return written == 0;
    }

    return 1;
#else
    return 0;
#endif
}


int main()
{
    SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);

    HANDLE current = GetCurrentThread();
    SetThreadPriority(current, THREAD_PRIORITY_HIGHEST);

#if !_DEBUG
    uint32_t m = 0;
    for (m = 0; m < 10; m++)
#endif
    {
        LARGE_INTEGER start, end, freq;
        QueryPerformanceCounter(&start);

#if !_DEBUG
        uint32_t index;
        for (index = 0; index < 1000; index++)
#endif
        {
            uint32_t result;
            result = run(data, sizeof(data));
            if (result != 0)
                return 1;
        }

        QueryPerformanceCounter(&end);
        QueryPerformanceFrequency(&freq);

        printf("%lld - %lld\n", end.QuadPart - start.QuadPart, freq.QuadPart);
    }
    return 0;
}

