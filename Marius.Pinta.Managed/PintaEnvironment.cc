#include "Stdafx.h"
#include "PintaEngine.h"
#include "PintaEnvironment.h"

using namespace System;
using namespace System::Runtime::InteropServices;
using namespace Marius::Pinta::Managed;

#define GCHANDLE_TO_POINTER(handle) ((GCHandle::operator IntPtr(handle)).ToPointer())
#define POINTER_TO_GCHANDLE(pointer) (GCHandle::operator GCHandle(IntPtr(pointer)))

PintaEngine^ pinta_managed_get_engine(void * context)
{
    GCHandle handle = POINTER_TO_GCHANDLE(context);
    return safe_cast<PintaEngine^>(handle.Target);
}

void *pinta_managed_file_open(void *context, void *name_data, uint32_t name_length)
{
    PintaEngine ^engine = pinta_managed_get_engine(context);

    if (engine->Environment != nullptr)
    {
        String ^name = Marshal::PtrToStringUni(IntPtr(name_data), name_length);
        Object^ fileHandle = engine->Environment->OpenFile(name);

        GCHandle handle = GCHandle::Alloc(fileHandle);
        return GCHANDLE_TO_POINTER(handle);
    }

    return pinta_platform_file_open(context, name_data, name_length);
}

uint32_t pinta_managed_file_size(void *context, void *handle)
{
    PintaEngine ^engine = pinta_managed_get_engine(context);

    if (engine->Environment != nullptr)
    {
        GCHandle fileHandle = POINTER_TO_GCHANDLE(handle);
        return engine->Environment->GetFileSize(fileHandle.Target);
    }

    return pinta_platform_file_size(context, handle);
}

uint32_t pinta_managed_file_read(void *context, void *handle, void *buffer, uint32_t length)
{
    PintaEngine ^engine = pinta_managed_get_engine(context);

    if (engine->Environment != nullptr)
    {
        array<Byte>^ data = gcnew array<Byte>(length);
        GCHandle fileHandle = POINTER_TO_GCHANDLE(handle);
        
        UInt32 result = engine->Environment->ReadFile(fileHandle.Target, data);

        Marshal::Copy(data, 0, IntPtr(buffer), result);
        return result;
    }

    return pinta_platform_file_read(context, handle, buffer, length);
}

void pinta_managed_file_close(void *context, void *handle)
{
    PintaEngine ^engine = pinta_managed_get_engine(context);

    if (engine->Environment != nullptr)
    {
        GCHandle fileHandle = POINTER_TO_GCHANDLE(handle);
        try
        {
            engine->Environment->CloseFile(fileHandle.Target);
        }
        finally
        {
            fileHandle.Free();
        }

        return;
    }

    pinta_platform_file_close(context, handle);
}

uint32_t pinta_managed_file_write(void *context, void *handle, void *data, uint32_t data_length)
{
    PintaEngine ^engine = pinta_managed_get_engine(context);

    if (engine->Environment != nullptr)
    {
        array<Byte>^ buffer = gcnew array<Byte>(data_length);
        GCHandle fileHandle = POINTER_TO_GCHANDLE(handle);

        Marshal::Copy(IntPtr(data), buffer, 0, data_length);
        UInt32 result = engine->Environment->WriteFile(fileHandle.Target, buffer);
        return result;
    }

    return pinta_platform_file_read(context, handle, data, data_length);
}
