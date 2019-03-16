#include "pinta_tests.h"

u8 test_memory[64 * 1024];

u8 intersects(void *a, u32 a_length, void *b, u32  b_length)
{
    u8 *a_start, *a_end;
    u8 *b_start, *b_end;

    a_start = (u8*)a;
    a_end = &a_start[a_length];

    b_start = (u8*)b;
    b_end = &b_start[b_length];

    if (a_start >= b_start && a_start <= b_end)
        return 1;
    if (a_end >= b_start && a_end <= b_end)
        return 1;
   
    if (b_start >= a_start && b_start <= a_end)
        return 1;
    if (b_end >= a_start && b_end <= a_end)
        return 1;

    return 0;
}

PINTA_TEST_BEGIN(native_memory_alloc, 1)
{
    PintaNativeMemory *memory;
    void *a, *b, *c;

    memory = pinta_memory_init(test_memory, 64 * 1024);

    sput_fail_if(memory == NULL, "Initialized native memory");
    sput_fail_if((void*)memory < (void*)test_memory, "Native memory < provided memory");
    sput_fail_if(memory->start > memory->end, "Start < End");

    a = pinta_memory_alloc(memory, 1025);
    b = pinta_memory_alloc(memory, 5744);
    c = pinta_memory_alloc(memory, 1);

    sput_fail_if(a == NULL, "Allocated first block");
    sput_fail_if(b == NULL, "Allocated second block");
    sput_fail_if(c == NULL, "Allocated third block");

    sput_fail_if(intersects(a, 1025, b, 5744), "Blocks a and b are distinct");
    sput_fail_if(intersects(b, 5744, c, 1), "Blocks b and c are distinct");
    sput_fail_if(intersects(c, 1, a, 1025), "Blocks c and a are distinct");

    pinta_memory_free(memory, a);
    pinta_memory_free(memory, b);
    pinta_memory_free(memory, c);

    sput_fail_if(memory->free == NULL, "Free block is present");
    sput_fail_if(memory->free->data.free.next != NULL, "Only one free block is present");
    sput_fail_if(memory->free->data.free.prev != NULL, "Free block is correct");

    PINTA_RETURN();
}
PINTA_TEST_END(PINTA_OK)

PINTA_TEST_BEGIN(native_memory_free_complex, 1)
{
    PintaNativeMemory *memory;
    void *a, *b, *c;

    memory = pinta_memory_init(test_memory, 64 * 1024);

    sput_fail_if(memory == NULL, "Initialized native memory");
    sput_fail_if((void*)memory < (void*)test_memory, "Native memory < provided memory");
    sput_fail_if(memory->start > memory->end, "Start < End");

    a = pinta_memory_alloc(memory, 1025);
    b = pinta_memory_alloc(memory, 5744);
    c = pinta_memory_alloc(memory, 1);

    pinta_memory_free(memory, b);
    pinta_memory_free(memory, a);
    pinta_memory_free(memory, c);

    sput_fail_if(memory->free == NULL, "Free block is present");
    sput_fail_if(memory->free->data.free.next != NULL, "Only one free block is present");
    sput_fail_if(memory->free->data.free.prev != NULL, "Free block is correct");

    memory = pinta_memory_init(test_memory, 64 * 1024);
    a = pinta_memory_alloc(memory, 1025);
    b = pinta_memory_alloc(memory, 5744);
    c = pinta_memory_alloc(memory, 1);

    pinta_memory_free(memory, a);
    pinta_memory_free(memory, c);
    pinta_memory_free(memory, b);
   
    sput_fail_if(memory->free == NULL, "Free block is present");
    sput_fail_if(memory->free->data.free.next != NULL, "Only one free block is present");
    sput_fail_if(memory->free->data.free.prev != NULL, "Free block is correct");

    memory = pinta_memory_init(test_memory, 64 * 1024);
    a = pinta_memory_alloc(memory, 1025);
    b = pinta_memory_alloc(memory, 5744);
    c = pinta_memory_alloc(memory, 1);

    pinta_memory_free(memory, c);
    pinta_memory_free(memory, a);
    pinta_memory_free(memory, b);
   
    sput_fail_if(memory->free == NULL, "Free block is present");
    sput_fail_if(memory->free->data.free.next != NULL, "Only one free block is present");
    sput_fail_if(memory->free->data.free.prev != NULL, "Free block is correct");

    memory = pinta_memory_init(test_memory, 64 * 1024);
    a = pinta_memory_alloc(memory, 1025);
    b = pinta_memory_alloc(memory, 5744);
    c = pinta_memory_alloc(memory, 1);

    pinta_memory_free(memory, c);
    pinta_memory_free(memory, b);
    pinta_memory_free(memory, a);
   
    sput_fail_if(memory->free == NULL, "Free block is present");
    sput_fail_if(memory->free->data.free.next != NULL, "Only one free block is present");
    sput_fail_if(memory->free->data.free.prev != NULL, "Free block is correct");

    PINTA_RETURN();
}
PINTA_TEST_END(PINTA_OK)

void pinta_tests_memory()
{
    sput_enter_suite("Native memory tests");
    sput_run_test(native_memory_alloc);
    sput_run_test(native_memory_free_complex);
    sput_leave_suite();
}
