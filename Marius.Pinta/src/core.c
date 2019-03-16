#include "pinta.h"

void pinta_core_init_type(PintaType *type)
{
    type->is_string = 0;

    type->gc_walk = NULL;
    type->gc_relocate = NULL;

    type->to_integer = pinta_core_default_to_integer;
    type->to_integer_value = pinta_core_default_to_integer_value;
    type->to_decimal = pinta_core_default_to_decimal;
    type->to_string = pinta_core_default_to_string;
    type->to_numeric = pinta_core_default_to_decimal;
    type->to_bool = pinta_core_default_to_bool;

    type->to_zero = pinta_core_default_to_zero;

    type->get_member = NULL;
    type->set_member = NULL;

    type->get_item = pinta_core_default_get_item;
    type->set_item = pinta_core_default_set_item;

    type->get_length = pinta_core_default_get_length;
    type->get_char = pinta_core_default_get_char;

    type->debug_write = pinta_core_default_debug_write;
}

PintaException pinta_null_to_bool(PintaCore *core, PintaReference *value, u8 *result)
{
    pinta_assert(value != NULL);
    pinta_assert(result != NULL);

    PINTA_UNUSED(core);
    PINTA_UNUSED(value);

    *result = 0;
    return PINTA_OK;
}

PintaException pinta_null_debug_write(PintaCore *core, PintaReference *value, u32 max_depth, PintaJsonWriter *writer)
{
    PINTA_UNUSED(value);
    PINTA_UNUSED(max_depth);

    return pinta_json_write_null(core, writer);
}

void pinta_null_init_type(PintaType *type)
{
    type->is_string = 1;

    type->to_bool = pinta_null_to_bool;
    type->debug_write = pinta_null_debug_write;
}

PintaCore *pinta_core_init(PintaNativeMemory *memory, u32 heap_memory_length_in_bytes, u32 stack_memory_in_bytes)
{
    PintaCore *core;
    PintaThread *thread;
    u32 index;

    if (heap_memory_length_in_bytes < 1024)
        return NULL;

    if (stack_memory_in_bytes < 128)
        return NULL;

    core = (PintaCore*)pinta_memory_alloc(memory, sizeof(PintaCore));
    if (core == NULL)
        return NULL;

    core->memory = memory;
    core->domains = NULL;
    core->native = NULL;
    core->debugger = NULL;

    core->heap_handles = NULL;

    core->weak.reference = NULL;
    core->externals.reference = NULL;
    core->output.reference = NULL;
    core->null.reference = NULL;
    core->internal_functions = NULL;
    core->internal_functions_length = 0;
    core->heap = pinta_heap_init(memory, heap_memory_length_in_bytes);

    if (core->heap == NULL)
        return NULL;

    pinta_core_init_type(&core->null_type);
    for (index = 0; index < PINTA_KIND_LENGTH; index++)
        pinta_core_init_type(&core->types[index]);

    pinta_null_init_type(&core->null_type);
    pinta_integer_init_type(&core->types[PINTA_KIND_INTEGER]);
    pinta_decimal_init_type(&core->types[PINTA_KIND_DECIMAL]);
    pinta_string_init_type(&core->types[PINTA_KIND_STRING]);
    pinta_substring_init_type(&core->types[PINTA_KIND_SUBSTRING]);
    pinta_multistring_init_type(&core->types[PINTA_KIND_MULTISTRING]);
    pinta_char_init_type(&core->types[PINTA_KIND_CHAR]);
    pinta_array_init_type(&core->types[PINTA_KIND_ARRAY]);
    pinta_object_init_type(&core->types[PINTA_KIND_OBJECT]);
    pinta_buffer_init_type(&core->types[PINTA_KIND_BUFFER]);
    pinta_blob_init_type(&core->types[PINTA_KIND_BLOB]);
    pinta_weak_init_type(&core->types[PINTA_KIND_WEAK]);
    pinta_function_managed_init_type(&core->types[PINTA_KIND_FUNCTION_MANAGED]);
    pinta_function_native_init_type(&core->types[PINTA_KIND_FUNCTION_NATIVE]);
    pinta_property_table_init_type(&core->types[PINTA_KIND_PROPERTY_TABLE]);
    pinta_global_object_init_type(&core->types[PINTA_KIND_GLOBAL_OBJECT]);

    thread = (PintaThread*)pinta_memory_alloc(memory, sizeof(PintaThread));
    if (thread == NULL)
        return NULL;

    thread->core = core;
    thread->code_is_suspended = 0;
    thread->code_finished = 0;
    thread->code_result = PINTA_OK;
    thread->code_next_pointer = NULL;
    thread->code_pointer = NULL;
    thread->frame = pinta_frame_init(memory, stack_memory_in_bytes);
    thread->frame->code_end = NULL;
    thread->frame->code_start = NULL;
    thread->frame->is_final_frame = 1;
    thread->prev = NULL;
    thread->next = NULL;
    thread->domain = NULL;

    core->threads = thread;

    core->cache = pinta_core_cache_init(memory);

    return core;
}

PintaException pinta_core_init_domain(PintaCore *core, PintaModule *module, PintaModuleDomain **result)
{
    PintaException exception = PINTA_OK;
    PintaModuleDomain *domain;

    pinta_assert(core != NULL);
    pinta_assert(core->memory != NULL);
    pinta_assert(module != NULL);

    domain = (PintaModuleDomain*)pinta_memory_alloc(core->memory, sizeof(PintaModuleDomain));
    if (domain == NULL)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_OUT_OF_MEMORY);

    domain->module = module;

    domain->global_object.reference = NULL;
    
    domain->globals.reference = NULL;
    domain->strings.reference = NULL;

    domain->next = core->domains;
    domain->prev = NULL;

    core->domains = domain;

    if (module->globals_length > 0)
        PINTA_CHECK(pinta_lib_array_alloc(core, module->globals_length, &domain->globals));

    if (module->strings_length > 0)
        PINTA_CHECK(pinta_lib_array_alloc(core, module->strings_length, &domain->strings));

    if (result)
        *result = domain;

    pinta_debug_raise_domain(core, module, domain);

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaHeapObject *pinta_core_alloc(PintaCore *core, u8 block_kind, u8 block_flags, u32 block_length)
{
    PintaHeap *heap;
    PintaHeapObject *result;

    pinta_assert(core != NULL);

    if (core->heap == NULL)
        return NULL;

    result = NULL;

    for (heap = core->heap; heap != NULL; heap = heap->next)
    {
        result = pinta_heap_alloc(heap, block_kind, block_flags, block_length);
        if (result != NULL)
            return result;
    }

    if (result == NULL)
    {
        pinta_core_gc(core, 0);

        for (heap = core->heap; heap != NULL; heap = heap->next)
        {
            result = pinta_heap_alloc(heap, block_kind, block_flags, block_length);
            if (result != NULL)
                return result;
        }

        // lets bring the big guns
        pinta_gc_compact(core);

        for (heap = core->heap; heap != NULL; heap = heap->next)
        {
            result = pinta_heap_alloc(heap, block_kind, block_flags, block_length);
            if (result != NULL)
                return result;
        }
    }

    // we give up, not enough memory (or it might be too fragmented [did someone alloc pinned?])
    return NULL;
}

void pinta_core_gc(PintaCore *core, u8 compact)
{
    PintaHeap *heap;

    pinta_assert(core != NULL);

    pinta_trace1("gc compact: %d...", (int)compact);

    pinta_gc_mark(core);
    pinta_gc_clear_weak(core);

    for (heap = core->heap; heap != NULL; heap = heap->next)
        pinta_gc_sweep(heap);

    if (compact)
        pinta_gc_compact(core);

    pinta_trace0("ended");
}

void pinta_core_pin(PintaHeapObject *object)
{
    if (object == NULL)
        return;

    pinta_assert(object->block_kind != PINTA_KIND_FREE);

    object->block_flags |= PINTA_FLAG_PINNED;
}

void pinta_core_unpin(PintaHeapObject *object)
{
    if (object == NULL)
        return;

    pinta_assert(object->block_kind != PINTA_KIND_FREE);

    object->block_flags &= ~PINTA_FLAG_PINNED;
}

PintaHeapCache *pinta_core_cache_init(PintaNativeMemory *memory)
{
    i32 index = 0;
    PintaHeapCache *result;

    result = (PintaHeapCache*)pinta_memory_alloc(memory, sizeof(PintaHeapCache));

    for (index = 0; index < 101; index++)
    {
        pinta_heap_object_init(&result->integers[index], PINTA_KIND_INTEGER, PINTA_FLAG_KEEP_ALIVE, 1);
        pinta_integer_set_value(&result->integers[index], index - 1);
    }

    for (index = 0; index < 128; index++)
    {
        pinta_heap_object_init(&result->chars[index], PINTA_KIND_CHAR, PINTA_FLAG_KEEP_ALIVE, 1);
        pinta_char_set_value(&result->chars[index], (wchar)index);
    }

    pinta_heap_object_init(&result->string_empty[0], PINTA_KIND_STRING, PINTA_FLAG_KEEP_ALIVE, 2);
    pinta_string_set_data(&result->string_empty[0], (wchar*)&result->string_empty[1]);
    pinta_string_set_length(&result->string_empty[0], 0);
    pinta_string_set_item(&result->string_empty[0], 0, PINTA_CHAR('\0'));

    pinta_heap_object_init(&result->array_empty[0], PINTA_KIND_ARRAY, PINTA_FLAG_KEEP_ALIVE, 2);
    pinta_array_set_length(&result->array_empty[0], 0);

    pinta_heap_object_init(&result->property_table_empty[0], PINTA_KIND_PROPERTY_TABLE, PINTA_FLAG_KEEP_ALIVE, 1);
    pinta_property_table_set_capacity(&result->property_table_empty[0], 0);

    return result;
}

#if PINTA_DEBUG

// we are no longer including wchar.h, thus add definition in here
wint_t putwchar(wchar_t);

PintaException pinta_core_print(PintaCore *core, PintaHeapObject *value)
{
    PintaException exception = PINTA_OK;
    PintaKind kind;
    wchar *data;
    wchar character;

    u32 index;
    u32 length, offset, data_length;

    PintaHeapObject *string;
    PintaHeapObject **parts;

    pinta_assert(core != NULL);
    pinta_assert(value != NULL);

    if (value == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    kind = (PintaKind)value->block_kind;
    switch (kind)
    {
    case PINTA_KIND_FREE:
        PINTA_THROW(PINTA_EXCEPTION_ENGINE);
        break;

    case PINTA_KIND_STRING:
        length = pinta_string_get_length(value);
        data = pinta_string_get_data(value);
        if (data == NULL) // null string?
            PINTA_RETURN();

        for (index = 0; index < length; index++)
            putwchar(data[index]);
        break;

    case PINTA_KIND_SUBSTRING:
        string = pinta_substring_get_value(value);
        if (string == NULL)
            PINTA_RETURN(); // this should not happen

        offset = pinta_substring_get_offset(value);
        length = pinta_substring_get_length(value);
        data = pinta_string_get_data(string);
        data_length = pinta_string_get_length(string);

        if (offset >= data_length) // are we pointing past string?
            PINTA_RETURN();

        if (offset + length > data_length)  // do we want more data than it can provide?
            length = data_length - offset;

        data = &data[offset];
        for (index = 0; index < length; index++)
            putwchar(data[index]);
        break;

    case PINTA_KIND_MULTISTRING:
        parts = pinta_multistring_get_value(value);
        if (parts == NULL)
            PINTA_RETURN();

        for (index = 0; index < PINTA_MULTISTRING_COUNT; index++)
        {
            if (parts[index] != NULL)
                PINTA_CHECK(pinta_core_print(core, parts[index]));
        }

        break;

    case PINTA_KIND_CHAR:

        character = pinta_char_get_value(value);
        putwchar(character);

        break;
        
    default:
        if (kind < PINTA_KIND_LENGTH)
            break;
        PINTA_THROW(PINTA_EXCEPTION_ENGINE);
    }

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_core_print_reference(PintaCore *core, PintaReference *value)
{
    pinta_assert(core != NULL);
    pinta_assert(value != NULL);

    if (value->reference == NULL)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_NULL_REFERENCE);

    return pinta_core_print(core, value->reference);
}

#endif

PintaType *pinta_core_get_object_type(PintaCore *core, PintaHeapObject *object)
{
    u8 kind;
    if (object == NULL)
        return &core->null_type;

    kind = object->block_kind;
    pinta_assert(kind < PINTA_KIND_LENGTH);

    return &core->types[kind];
}

PintaType *pinta_core_get_type(PintaCore *core, PintaReference *reference)
{
    pinta_assert(core != NULL);
    pinta_assert(reference != NULL);

    return pinta_core_get_object_type(core, reference->reference);
}

PintaException pinta_core_default_to_integer(PintaCore *core, PintaReference *value, PintaReference *result)
{
    PINTA_UNUSED(value);

    return pinta_lib_integer_alloc_value(core, 0, result);
}

PintaException pinta_core_default_to_integer_value(PintaCore *core, PintaReference *value, i32 *result)
{
    PINTA_UNUSED(core);
    PINTA_UNUSED(value);

    *result = 0;
    return PINTA_OK;
}

PintaException pinta_core_default_to_decimal(PintaCore *core, PintaReference *value, PintaReference *result)
{
    PINTA_UNUSED(value);

    return pinta_lib_decimal_alloc_zero(core, result);
}

PintaException pinta_core_default_to_string(PintaCore *core, PintaReference *value, PintaReference *result)
{
    PINTA_UNUSED(core);
    PINTA_UNUSED(value);

    pinta_assert(core != NULL);
    pinta_assert(value != NULL);
    pinta_assert(result != NULL);

    result->reference = NULL;
    return PINTA_OK;
}

PintaException pinta_core_default_to_bool(PintaCore *core, PintaReference *value, u8 *result)
{
    PINTA_UNUSED(core);
    
    pinta_assert(core != NULL);
    pinta_assert(value != NULL);
    pinta_assert(result != NULL);

    *result = 1;
    return PINTA_OK;
}

PintaException pinta_core_default_to_zero(PintaCore *core, PintaReference *result)
{
    return pinta_lib_decimal_alloc_zero(core, result);
}

PintaException pinta_core_default_get_item(PintaCore *core, PintaReference *array, u32 index, PintaReference *result)
{
    PINTA_UNUSED(core);
    PINTA_UNUSED(index);
    PINTA_UNUSED(result);

    pinta_assert(core != NULL);
    pinta_assert(array != NULL);
    pinta_assert(result != NULL);

    if (array->reference == NULL)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_NULL_REFERENCE);

    return PINTA_EXCEPTION(PINTA_EXCEPTION_INVALID_OPERATION);
}

PintaException pinta_core_default_set_item(PintaCore *core, PintaReference *array, u32 index, PintaReference *result)
{
    PINTA_UNUSED(core);
    PINTA_UNUSED(index);

    pinta_assert(core != NULL);
    pinta_assert(array != NULL);
    pinta_assert(result != NULL);

    if (array->reference == NULL)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_NULL_REFERENCE);

    return PINTA_EXCEPTION(PINTA_EXCEPTION_INVALID_OPERATION);
}

PintaException pinta_core_default_get_length(PintaCore *core, PintaReference *value, u32 *result)
{
    PINTA_UNUSED(core);
    PINTA_UNUSED(result);

    pinta_assert(core != NULL);
    pinta_assert(value != NULL);
    pinta_assert(result != NULL);

    if (value->reference == NULL)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_NULL_REFERENCE);

    return PINTA_EXCEPTION(PINTA_EXCEPTION_INVALID_OPERATION);
}

PintaException pinta_core_default_get_char(PintaCore *core, PintaReference *value, u32 index, wchar *result)
{
    PINTA_UNUSED(core);
    PINTA_UNUSED(index);
    PINTA_UNUSED(result);

    pinta_assert(core != NULL);
    pinta_assert(value != NULL);
    pinta_assert(result != NULL);

    if (value->reference == NULL)
        return PINTA_EXCEPTION(PINTA_EXCEPTION_NULL_REFERENCE);

    return PINTA_EXCEPTION(PINTA_EXCEPTION_INVALID_OPERATION);
}

PintaException pinta_core_default_copy(PintaCore *core, PintaReference *value, PintaReference *result)
{
    PINTA_UNUSED(core);

    pinta_assert(core != NULL);
    pinta_assert(value != NULL);
    pinta_assert(result != NULL);

    result->reference = value->reference;

    return PINTA_OK;
}

PintaException pinta_core_default_debug_write(PintaCore *core, PintaReference *value, u32 max_depth, PintaJsonWriter *writer)
{
    PINTA_UNUSED(core);
    PINTA_UNUSED(value);
    PINTA_UNUSED(max_depth);
    PINTA_UNUSED(writer);

    return PINTA_EXCEPTION_NOT_IMPLEMENTED;
}
