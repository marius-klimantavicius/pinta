#if defined(WIN32)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdint.h>

void *pinta_platform_file_open(void *context, void *name_data, uint32_t name_length)
{
    HANDLE handle;

    ((void)name_length);
    ((void)context);

    wchar_t *name = (wchar_t*)name_data;
    handle = CreateFile(name, FILE_GENERIC_READ | FILE_GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (handle == INVALID_HANDLE_VALUE)
        return NULL;

    return (void*)handle;
}

uint32_t pinta_platform_file_size(void *context, void *handle)
{
    ((void)context);

    HANDLE file_handle = (HANDLE)handle;
    return (uint32_t)GetFileSize(file_handle, NULL);
}

uint32_t pinta_platform_file_read(void* context, void *handle, void *buffer, uint32_t length)
{
    ((void)context);

    DWORD actual_read;
    HANDLE file_handle = (HANDLE)handle;
    BOOL success = ReadFile(file_handle, buffer, length, &actual_read, NULL);
    if (!success)
        return 0;

    return (uint32_t)actual_read;
}

void pinta_platform_file_close(void *context, void *handle)
{
    ((void)context);

    HANDLE file_handle = (HANDLE)handle;
    CloseHandle(file_handle);
}

uint32_t pinta_platform_file_write(void *context, void *handle, void *data, uint32_t data_length)
{
    ((void)context);

    HANDLE fileHandle = (HANDLE)handle;
    DWORD written = 0;

    BOOL success = WriteFile(fileHandle, data, data_length, &written, NULL);
    if (!success)
        return 0;

    return (uint32_t)written;
}


uint32_t pinta_platform_file_set_end_of_file(void *context, void *handle)
{
    ((void)context);

    HANDLE fileHandle = (HANDLE)handle;

    BOOL success = SetEndOfFile(fileHandle);
    if (!success)
        return 0;

    return 1;
}
#endif