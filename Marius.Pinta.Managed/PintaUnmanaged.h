#if !defined(UNMANAGED_H)
#define UNMANAGED_H

#pragma unmanaged

typedef struct PintaManagedEngineState
{
    void *engine;

    u32 native_length;
    void *native_memory;

    PintaApi *api;
    PintaCore *core;
    PintaModuleDomain *domain;

    u32 heap_length;
    u32 stack_length;

} PintaManagedEngineState;

PintaException pinta_managed_init(PintaManagedEngineState *state, wchar *filename);
PintaException pinta_managed_set_global(PintaManagedEngineState *state, void *name, i32 nameLength, void *value, i32 valueLength);
PintaException pinta_managed_set_global_null(PintaManagedEngineState *state, void *name, i32 nameLength);
PintaException pinta_managed_get_global(PintaManagedEngineState *state, void *name, i32 nameLength, void **valueData, i32 *valueLength);
PintaException pinta_managed_execute(PintaManagedEngineState *state, uint32_t *bufferLength, void **bufferData);

#pragma managed

#endif /* UNMANAGED_H */