#pragma once

#include "Stdafx.h"

#pragma unmanaged
extern "C"
{
    void *pinta_platform_file_open(void *context, void *name_data, uint32_t name_length);
    uint32_t pinta_platform_file_size(void *context, void *handle);
    uint32_t pinta_platform_file_read(void *context, void *handle, void *buffer, uint32_t length);
    void pinta_platform_file_close(void *context, void *handle);
    uint32_t pinta_platform_file_write(void *context, void *handle, void *data, uint32_t data_length);
}

#pragma managed

void *pinta_managed_file_open(void *context, void *name_data, uint32_t name_length);
uint32_t pinta_managed_file_size(void *context, void *handle);
uint32_t pinta_managed_file_read(void *context, void *handle, void *buffer, uint32_t length);
void pinta_managed_file_close(void *context, void *handle);
uint32_t pinta_managed_file_write(wchar_t *filename, void *data, uint32_t data_length);
