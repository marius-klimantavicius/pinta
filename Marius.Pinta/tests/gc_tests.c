#include "pinta_tests.h"

wchar *this_is_hello_world = L"This is \"Hello, World!\"";
wchar *some_data = L"Some data";

PINTA_TEST_BEGIN(no_objects_after_collection, 1)
{
	u32 alive_count;
	PintaReference *target;

	target = PINTA_GC_LOCAL(0);

	alive_count = pinta_debug_get_alive_count(core->heap);
	sput_fail_if(alive_count > 0, "No alive objects");

	pinta_core_gc(core, 0);

	PINTA_CHECK(pinta_lib_string_alloc(core, 20, target));
	sput_fail_if(target->reference == NULL, "Allocated");

	alive_count = pinta_debug_get_alive_count(core->heap);
	sput_fail_if(alive_count != 1, "Single alive object");

	target->reference = NULL;
	pinta_core_gc(core, 0);
	alive_count = pinta_debug_get_alive_count(core->heap);
	sput_fail_if(alive_count != 0, "No alive objects");
}
PINTA_TEST_END(PINTA_OK)

PINTA_TEST_BEGIN(native_stack_is_gc_root, 2)
{
	u32 alive_count;
	PintaReference *name;
	PintaReference *value;

	name = PINTA_GC_LOCAL(0);
	value = PINTA_GC_LOCAL(1);

	PINTA_CHECK(pinta_lib_string_alloc_value(core, this_is_hello_world, (u32)wcslen(this_is_hello_world), name));
	PINTA_CHECK(pinta_lib_substring_alloc(core, name, 9, 13, value));

	pinta_core_gc(core, 1);

	sput_fail_if(name->reference == NULL, "Not null reference");
	sput_fail_if(value->reference == NULL, "Not null reference");

    sput_fail_if(memcmp(this_is_hello_world, pinta_string_ref_get_data(name), wcslen(this_is_hello_world) * sizeof(wchar)) != 0, "String values equal");
	sput_fail_if(pinta_substring_ref_get_value(value) != name->reference, "Valid reference");
    sput_fail_if(pinta_substring_ref_get_length(value) != 13, "Valid length");
    sput_fail_if(pinta_substring_ref_get_offset(value) != 9, "Valid offset");

	PINTA_CHECK(pinta_lib_substring_to_string(core, value, value));

	pinta_core_gc(core, 1);
	alive_count = pinta_debug_get_alive_count(core->heap);

	sput_fail_if(alive_count != 2, "Two alive objects");
}
PINTA_TEST_END(PINTA_OK)

PINTA_TEST_BEGIN(locals_are_gc_root, 2)
{
	PintaReference *value;
    PintaReference *locals;
    PintaThread *thread = core->threads;

    locals = PINTA_GC_LOCAL(0);
    value = PINTA_GC_LOCAL(1);

	thread->frame = pinta_frame_push(thread->frame);
	sput_fail_if(thread->frame == NULL, "Stack frame pushed");

    PINTA_CHECK(pinta_lib_array_alloc(core, 1, locals));
    thread->frame->function_locals.reference = locals->reference;
    locals->reference = NULL;

    PINTA_CHECK(pinta_lib_array_get_item(core, &thread->frame->function_locals, 0, value));
	PINTA_CHECK(pinta_lib_string_alloc(core, 51, value));
    PINTA_CHECK(pinta_lib_array_set_item(core, &thread->frame->function_locals, 0, value));

	sput_fail_if(value->reference == NULL, "Allocated");
	sput_fail_if(pinta_string_ref_get_data(value) == NULL, "Non null string data");
    sput_fail_if(pinta_string_ref_get_data(value) < (wchar*)core->heap->start || pinta_string_ref_get_data(value) > (wchar*)core->heap->end, "String data within heap");

	swprintf(pinta_string_ref_get_data(value), pinta_string_ref_get_length(value), some_data);

    value->reference = NULL;

	pinta_core_gc(core, 0);
	pinta_core_gc(core, 1);

    PINTA_CHECK(pinta_lib_array_get_item(core, &thread->frame->function_locals, 0, value));
    sput_fail_if(memcmp(pinta_string_ref_get_data(value), some_data, wcslen(some_data) * sizeof(wchar)) != 0, "Valid value");
}
PINTA_TEST_END(PINTA_OK)

PINTA_TEST_BEGIN(automatic_gc_in_loop, 1)
{
	u32 counter = 0;
	PintaReference *value;

	value = PINTA_GC_LOCAL(0);

	for (counter = 0; counter < 10000; counter++)
	{
		PINTA_CHECK(pinta_lib_string_alloc(core, 27, value));
	}

	pinta_core_gc(core, 0);
	counter = pinta_debug_get_alive_count(core->heap);
	sput_fail_if(counter != 1, "Single alive object");
}
PINTA_TEST_END(PINTA_OK)

PINTA_TEST_BEGIN(pinned_should_not_move, 1)
{
	u32 alive_count = 0;
	PintaReference *pinned;
	PintaHeapObject *current;

	pinned = PINTA_GC_LOCAL(0);

	PINTA_CHECK(pinta_lib_string_alloc(core, 29, pinned));
	PINTA_CHECK(pinta_lib_string_alloc(core, 30, pinned));

	current = pinned->reference;
	pinned->reference->block_flags |= PINTA_FLAG_PINNED;

	alive_count = pinta_debug_get_alive_count(core->heap);
	sput_fail_if(alive_count != 2, "Two objects alive");

	pinta_core_gc(core, 1);

	sput_fail_if(current != pinned->reference, "Pinned object was not moved");
}
PINTA_TEST_END(PINTA_OK)

PINTA_TEST_BEGIN(multiple_pinned_should_not_move, 3)
{
	u32 alive_count;
	PintaHeapObject *obj0, *obj1;
	PintaReference *val0, *val1, *val2;

	val0 = PINTA_GC_LOCAL(0);
	val1 = PINTA_GC_LOCAL(1);
	val2 = PINTA_GC_LOCAL(2);

	PINTA_CHECK(pinta_lib_string_alloc(core, 11, val0));
	PINTA_CHECK(pinta_lib_string_alloc(core, 12, val0));
	PINTA_CHECK(pinta_lib_string_alloc(core, 13, val0));

	obj0 = val0->reference;
	val0->reference->block_flags |= PINTA_FLAG_PINNED;

	PINTA_CHECK(pinta_lib_string_alloc(core, 20, val1));
	PINTA_CHECK(pinta_lib_string_alloc(core, 23, val1));

	PINTA_CHECK(pinta_lib_string_alloc(core, 15, val2));
	PINTA_CHECK(pinta_lib_string_alloc(core, 16, val2));
	PINTA_CHECK(pinta_lib_string_alloc(core, 17, val2));

	val2->reference = NULL;

	obj1 = val1->reference;
	val1->reference->block_flags |= PINTA_FLAG_PINNED;

	pinta_core_gc(core, 0);

	alive_count = pinta_debug_get_alive_count(core->heap);
	sput_fail_if(alive_count != 2, "Two objects alive");

	pinta_core_gc(core, 1);

	sput_fail_if(val0->reference != obj0, "Object 0 was not moved");
	sput_fail_if(val1->reference != obj1, "Object 1 was not moved");
}
PINTA_TEST_END(PINTA_OK)

PINTA_TEST_BEGIN(move_unpinned_to_start, 3)
{
	PintaReference *val0, *val1, *val2;
	PintaHeapObject *obj0, *obj1, *obj2;

	val0 = PINTA_GC_LOCAL(0);
	val1 = PINTA_GC_LOCAL(1);
	val2 = PINTA_GC_LOCAL(2);

	PINTA_CHECK(pinta_lib_string_alloc(core, 11, val0));
	PINTA_CHECK(pinta_lib_string_alloc(core, 22, val0));
	PINTA_CHECK(pinta_lib_string_alloc(core, 15, val0));
	PINTA_CHECK(pinta_lib_string_alloc(core, 10, val0));

	obj0 = val0->reference;
	obj0->block_flags |= PINTA_FLAG_PINNED;

	PINTA_CHECK(pinta_lib_string_alloc(core, 15, val1));
	PINTA_CHECK(pinta_lib_string_alloc(core, 17, val2));

	obj1 = val1->reference;
	obj2 = val2->reference;

	pinta_core_gc(core, 1);

	sput_fail_if(val0->reference != obj0, "Pinned was not moved");
	sput_fail_if(val1->reference > obj1, "Object 1 was moved down");
	sput_fail_if(val2->reference > obj2, "Object 2 was moved down");
}
PINTA_TEST_END(PINTA_OK)

PINTA_TEST_BEGIN(cyclic_reference_should_be_freed, 2)
{
    PintaReference *val0, *val1;
    u32 alive_count;

    val0 = PINTA_GC_LOCAL(0);
    val1 = PINTA_GC_LOCAL(1);

    PINTA_CHECK(pinta_lib_array_alloc(core, 3, val0));
    PINTA_CHECK(pinta_lib_array_alloc(core, 3, val1));

    PINTA_CHECK(pinta_lib_array_set_item(core, val0, 0, val1));
    PINTA_CHECK(pinta_lib_array_set_item(core, val0, 1, val0));

    PINTA_CHECK(pinta_lib_array_set_item(core, val1, 0, val0));
    PINTA_CHECK(pinta_lib_array_set_item(core, val1, 1, val1));

    val0->reference = NULL;

    pinta_core_gc(core, 1);

    alive_count = pinta_debug_get_alive_count(core->heap);
    sput_fail_if(alive_count != 2, "Two objects alive");

    val1->reference = NULL;

    pinta_core_gc(core, 1);

    alive_count = pinta_debug_get_alive_count(core->heap);
    sput_fail_if(alive_count != 0, "No objects alive");
}
PINTA_TEST_END(PINTA_OK)

PINTA_TEST_BEGIN(heap_handle_should_survive_gc_and_be_moved, 1)
{
    PintaReference *value;
    PintaHeapObject *handle_object;
    PintaHeapHandle handle;

    value = PINTA_GC_LOCAL(0);

    PINTA_CHECK(pinta_lib_blob_alloc(core, 100, value));
    value->reference = NULL;

    PINTA_CHECK(pinta_heap_handle_register(core, &handle));

    PINTA_CHECK(pinta_lib_blob_alloc(core, 101, &handle.reference));

    handle_object = handle.reference.reference;
    pinta_core_gc(core, 1);

    sput_fail_if(handle_object == handle.reference.reference, "Was not moved");
    sput_fail_if(pinta_blob_get_length(handle.reference.reference) != 101, "Incorrect object");

    pinta_heap_handle_unregister(core, &handle);
}
PINTA_TEST_END(PINTA_OK);

void pinta_tests_gc()
{
	sput_enter_suite("GC tests");
	sput_run_test(no_objects_after_collection);
	sput_run_test(native_stack_is_gc_root);
	sput_run_test(locals_are_gc_root);
	sput_run_test(automatic_gc_in_loop);
	sput_run_test(pinned_should_not_move);
	sput_run_test(multiple_pinned_should_not_move);
	sput_run_test(move_unpinned_to_start);
    sput_run_test(cyclic_reference_should_be_freed);
    sput_run_test(heap_handle_should_survive_gc_and_be_moved);
	sput_leave_suite();
}
