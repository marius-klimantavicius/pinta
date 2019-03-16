#ifndef PINTA_TESTS_H
#define PINTA_TESTS_H

#include "pinta.h"
#include "pinta-api.h"

#if defined(_MSC_VER)
#pragma warning(disable: 4127)
#endif

#include "sput.h"

extern PintaApi *api;
extern PintaCore *core;

#define PINTA_GC_LOCAL(index) &__frame_references[index]

#define PINTA_GC_INIT(count) \
    do { \
    PintaNativeFrame __frame; \
    PintaReference __frame_references[count]; \
    __frame.references = __frame_references; \
    __frame.length = count; \
    __frame.next = core->native; \
    core->native = &__frame; \
    memset(__frame.references, 0, sizeof(__frame_references));

#define PINTA_TEST_BEGIN(name, count) void name() \
{ \
    PintaException exception = PINTA_OK; \
    pinta_init_tests(30 * 1024, 1024); \
    PINTA_GC_INIT(count);

#define PINTA_TEST_BEGIN_MEMORY(name, count, heap_size, stack_size) void name() \
{ \
    PintaException exception = PINTA_OK; \
    pinta_init_tests(heap_size, stack_size); \
    PINTA_GC_INIT(count);

#define PINTA_TEST_END(expected_exception) PINTA_EXIT: \
    PINTA_GC_EXIT(core); \
    pinta_dispose_tests(); \
    sput_fail_if(exception != (expected_exception), pinta_tests_message(L"Expected result - %d, actual result - %d", expected_exception, exception)); \
}

void pinta_init_tests(int heap_size, int stack_size);
void pinta_dispose_tests(void);

void pinta_tests(void);
void pinta_tests_gc(void);
void pinta_tests_array(void);
void pinta_tests_memory(void);
void pinta_tests_code(void);
void pinta_tests_code_v2(void);
void pinta_tests_buffer(void);
void pinta_tests_format(void);
void pinta_tests_integer(void);
void pinta_tests_decimal(void);
void pinta_tests_weak(void);
void pinta_tests_encoding(void);
void pinta_tests_property(void);
void pinta_tests_object(void);
void pinta_tests_function(void);
void pinta_tests_pattern(void);
void pinta_tests_json(void);

char *pinta_tests_message(const wchar *format, ...);

PintaException pinta_test_load_module(PintaCore *core, wchar *filename, PintaModuleDomain **result);
PintaException pinta_test_execute_module(PintaCore *core, wchar *filename, PintaModuleDomain **result);
PintaException pinta_test_compare_equal(PintaReference *left, PintaReference *right, i32 *compare_result);
PintaException pinta_test_internal_print(PintaCore *core, PintaReference *arguments, PintaReference *return_value);
PintaException pinta_test_internal_assert(PintaCore *core, PintaReference *arguments, PintaReference *return_value);
PintaException pinta_test_internal_integer_eq_6(PintaCore *core, PintaReference *arguments, PintaReference *return_value);
PintaException pinta_test_internal_strings_eq_hi(PintaCore *core, PintaReference *arguments, PintaReference *return_value);
PintaException pinta_test_internal_decimal_eq_pi(PintaCore *core, PintaReference *arguments, PintaReference *return_value);
PintaException pinta_test_internal_unreachable(PintaCore *core, PintaReference *arguments, PintaReference *return_value);
PintaException pinta_test_set_global(PintaCore *core, PintaModuleDomain *domain, wchar *nameValue, wchar *valueValue);
PintaException pinta_test_expect_buffer(PintaCore *core, PintaReference *arguments, PintaReference *return_value);
PintaException pinta_test_global_function(PintaCore *core, PintaReference *function, PintaReference *function_this, PintaReference *function_arguments, PintaReference *return_value);

void *pinta_platform_file_open(void *context, void *name_data, uint32_t name_length);
uint32_t pinta_platform_file_size(void *context, void *handle);
uint32_t pinta_platform_file_read(void *context, void *handle, void *buffer, uint32_t length);
void pinta_platform_file_close(void *context, void *handle);
uint32_t pinta_platform_file_write(wchar_t *filename, void *data, uint32_t data_length);

#endif // PINTA_TESTS_H
