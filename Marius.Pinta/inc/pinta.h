#ifndef PINTA_H
#define PINTA_H

#ifdef __cplusplus
extern "C" {
#endif
    
#include <assert.h>
#include <stdint.h>
#include <string.h>
    
#if !defined(PINTA_DEBUG)

#if _DEBUG
#define PINTA_DEBUG 1
#else
#define PINTA_DEBUG 0
#endif

#endif /* !defined(PINTA_DEBUG) */

#if !defined(WCHAR_MAX)
#error "WCHAR_MAX is needed"
#endif

#if !defined(PINTA_HAVE_LONG_LITERAL)
#define PINTA_HAVE_LONG_LITERAL         1
#endif /* !defined(PINTA_HAVE_LONG_LITERAL) */

#if PINTA_HAVE_LONG_LITERAL
#define PINTA_CHAR(x)                   L ## x
#define PINTA_STRING(x)                 L ## x
#else
#define PINTA_CHAR(x)                   ((wchar)(x))
#define PINTA_STRING(x)                 ((wchar*)(x))
#endif /* PINTA_HAVE_LONG_LITERAL */

#define PINTA_LITERAL_LENGTH(literal)   ((sizeof(literal) / sizeof(wchar)) - 1)

#define PINTA_UNUSED(x)                 ((void)x)

#if PINTA_DEBUG
#define pinta_assert(_Expression)       assert(_Expression)
#define PINTA_EXCEPTION(_exception)     (_exception)
#else
#define pinta_assert(_Expression)       
#define PINTA_EXCEPTION(_exception)     (_exception)
#endif

#define pinta_trace0(message)               ((void)0)
#define pinta_trace1(format, arg0)          ((void)0)
#define pinta_trace2(format, arg0, arg1)    ((void)0)

// let the hacking begin

#define PINTA_GC_ENTER(core, gc)                            \
    do {                                                    \
    PintaNativeFrame __frame;                               \
    __frame.references = (PintaReference*)&(gc);            \
    __frame.length = sizeof(gc) / sizeof(PintaReference);   \
    __frame.next = (core)->native;                          \
    (core)->native = &__frame;                              \
    memset(__frame.references, 0, sizeof(gc))

#if defined(_MSC_VER) && _MSC_VER < 1910
#define PINTA_GC_EXIT(core)                                 \
    (core)->native = __frame.next;                          \
    } while(0, 0)
#else
#define PINTA_GC_EXIT(core)                                 \
    (core)->native = __frame.next;                          \
    } while(0)
#endif

#define PINTA_GC_RETURN(core) PINTA_EXIT:                   \
    PINTA_GC_EXIT(core);                                    \
    return (exception)

    // as I am compiling with warnings as errors I need to disable C4127 on msvc, so I use microsoft-specific workaround (0, 0)
#if defined(_MSC_VER) && _MSC_VER < 1910
#define PINTA_CHECK(x)                  do { if ((exception = (x)) != PINTA_OK) { goto PINTA_EXIT; } } while(0, 0)
#define PINTA_THROW(x)                  do { exception = (x); goto PINTA_EXIT; } while(0, 0)
#define PINTA_RETURN()                  do { exception = PINTA_OK; goto PINTA_EXIT; } while(0, 0)
#else
#define PINTA_CHECK(x)                  do { if ((exception = (x)) != PINTA_OK) { goto PINTA_EXIT; } } while(0)
#define PINTA_THROW(x)                  do { exception = (x); goto PINTA_EXIT; } while(0)
#define PINTA_RETURN()                  do { exception = PINTA_OK; goto PINTA_EXIT; } while(0)
#endif // defined(_MSC_VER)

#define PINTA_DECIMAL_DIGITS            25
#define PINTA_DECIMAL_SCALE             100000000LL
#define PINTA_DECIMAL_ROUNDER           50000000LL
#define PINTA_DECIMAL_LIMIT             10000000000000000LL
#define PINTA_DECIMAL_PRECISION         8

#define PINTA_MULTISTRING_COUNT         (sizeof(PintaHeapObject) / sizeof(PintaHeapObject*))

#define U16_MAX                         UINT16_MAX
#define PINTA_INT_DIGITS                11

#define PINTA_DECOMPRESS_MAX_STACK      8


typedef enum PintaFlags
{
    PINTA_FLAG_NONE                     = 0x00,
    PINTA_FLAG_ALIVE                    = 0x10,
    PINTA_FLAG_ZERO                     = 0x20,
    PINTA_FLAG_KEEP_ALIVE               = 0x40,
    PINTA_FLAG_PINNED                   = 0x80
} PintaFlags;

typedef enum PintaKind
{
    PINTA_KIND_INTEGER                  = 0x00,
    PINTA_KIND_DECIMAL                  = 0x01,
    PINTA_KIND_STRING                   = 0x02,
    PINTA_KIND_SUBSTRING                = 0x03,
    PINTA_KIND_MULTISTRING              = 0x04,
    PINTA_KIND_CHAR                     = 0x05,
    PINTA_KIND_ARRAY                    = 0x06,
    PINTA_KIND_OBJECT                   = 0x07,
    PINTA_KIND_BUFFER                   = 0x08,
    PINTA_KIND_BLOB                     = 0x09,
	PINTA_KIND_WEAK                     = 0x0A,
    PINTA_KIND_FUNCTION_MANAGED         = 0x0B,
    PINTA_KIND_FUNCTION_NATIVE          = 0x0C,
    PINTA_KIND_PROPERTY_TABLE           = 0x0D,
    PINTA_KIND_GLOBAL_OBJECT            = 0x0E,
    PINTA_KIND_LENGTH                         ,
    PINTA_KIND_FREE                     = 0xFF
} PintaKind;

typedef enum PintaException
{
    PINTA_OK                            = 0x00000000,
    PINTA_EXCEPTION_STACK_OVERFLOW      = 0x00000001,
    PINTA_EXCEPTION_STACK_UNDERFLOW     = 0x00000002,
    PINTA_EXCEPTION_TYPE_MISMATCH       = 0x00000003,
    PINTA_EXCEPTION_OUT_OF_MEMORY       = 0x00000004,
    PINTA_EXCEPTION_NULL_REFERENCE      = 0x00000005,
    PINTA_EXCEPTION_BAD_FORMAT          = 0x00000006,
    PINTA_EXCEPTION_OUT_OF_RANGE        = 0x00000007,
    PINTA_EXCEPTION_INVALID_OPERATION   = 0x00000008,
    PINTA_EXCEPTION_INVALID_OPCODE      = 0x00000009,
    PINTA_EXCEPTION_NOT_IMPLEMENTED     = 0x0000000A,
    PINTA_EXCEPTION_ENGINE              = 0x0000000B,
    PINTA_EXCEPTION_INDEX_OUT_OF_BOUNDS = 0x0000000C,
    PINTA_EXCEPTION_DIVISION_BY_ZERO    = 0x0000000D,
    PINTA_EXCEPTION_INVALID_MODULE      = 0x0000000E,
    PINTA_EXCEPTION_FILE_NOT_FOUND      = 0x0000000F,
    PINTA_EXCEPTION_INVALID_ARGUMENTS   = 0x00000010,
    PINTA_EXCEPTION_PLATFORM            = 0x00000011,
    PINTA_EXCEPTION_NOT_REACHABLE       = 0x00000012,
    PINTA_EXCEPTION_INVALID_SEQUENCE    = 0x00000013
} PintaException;

typedef enum PintaCode
{
    PINTA_CODE_NOP                      = 0x00,
    PINTA_CODE_ADD                      = 0x01,
    PINTA_CODE_SUBTRACT                 = 0x02,
    PINTA_CODE_MULTIPLY                 = 0x03,
    PINTA_CODE_DIVIDE                   = 0x04,
    PINTA_CODE_REMAINDER                = 0x05,
    PINTA_CODE_BITWISE_AND              = 0x06,
    PINTA_CODE_BITWISE_OR               = 0x07,
    PINTA_CODE_EXCLUSIVE_OR             = 0x08,
    PINTA_CODE_BITWISE_EXCLUSIVE_OR     = 0x09,
    PINTA_CODE_NOT                      = 0x0A,
    PINTA_CODE_BITWISE_NOT              = 0x0B,
    PINTA_CODE_NEGATE                   = 0x0C,
    PINTA_CODE_COMPARE_EQUAL            = 0x0D,
    PINTA_CODE_COMPARE_LESS_THAN        = 0x0E,
    PINTA_CODE_COMPARE_MORE_THAN        = 0x0F,
    PINTA_CODE_COMPARE_NULL             = 0x10,
    PINTA_CODE_CONVERT_INTEGER          = 0x11,
    PINTA_CODE_CONVERT_DECIMAL          = 0x12,
    PINTA_CODE_CONVERT_STRING           = 0x13,
    PINTA_CODE_NEW_ARRAY                = 0x14,
    PINTA_CODE_CONCAT                   = 0x15,
    PINTA_CODE_SUBSTRING                = 0x16,
    PINTA_CODE_JUMP                     = 0x17,
    PINTA_CODE_JUMP_ZERO                = 0x18,
    PINTA_CODE_JUMP_NOT_ZERO            = 0x19,
    PINTA_CODE_CALL                     = 0x1A,
    PINTA_CODE_CALL_INTERNAL            = 0x1B,
    PINTA_CODE_RETURN                   = 0x1C,
    PINTA_CODE_LOAD_NULL                = 0x1D,
    PINTA_CODE_LOAD_INTEGER_ZERO        = 0x1E,
    PINTA_CODE_LOAD_DECIMAL_ZERO        = 0x1F,
    PINTA_CODE_LOAD_INTEGER_ONE         = 0x20,
    PINTA_CODE_LOAD_DECIMAL_ONE         = 0x21,
    PINTA_CODE_LOAD_INTEGER             = 0x22,
    PINTA_CODE_LOAD_STRING              = 0x23,
    PINTA_CODE_STORE_LOCAL              = 0x24,
    PINTA_CODE_STORE_GLOBAL             = 0x25,
    PINTA_CODE_STORE_ARGUMENT           = 0x26,
    PINTA_CODE_STORE_ITEM               = 0x27,
    PINTA_CODE_LOAD_LOCAL               = 0x28,
    PINTA_CODE_LOAD_GLOBAL              = 0x29,
    PINTA_CODE_LOAD_ARGUMENT            = 0x2A,
    PINTA_CODE_LOAD_ITEM                = 0x2B,
    PINTA_CODE_DUPLICATE                = 0x2C,
    PINTA_CODE_POP                      = 0x2D,
    PINTA_CODE_EXIT                     = 0x2E,
    PINTA_CODE_GET_LENGTH               = 0x2F,
    PINTA_CODE_ERROR                    = 0x30,
    PINTA_CODE_LOAD_BLOB                = 0x31,
    PINTA_CODE_LOAD_CLOSURE             = 0x32,
    PINTA_CODE_LOAD_MEMBER              = 0x33,
    PINTA_CODE_LOAD_THIS                = 0x34,
    PINTA_CODE_STORE_CLOSURE            = 0x35,
    PINTA_CODE_STORE_MEMBER             = 0x36,
    PINTA_CODE_NEW                      = 0x37,
    PINTA_CODE_NEW_FUNCTION             = 0x38,
    PINTA_CODE_NEW_OBJECT               = 0x39,
    PINTA_CODE_INVOKE                   = 0x3A,
    PINTA_CODE_INVOKE_MEMBER            = 0x3B,
    PINTA_CODE_DUPLICATE_MULTIPLE       = 0x3C,
    PINTA_CODE_MATCH                    = 0x3D,
    PINTA_CODE_BREAK                    = 0x3E, // TODO, maybe 0xFF?
    PINTA_CODE_LENGTH
} PintaCode;

typedef enum PintaCodeMatchType
{
    PINTA_CODE_MATCH_TYPE_SIMPLE        = 0x00,
    PINTA_CODE_MATCH_TYPE_PAN           = 0x10,
} PintaCodeMatchType;

#define PINTA_CODE_TOKEN_EMPTY          0xFFFFFFFF
#define PINTA_CODE_MODULE_MAGIC         0x50496E74
#define PINTA_MEMORY_COOKIE_ALIVE       0x616C6976
#define PINTA_MEMORY_COOKIE_FREE        0x64656164

#define PINTA_JSON_LENGTH_AUTO          0xFFFFFFFF

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;

typedef uint64_t u64;
typedef int64_t i64;

#if WCHAR_MAX > 0xFFFF || WCHAR_MAX <= 0xFF
typedef uint16_t wchar;
#else
typedef wchar_t wchar;
#endif

typedef i64 decimal;

typedef struct PintaHeap PintaHeap;
typedef struct PintaHeapHandle PintaHeapHandle;
typedef struct PintaReference PintaReference;
typedef struct PintaHeapReloc PintaHeapReloc;

typedef struct PintaType PintaType;

typedef struct PintaHeapObject PintaHeapObject;
typedef struct PintaInteger PintaInteger;
typedef struct PintaDecimal PintaDecimal;
typedef struct PintaArray PintaArray;
typedef struct PintaString PintaString;
typedef struct PintaMultiString PintaMultiString;
typedef struct PintaSubstring PintaSubstring;
typedef struct PintaBuffer PintaBuffer;
typedef struct PintaBufferBlob PintaBufferBlob;
typedef struct PintaBlob PintaBlob;
typedef struct PintaWeak PintaWeak;
typedef struct PintaCharacter PintaCharacter;
typedef struct PintaFunctionManaged PintaFunctionManaged;
typedef struct PintaFunctionNative PintaFunctionNative;
typedef struct PintaFunctionBody PintaFunctionBody;
typedef struct PintaObject PintaObject;
typedef struct PintaPropertyTable PintaPropertyTable;
typedef struct PintaProperty PintaProperty;
typedef struct PintaPropertyValue PintaPropertyValue;
typedef struct PintaPropertyAccessor PintaPropertyAccessor;
typedef struct PintaPropertyNative PintaPropertyNative;
typedef struct PintaPropertySlot PintaPropertySlot;
typedef struct PintaGlobalObject PintaGlobalObject;
typedef struct PintaFree PintaFree;

typedef struct PintaHeapCache PintaHeapCache;

typedef struct PintaCore PintaCore;
typedef struct PintaModuleDomain PintaModuleDomain;
typedef struct PintaThread PintaThread;
typedef struct PintaStackFrame PintaStackFrame;
typedef struct PintaNativeFrame PintaNativeFrame;

typedef struct PintaFormat PintaFormat;
typedef struct PintaFormatOptions PintaFormatOptions;

typedef struct PintaHeapObjectWalkerState PintaHeapObjectWalkerState;

typedef struct PintaModule PintaModule;
typedef struct PintaModuleString PintaModuleString;
typedef struct PintaModuleBlob PintaModuleBlob;
typedef struct PintaModuleGlobal PintaModuleGlobal;
typedef struct PintaModuleFunction PintaModuleFunction;

typedef struct PintaCodeString PintaCodeString;
typedef struct PintaCodeBlob PintaCodeBlob;
typedef struct PintaCodeFunction PintaCodeFunction;

typedef struct PintaReader PintaReader;
typedef struct PintaWriter PintaWriter;
typedef struct PintaNativeMemory PintaNativeMemory;
typedef struct PintaNativeBlock PintaNativeBlock;
typedef struct PintaNativeFree PintaNativeFree;

typedef struct PintaDecompressor PintaDecompressor;
typedef struct PintaDecompressState PintaDecompressState;
typedef struct PintaPlatformFileData PintaPlatformFileData;

typedef struct PintaEnvironment PintaEnvironment;

typedef struct PintaDebugger PintaDebugger;
typedef struct PintaJsonWriter PintaJsonWriter;

typedef u32(*PintaHeapObjectWalker)(PintaHeapObject *object, PintaHeapObjectWalkerState *state, PintaHeapObject **result);
typedef void(*PintaHeapObjectRelocate)(PintaHeapObject *object, PintaHeapReloc *reloc, u32 count);

typedef PintaException(*PintaObjectToZero)(PintaCore *core, PintaReference *result);
typedef PintaException(*PintaObjectFunction)(PintaCore *core, PintaReference *value, PintaReference *result);
typedef PintaException(*PintaObjectToInteger)(PintaCore *core, PintaReference *value, i32 *result);
typedef PintaException(*PintaObjectToBool)(PintaCore *core, PintaReference *value, u8 *result);
typedef PintaException(*PintaObjectIndexFunction)(PintaCore *core, PintaReference *array, u32 index, PintaReference *value);

typedef PintaException(*PintaObjectGetLengthValue)(PintaCore *core, PintaReference *array, u32 *result);
typedef PintaException(*PintaObjectGetCharValue)(PintaCore *core, PintaReference *array, u32 index, wchar *result);

typedef PintaException(*PintaObjectGetMember)(PintaCore *core, PintaReference *object, PintaReference *name, u8 *is_accessor, PintaReference *result);
typedef PintaException(*PintaObjectSetMember)(PintaCore *core, PintaReference *object, PintaReference *name, PintaReference *value, u8 *is_accessor, PintaReference *result);

typedef PintaException(*PintaDebugWrite)(PintaCore *core, PintaReference *value, u32 max_depth, PintaJsonWriter *writer);

typedef PintaException(*PintaCoreInternalFunction)(PintaCore *core, PintaReference *arguments, PintaReference *return_value);
typedef PintaException(*PintaFunctionDelegate)(PintaCore *core, PintaThread *thread, PintaReference *function, PintaReference *function_this, PintaReference *function_arguments, u8 *discard_return_value, PintaReference *return_value);

typedef PintaException(*PintaFrameTransform)(PintaThread *thread, PintaReference *value);

typedef PintaException(*PintaPropertyNativeDelegate)(PintaCore *core, PintaReference *object, PintaReference *name, u32 native_token, u8 is_set, PintaReference *value);

typedef PintaException(*PintaFileOpen)(PintaCore *core, PintaReference *file_name, void **file_handle);
typedef PintaException(*PintaFileSize)(PintaCore *core, void *file_handle, u32 *file_size);
typedef PintaException(*PintaFileRead)(PintaCore *core, void *file_handle, PintaReference *buffer, u32 length, u32 *actual_read);
typedef PintaException(*PintaFileClose)(PintaCore *core, void *file_handle);
typedef PintaException(*PintaLoadModule)(PintaCore *core, PintaReference *module_name, PintaModuleDomain **domain);

typedef void(*PintaDebugOnTick)(PintaCore *core);
typedef void(*PintaDebugOnExit)(PintaCore *core, PintaException exception);
typedef void(*PintaDebugOnDomain)(PintaCore *core, PintaModule *module, PintaModuleDomain *domain);
typedef void(*PintaDebugOnStep)(PintaCore *core, PintaThread *thread);
typedef void(*PintaDebugOnException)(PintaCore *core, PintaThread *thread, PintaException exception);
typedef void(*PintaDebugOnBeforeCall)(PintaCore *core, PintaThread *thread, u32 token, u32 arguments_count);
typedef void(*PintaDebugOnAfterCall)(PintaCore *core, PintaThread *thread);
typedef void(*PintaDebugOnBeforeInvoke)(PintaCore *core, PintaThread *thread, u32 arguments_count, u8 has_this);
typedef void(*PintaDebugOnAfterInvoke)(PintaCore *core, PintaThread *thread);
typedef void(*PintaDebugOnBeforeReturn)(PintaCore *core, PintaThread *thread);
typedef void(*PintaDebugOnAfterReturn)(PintaCore *core, PintaThread *thread);
typedef void(*PintaDebugOnBreak)(PintaCore *core, PintaThread *thread, u8 *code);

static const u32 PINTA_PROPERTY_CONFIGURABLE = 0x80000000;
static const u32 PINTA_PROPERTY_ENUMERABLE = 0x40000000;
static const u32 PINTA_PROPERTY_WRITEABLE_OR_NATIVE = 0x20000000;
static const u32 PINTA_PROPERTY_ACCESSOR = 0x10000000;

static const u32 PINTA_PROPERTY_MASK = 0xF0000000;

static const u8 PINTA_CODE_MATCH_TYPE_MASK = 0xF0;
static const u8 PINTA_CODE_MATCH_TYPE_BITS = 4;
static const u8 PINTA_CODE_MATCH_ARGUMENTS_MASK = 0x0F;
static const u8 PINTA_CODE_MATCH_ARGUMENTS_BITS = 4;

// heap block represents single entry in heap
struct PintaInteger
{
    i32 integer_value;
};

struct PintaDecimal
{
    decimal decimal_value;
};

struct PintaArray
{
    u32 array_length;
};

struct PintaString
{
    u32 string_length;
    wchar *string_data;        // explicit pointer to data (though it might be just after the block) but it can also be from const pool
};

struct PintaMultiString
{
    PintaHeapObject **parts;    // contains 4 parts (same as with string.data - it is a pointer  to blocks)
};

struct PintaSubstring
{
    u16 string_offset;
    u16 string_length;
    PintaHeapObject *string_reference;
};

struct PintaBuffer
{
    u32 buffer_length;
    u32 buffer_position;
};

struct PintaBufferBlob
{
    PintaHeapObject *buffer_blob;
};

struct PintaCharacter
{
    wchar character_value;
};

struct PintaBlob
{
    u32 blob_length;
    u8 *blob_data;
};

struct PintaWeak
{
    PintaHeapObject *target;
    PintaHeapObject *next;
};

struct PintaFunctionManaged
{
    PintaModuleDomain *function_domain;
    u32 function_token;
};

struct PintaFunctionNative
{
    PintaFunctionDelegate function_delegate;
    u32 function_tag;
};

struct PintaFunctionBody
{
    PintaHeapObject *function_closure;
    PintaHeapObject *function_binding;

    PintaHeapObject *function_object;
    PintaHeapObject *function_prototype;
};

struct PintaObject
{
    PintaHeapObject *object_prototype;
    PintaHeapObject *object_body;
};

struct PintaPropertyTable
{
    u32 table_capacity;
    u32 table_length;
};

struct PintaPropertyValue
{
    PintaHeapObject *value;
};

struct PintaPropertyAccessor
{
    PintaHeapObject *get;
    PintaHeapObject *set;
};

struct PintaPropertyNative
{
    PintaPropertyNativeDelegate native_delegate;
    u32 native_token;
};

struct PintaProperty
{
    u32 key_hash_code;
    PintaHeapObject *key;

    union PintaPropertyTableValue
    {
        PintaPropertyValue data;
        PintaPropertyAccessor accessor;
        PintaPropertyNative native;
    } value;
};

struct PintaFormat
{
    i32 format_end;
    i32 width;
    i32 precision;

    u8 is_right_padding;
    u8 is_variable_width;
    u8 is_variable_precision;
    u8 padding_1;

    wchar print_decimal;
    wchar print_sign;
    wchar plus;
    wchar pad;
    wchar specifier;
    wchar padding_2;
};

struct PintaFormatOptions
{
    PintaObjectFunction convert_decimal;
    PintaObjectFunction convert_string;
    PintaObjectFunction convert_integer;
};

struct PintaNativeMemory
{
    PintaNativeBlock *start;
    PintaNativeBlock *end;
    PintaNativeBlock *top;

    PintaNativeBlock *free;
};

struct PintaNativeFree
{
    PintaNativeBlock *prev;
    PintaNativeBlock *next;
};

struct PintaNativeBlock
{
    u32 cookie;
    u32 length;

    union PintaNativeData
    {
        void *memory;
        PintaNativeFree free;
    } data;
};

struct PintaReader
{
    u8 *start;
    u8 *end;

    u8 *current;
};

struct PintaWriter
{
    u8 *start;
    u8 *end;
    
    u8 *current;
};

struct PintaPlatformFileData
{
    u32 length;
    void *data;
};

struct PintaPropertySlot
{
    u32 is_valid : 1;
    u32 is_found : 1;
    u32 is_enumerable : 1;
    u32 is_configurable : 1;
    u32 is_writeable : 1;
    u32 is_accessor : 1;
    u32 is_native : 1;

    u32 property_id;
};

struct PintaGlobalObject
{
    PintaModuleDomain *global_domain;
    PintaHeapObject *global_body;
};

struct PintaFree
{
    PintaHeapObject *next;
    PintaHeapObject *prev;
};

struct PintaHeapObject
{
    u8 block_kind;                  // PINTA_KIND_*
    u8 block_flags;                 // PINTA_FLAG_*
    u16 block_tag;
    u32 block_length;               // length in blocks (sizeof(PintaHeapObject)) NOT bytes

    union PintaHeapObjectData
    {
        PintaInteger integer;
        PintaDecimal decimal;

        PintaArray array;
        PintaString string;
        PintaMultiString multistring;
        PintaSubstring substring;
        PintaCharacter character;
        PintaBuffer buffer;
        PintaBlob blob;
        PintaWeak weak;
        PintaFunctionManaged function_managed;
        PintaFunctionNative function_native;
        PintaObject object;
        PintaPropertyTable property_table;
        PintaGlobalObject global_object;

        PintaFree free;

    } data;
};

struct PintaType
{
    u32 is_string : 1;

    PintaHeapObjectWalker gc_walk;
    PintaHeapObjectRelocate gc_relocate;

    PintaObjectFunction to_integer;
    PintaObjectToInteger to_integer_value;
    PintaObjectFunction to_decimal;
    PintaObjectFunction to_string;
    PintaObjectFunction to_numeric;
    PintaObjectToBool to_bool;
    PintaObjectToZero to_zero;

    PintaObjectIndexFunction get_item;
    PintaObjectIndexFunction set_item;

    PintaObjectGetMember get_member;
    PintaObjectSetMember set_member;

    PintaObjectGetLengthValue get_length;
    PintaObjectGetCharValue get_char;

    PintaDebugWrite debug_write;
};

struct PintaHeapObjectWalkerState
{
    u32 index;
    u32 field;
};

struct PintaHeap
{
    PintaHeapObject *start;
    PintaHeapObject *top;          // the heap is valid only up to top, invariants: start <= top <= end
    PintaHeapObject *end;

    PintaHeapObject *free;
    u32 largest_free_block_length;

    PintaHeap *prev;
    PintaHeap *next;
};

struct PintaReference
{
    PintaHeapObject *reference;
};

struct PintaHeapHandle
{
    PintaReference reference;
    PintaHeapHandle *prev;
    PintaHeapHandle *next;
};

struct PintaStackFrame
{
    u8 *return_address;
    PintaModuleDomain *return_domain;

    u8 *code_start;
    u8 *code_end;

    u8 is_final_frame;
    u8 padding_1;
    u8 padding_2;

    u8 discard_result;
    PintaFrameTransform transform_result;
    u32 transform_tag;

    PintaReference *stack_start;
    PintaReference *stack_end;
    PintaReference *stack;

    PintaReference function_closure;
    PintaReference function_this;
    PintaReference function_arguments;
    PintaReference function_locals;

    PintaStackFrame *next;
    PintaStackFrame *prev;
};

struct PintaNativeFrame
{
    PintaReference *references;
    u32 length;

    PintaNativeFrame *next;
};

struct PintaThread
{
    PintaCore *core;
    PintaModuleDomain *domain;

    u32 code_is_suspended;
    u32 code_finished;
    u32 code_result;
    u8 *code_pointer;
    u8 *code_next_pointer;

    PintaStackFrame *frame;

    PintaThread *next;
    PintaThread *prev;
};

struct PintaHeapCache
{
    PintaHeapObject integers[101]; // -1 .. 99
    PintaHeapObject chars[128];
    PintaHeapObject array_empty[2]; // length of 2 because array is always of length 2, there is ONLY 1 object
    PintaHeapObject string_empty[2]; // length of 2 because string is always of length 2, there is ONLY 1 object
    PintaHeapObject property_table_empty[1];
};

struct PintaModuleDomain
{
    PintaModule *module;

    PintaReference global_object;

    PintaReference globals;
    PintaReference strings;

    PintaModuleDomain *prev;
    PintaModuleDomain *next;
};

struct PintaCore
{
    PintaNativeMemory *memory;
    PintaEnvironment *environment;
    PintaDebugger *debugger;

    PintaHeap *heap;
    PintaThread *threads;
    PintaModuleDomain *domains;

    PintaNativeFrame *native;
    PintaHeapHandle *heap_handles;

    PintaType null_type;
    PintaType types[PINTA_KIND_LENGTH];

    PintaReference weak;
    PintaReference externals;
    PintaReference output;
    PintaReference null;

    PintaHeapCache *cache;

    PintaCoreInternalFunction *internal_functions;
    u32 internal_functions_length;
};

struct PintaHeapReloc
{
    PintaHeapObject *start;
    PintaHeapObject *end;

    u32 offset;
};

struct PintaModule
{
    u32 magic;
    u32 flags;

    u8 signature[32];

    u32 strings_length;
    u32 strings_offset;

    u32 blobs_length;
    u32 blobs_offset;

    u32 globals_length;
    u32 globals_offset;

    u32 functions_length;
    u32 functions_offset;

    u32 start_function_token;

    u32 data_length;
    u32 data_offset;
};

struct PintaModuleString
{
    u32 string_offset;
};

struct PintaModuleBlob
{
    u32 blob_offset;
};

struct PintaModuleGlobal
{
    u32 string_token;
};

struct PintaModuleFunction
{
    u32 string_name_token;

    u32 arguments_count;
    u32 locals_count;

    u32 code_length;
    u32 code_offset;
};

struct PintaCodeString
{
    u32 string_length;
    u8 *string_data;
};

struct PintaCodeBlob
{
    u32 blob_length;
    u8 *blob_data;
};

struct PintaCodeFunction
{
    u32 string_name_token;

    u32 arguments_count;
    u32 locals_count;

    u8 *code_start;
    u8 *code_end;
};

struct PintaDecompressState
{
    u8 *data;
    u32 length;
};

struct PintaDecompressor
{
    PintaDecompressState stack[PINTA_DECOMPRESS_MAX_STACK];
    PintaDecompressState *stack_start;
    PintaDecompressState *stack_top;
    PintaDecompressState *stack_end;
};

struct PintaEnvironment
{
    void *native_environment;

    PintaFileOpen file_open;
    PintaFileSize file_size;
    PintaFileRead file_read;
    PintaFileClose file_close;
    PintaLoadModule load_module;
};

struct PintaDebugger
{
    void *session;

    PintaDebugOnTick on_tick;
    PintaDebugOnExit on_exit;
    PintaDebugOnDomain on_domain;
    PintaDebugOnStep on_step;
    PintaDebugOnException on_exception;
    PintaDebugOnBeforeCall on_before_call;
    PintaDebugOnAfterCall on_after_call;
    PintaDebugOnBeforeCall on_before_call_internal;
    PintaDebugOnAfterCall on_after_call_internal;
    PintaDebugOnBeforeInvoke on_before_invoke;
    PintaDebugOnAfterInvoke on_after_invoke;
    PintaDebugOnBeforeReturn on_before_return;
    PintaDebugOnAfterReturn on_after_return;
    PintaDebugOnBreak on_break;
};

struct PintaJsonWriter
{
    PintaException(*write)(PintaCore *core, PintaJsonWriter *writer, wchar *value, u32 length);
    void *data;

    // .......Tnv
    // T - 0: object, 1: array
    // n - name was emitted (0 for arrays)
    // v - value was emitted
    u32 stack;
};

// ***********************************************
// Utility functions
// ***********************************************
u8                  char_is_whitespace(wchar c);

u32                 string_get_length(wchar *data);
u32                 string_get_length_c(char *data);
wchar              *string_find_char(wchar *s, wchar c);

u32                 integer_get_digit_count(u32 value);
u32                 integer_to_string(i32 value, wchar *string);
u32                 long_to_string(i64 value, wchar *string);

// ***********************************************
// Compression
// ***********************************************

void                pinta_decompress_init(PintaDecompressor *decompressor, u8 *data, u32 length);
void                pinta_decompress(u8 *data, u32 length, u8 *destination);

// ***********************************************
// Native memory
// ***********************************************

PintaNativeMemory  *pinta_memory_init(void *memory, u32 length);
PintaNativeBlock   *pinta_memory_remove(PintaNativeMemory *memory, PintaNativeBlock *item);
void               *pinta_memory_alloc(PintaNativeMemory *memory, u32 length);
void                pinta_memory_free(PintaNativeMemory *memory, void *item);
void                pinta_memory_insert(PintaNativeMemory *memory, PintaNativeBlock *location, PintaNativeBlock *item, u32 block_length);

// ***********************************************
// Binary
// ***********************************************

PintaException      pinta_binary_read_uint(PintaReader *reader, u32 *result);
PintaException      pinta_binary_read_sint(PintaReader *reader, i32 *result);
PintaException      pinta_binary_read_uleb128(PintaReader *reader, u32 *length, u32 *result);
PintaException      pinta_binary_read_uleb128p1(PintaReader *reader, u32 *length, u32 *result);
PintaException      pinta_binary_read_sleb128(PintaReader *reader, u32 *length, i32 *result);

PintaException      pinta_binary_write_uint(PintaWriter *writer, u32 value);
PintaException      pinta_binary_write_sint(PintaWriter *writer, i32 value);
PintaException      pinta_binary_write_uleb128(PintaWriter *writer, u32 *length, u32 value);
PintaException      pinta_binary_write_uleb128p1(PintaWriter *writer, u32 *length, u32 value);
PintaException      pinta_binary_write_sleb128(PintaWriter *writer, u32 *length, i32 value);

// ***********************************************
// Heap functions
// ***********************************************

PintaHeap          *pinta_heap_init(PintaNativeMemory *memory, u32 length_in_bytes);
PintaHeapObject    *pinta_heap_alloc(PintaHeap *heap, u8 block_kind, u8 block_flags, u32 block_length);
void                pinta_heap_object_init(PintaHeapObject *object, u8 block_kind, u8 block_flags, u32 block_length);

PintaHeapObject    *pinta_free_get_next(PintaHeapObject *object);
PintaHeapObject    *pinta_free_get_prev(PintaHeapObject *object);

void                pinta_free_set_next(PintaHeapObject *object, PintaHeapObject *value);
void                pinta_free_set_prev(PintaHeapObject *object, PintaHeapObject *value);
PintaHeapObject    *pinta_free_remove(PintaHeap *heap, PintaHeapObject *item);
void                pinta_free_insert_after(PintaHeap *heap, PintaHeapObject *location, PintaHeapObject *item, u32 block_length);
void                pinta_free_merge(PintaHeap *heap, PintaHeapObject *free);

u32                 pinta_debug_get_alive_count(PintaHeap *heap);

PintaException      pinta_heap_handle_register(PintaCore *core, PintaHeapHandle *heap_handle);
PintaException      pinta_heap_handle_unregister(PintaCore *core, PintaHeapHandle *heap_handle);

#if PINTA_DEBUG
void                pinta_debug_validate_heap(PintaHeap *heap);
void                pinta_debug_assert_code(PintaThread *thread);
void                pinta_debug_validate_handle_register(PintaCore *core, PintaHeapHandle *handle);
void                pinta_debug_validate_handle_unregister(PintaCore *core, PintaHeapHandle *handle);
#else
#define             pinta_debug_validate_heap(heap) ((void)heap)
#define             pinta_debug_assert_code(thread) ((void)thread)
#define             pinta_debug_validate_handle_register(core, handle) ((void)0)
#define             pinta_debug_validate_handle_unregister(core, handle) ((void)0)
#endif

// ***********************************************
// GC
// ***********************************************

void                pinta_gc_mark_object(PintaCore *core, PintaHeapObject *object);
void                pinta_gc_relocate_single(PintaReference *value, PintaHeapReloc *reloc, u32 count);
void                pinta_gc_relocate_heap(PintaCore *core, PintaHeap *heap, PintaHeapReloc *reloc, u32 count);
void                pinta_gc_relocate_range(PintaReference *start, PintaReference *end, PintaHeapReloc *reloc, u32 count);
void                pinta_gc_relocate_frame(PintaStackFrame *frame, PintaHeapReloc *reloc, u32 count);
void                pinta_gc_relocate_native(PintaNativeFrame *native, PintaHeapReloc *reloc, u32 count);

void                pinta_gc_mark_range(PintaCore *core, PintaReference *start, PintaReference *end);
void                pinta_gc_mark(PintaCore *core);
void                pinta_gc_sweep(PintaHeap *heap);
void                pinta_gc_clear_weak(PintaCore *core);

i32                 pinta_gc_relocate_compare(const void *key, const void *data);
PintaHeapReloc     *pinta_gc_relocate_find(void *value, PintaHeapReloc *reloc, u32 count);
void                pinta_gc_relocate(PintaCore *core, PintaHeapReloc *reloc, u32 count);

void                pinta_gc_compact_worker(PintaCore *core, PintaHeap *heap, PintaHeapReloc *reloc, u32 reloc_count);
void                pinta_gc_compact(PintaCore *core);

#if PINTA_DEBUG
void                pinta_gc_relocate_validate(u8 block_kind, PintaHeapObject *object, PintaHeapReloc *reloc, u32 count);
#else
#define             pinta_gc_relocate_validate(block_kind, object, reloc, count) ((void)0)
#endif

// ***********************************************
// Core
// ***********************************************

PintaCore          *pinta_core_init(PintaNativeMemory *heap_memory, u32 heap_memory_length_in_bytes, u32 stack_memory_in_bytes);
PintaException      pinta_core_init_domain(PintaCore *core, PintaModule *module, PintaModuleDomain **result);
PintaHeapCache     *pinta_core_cache_init(PintaNativeMemory *memory);

// allocated possibly with GC requested length of blocks, returns NULL on out of memory
// looks through all heaps
PintaHeapObject    *pinta_core_alloc(PintaCore *core, u8 block_kind, u8 block_flags, u32 block_length);
void                pinta_core_gc(PintaCore *core, u8 compact);
void                pinta_core_pin(PintaHeapObject *object);
void                pinta_core_unpin(PintaHeapObject *object);

// prints to console if value is string, substring or multistring
PintaException      pinta_core_print_reference(PintaCore *core, PintaReference *value);
PintaException      pinta_core_print(PintaCore *core, PintaHeapObject *value);

PintaType          *pinta_core_get_object_type(PintaCore *core, PintaHeapObject *object);
PintaType          *pinta_core_get_type(PintaCore *core, PintaReference *reference);
void                pinta_core_init_type(PintaType *type);

void                pinta_null_init_type(PintaType *type);
PintaException      pinta_null_to_bool(PintaCore *core, PintaReference *value, u8 *result);
PintaException      pinta_null_debug_write(PintaCore *core, PintaReference *value, u32 max_depth, PintaJsonWriter *writer);

PintaException      pinta_core_default_to_integer(PintaCore *core, PintaReference *value, PintaReference *result);
PintaException      pinta_core_default_to_integer_value(PintaCore *core, PintaReference *value, i32 *result);
PintaException      pinta_core_default_to_decimal(PintaCore *core, PintaReference *value, PintaReference *result);
PintaException      pinta_core_default_to_string(PintaCore *core, PintaReference *value, PintaReference *result);
PintaException      pinta_core_default_to_bool(PintaCore *core, PintaReference *value, u8 *result);
PintaException      pinta_core_default_to_zero(PintaCore *core, PintaReference *result);
PintaException      pinta_core_default_get_item(PintaCore *core, PintaReference *array, u32 index, PintaReference *result);
PintaException      pinta_core_default_set_item(PintaCore *core, PintaReference *array, u32 index, PintaReference *result);
PintaException      pinta_core_default_get_length(PintaCore *core, PintaReference *value, u32 *result);
PintaException      pinta_core_default_get_char(PintaCore *core, PintaReference *value, u32 index, wchar *result);
PintaException      pinta_core_default_copy(PintaCore *core, PintaReference *value, PintaReference *result);
PintaException      pinta_core_default_debug_write(PintaCore *core, PintaReference *value, u32 max_depth, PintaJsonWriter *writer);

// ***********************************************
// Integer
// ***********************************************

u32                 integer_get_log2(u32 value);
u8                  integer_from_string(wchar *string, u32 string_length, i32 *value);

i32                 pinta_integer_get_value(PintaHeapObject *object);
void                pinta_integer_set_value(PintaHeapObject *object, i32 value);
i32                 pinta_integer_ref_get_value(PintaReference *reference);
void                pinta_integer_ref_set_value(PintaReference *reference, i32 value);

void                pinta_integer_init_type(PintaType *type);
PintaHeapObject    *pinta_integer_alloc_object_value(PintaCore *core, i32 value);

PintaException      pinta_lib_integer_alloc_value(PintaCore *core, i32 value, PintaReference *result);
PintaException      pinta_lib_integer_alloc_zero(PintaCore *core, PintaReference *result);
PintaException      pinta_lib_integer_parse(PintaCore *core, PintaReference *value, PintaReference *result);
PintaException      pinta_lib_integer_to_string(PintaCore *core, PintaReference *value, PintaReference *result);
PintaException      pinta_lib_integer_try_parse(PintaCore *core, PintaReference *value, u8 *isFailed, PintaReference *result);
PintaException      pinta_lib_integer_try_parse_value(PintaCore *core, PintaReference *value, u8 *isFailed, i32 *result);
PintaException      pinta_lib_integer_to_bool(PintaCore *core, PintaReference *value, u8 *result);
PintaException      pinta_lib_integer_to_integer_value(PintaCore *core, PintaReference *value, i32 *result);
PintaException      pinta_lib_integer_debug_write(PintaCore *core, PintaReference *value, u32 max_depth, PintaJsonWriter *writer);

// ***********************************************
// Decimal
// ***********************************************

decimal             pinta_soft_multiply32(u32 left, u32 right);
decimal             pinta_soft_multiply64(decimal left, decimal right);

u8                  decimal_from_string(wchar *string, u32 string_length, decimal *value);
decimal             decimal_from32(i32 value);
u32                 decimal_to_string(decimal value, wchar *string);
i32                 decimal_to32(decimal value);
decimal             decimal_to_integral_value(decimal value);

u32                 decimal_is_zero(decimal value);
u32                 decimal_is_positive(decimal value);
u32                 decimal_is_negative(decimal value);
i32                 decimal_compare(decimal left, decimal right);

decimal             decimal_abs(decimal value);
decimal             decimal_add(decimal left, decimal right);
decimal             decimal_subtract(decimal left, decimal right);
decimal             decimal_multiply(decimal left, decimal right);
decimal             decimal_divide(decimal left, decimal right);
decimal             decimal_negate(decimal value);

decimal             pinta_decimal_get_value(PintaHeapObject *object);
void                pinta_decimal_set_value(PintaHeapObject *object, decimal value);
void                pinta_decimal_set_zero(PintaHeapObject *value);
decimal             pinta_decimal_ref_get_value(PintaReference *object);
void                pinta_decimal_ref_set_value(PintaReference *object, decimal value);

void                pinta_format_decimal_general(decimal value, i32 precision, wchar *string);
void                pinta_format_decimal_fixed(decimal value, i32 precision, wchar *string);
void                pinta_format_decimal_exponential(decimal value, i32 precision, wchar *string);

void                pinta_decimal_init_type(PintaType *type);
PintaHeapObject    *pinta_decimal_alloc_object(PintaCore *core);

PintaException      pinta_lib_decimal_alloc(PintaCore *core, PintaReference *result);
PintaException      pinta_lib_decimal_alloc_zero(PintaCore *core, PintaReference *result);
PintaException      pinta_lib_decimal_to_int32(PintaCore *core, PintaReference *value, PintaReference *result);
PintaException      pinta_lib_decimal_to_int32_value(PintaCore *core, PintaReference *value, i32 *result);
PintaException      pinta_lib_decimal_from_int32(PintaCore *core, PintaReference *value, PintaReference *result);
PintaException      pinta_lib_decimal_to_string(PintaCore *core, PintaReference *value, PintaReference *result);
PintaException      pinta_lib_decimal_parse(PintaCore *core, PintaReference *value, PintaReference *result);
PintaException      pinta_lib_decimal_try_parse(PintaCore *core, PintaReference *value, u8 *isFailed, PintaReference *result);
PintaException      pinta_lib_decimal_add(PintaCore *core, PintaReference *left, PintaReference *right, PintaReference *result);
PintaException      pinta_lib_decimal_subtract(PintaCore *core, PintaReference *left, PintaReference *right, PintaReference *result);
PintaException      pinta_lib_decimal_multiply(PintaCore *core, PintaReference *left, PintaReference *right, PintaReference *result);
PintaException      pinta_lib_decimal_divide(PintaCore *core, PintaReference *left, PintaReference *right, PintaReference *result);
PintaException      pinta_lib_decimal_compare(PintaCore *core, PintaReference *left, PintaReference *right, PintaReference *result);
PintaException      pinta_lib_decimal_negate(PintaCore *core, PintaReference *value, PintaReference *result);
PintaException      pinta_lib_decimal_is_zero(PintaCore *core, PintaReference *value, PintaReference *result);
PintaException      pinta_lib_decimal_is_positive(PintaCore *core, PintaReference *value, PintaReference *result);
PintaException      pinta_lib_decimal_is_negative(PintaCore *core, PintaReference *value, PintaReference *result);
PintaException      pinta_lib_decimal_to_bool(PintaCore *core, PintaReference *value, u8 *result);
PintaException      pinta_lib_decimal_debug_write(PintaCore *core, PintaReference *value, u32 max_depth, PintaJsonWriter *writer);

// ***********************************************
// String
// ***********************************************

i32                 pinta_string_compare(wchar *left, wchar *right, u32 length);

u32                 pinta_string_get_length(PintaHeapObject *string);
wchar              *pinta_string_get_data(PintaHeapObject *string);
wchar               pinta_string_get_item(PintaHeapObject *string, u32 index);
void                pinta_string_set_length(PintaHeapObject *string, u32 length);
void                pinta_string_set_data(PintaHeapObject *string, wchar *data);
void                pinta_string_set_item(PintaHeapObject *string, u32 index, wchar value);
wchar              *pinta_string_ref_get_data(PintaReference *reference);
u32                 pinta_string_ref_get_length(PintaReference *reference);
wchar               pinta_string_ref_get_item(PintaReference *reference, u32 index);
void                pinta_string_ref_set_length(PintaReference *reference, u32 string_length);
void                pinta_string_ref_set_data(PintaReference *reference, wchar *string_data);

void                pinta_string_init_type(PintaType *type);
void                pinta_string_relocate(PintaHeapObject *object, PintaHeapReloc *reloc, u32 count);
PintaHeapObject    *pinta_string_alloc_object(PintaCore *core, u32 length);
PintaHeapObject    *pinta_string_alloc_object_value(PintaCore *core, wchar *data, u32 length);
PintaHeapObject    *pinta_string_alloc_object_copy(PintaCore *core, wchar *data, u32 length);
u8                  pinta_string_equals(PintaHeapObject *left, PintaHeapObject *right);
i32                 pinta_string_index_of(PintaReference *reference, u32 start, wchar value);

PintaException      pinta_lib_string_alloc(PintaCore *core, u32 length, PintaReference *result);
PintaException      pinta_lib_string_alloc_value(PintaCore *core, wchar *data, u32 length, PintaReference *result);
PintaException      pinta_lib_string_alloc_copy(PintaCore *core, wchar *data, u32 length, PintaReference *result);
PintaException      pinta_lib_string_to_string(PintaCore *core, PintaReference *value, PintaReference *result);
PintaException      pinta_lib_string_get_length(PintaCore *core, PintaReference *string, u32 *result);
PintaException      pinta_lib_string_get_charcode(PintaCore *core, PintaReference *string, u32 index, PintaReference *result);
PintaException      pinta_lib_string_get_char(PintaCore *core, PintaReference *string, u32 index, wchar *result);
PintaException      pinta_lib_string_get_member(PintaCore *core, PintaReference *string, PintaReference *name, u8 *is_accessor, PintaReference *result);
PintaException      pinta_lib_string_get_item(PintaCore *core, PintaReference *string, u32 index, PintaReference *result);
PintaException      pinta_lib_string_to_integer(PintaCore *core, PintaReference *string, PintaReference *result);
PintaException      pinta_lib_string_to_integer_value(PintaCore *core, PintaReference *string, i32 *result);
PintaException      pinta_lib_string_to_decimal(PintaCore *core, PintaReference *string, PintaReference *result);
PintaException      pinta_lib_string_to_bool(PintaCore *core, PintaReference *string, u8 *result);
PintaException      pinta_lib_string_get_hashcode(PintaCore *core, PintaReference *string, u32 *result);
PintaException      pinta_lib_string_debug_write(PintaCore *core, PintaReference *value, u32 max_depth, PintaJsonWriter *writer);

// ***********************************************
// Substring
// ***********************************************

PintaHeapObject    *pinta_substring_get_value(PintaHeapObject *object);
u16                 pinta_substring_get_offset(PintaHeapObject *object);
u16                 pinta_substring_get_length(PintaHeapObject *object);
void                pinta_substring_set_value(PintaHeapObject *object, PintaHeapObject *value);
void                pinta_substring_set_offset(PintaHeapObject *object, u16 value);
void                pinta_substring_set_length(PintaHeapObject *object, u16 value);
PintaHeapObject    *pinta_substring_ref_get_value(PintaReference *object);
u16                 pinta_substring_ref_get_offset(PintaReference *object);
u16                 pinta_substring_ref_get_length(PintaReference *object);
void                pinta_substring_ref_set_value(PintaReference *object, PintaReference *value);
void                pinta_substring_ref_set_offset(PintaReference *object, u16 value);
void                pinta_substring_ref_set_length(PintaReference *object, u16 value);

void                pinta_substring_init_type(PintaType *type);
u32                 pinta_substring_walk(PintaHeapObject *object, PintaHeapObjectWalkerState *state, PintaHeapObject **result);
void                pinta_substring_relocate(PintaHeapObject *object, PintaHeapReloc *reloc, u32 count);
PintaHeapObject    *pinta_substring_alloc_object(PintaCore *core, PintaReference *string, u16 offset, u16 length);

PintaException      pinta_lib_substring_alloc(PintaCore *core, PintaReference *string, u16 offset, u16 length, PintaReference *result);
PintaException      pinta_lib_substring_to_string(PintaCore *core, PintaReference *value, PintaReference *result);
PintaException      pinta_lib_substring_get_length(PintaCore *core, PintaReference *value, u32 *result);
PintaException      pinta_lib_substring_get_charcode(PintaCore *core, PintaReference *string, u32 index, PintaReference *result);
PintaException      pinta_lib_substring_get_char(PintaCore *core, PintaReference *string, u32 index, wchar *result);
PintaException      pinta_lib_substring_get_member(PintaCore *core, PintaReference *string, PintaReference *name, u8 *is_accessor, PintaReference *result);
PintaException      pinta_lib_substring_get_item(PintaCore *core, PintaReference *string, u32 index, PintaReference *result);
PintaException      pinta_lib_substring_to_bool(PintaCore *core, PintaReference *string, u8 *result);
PintaException      pinta_lib_substring_debug_write(PintaCore *core, PintaReference *value, u32 max_depth, PintaJsonWriter *writer);

// ***********************************************
// Multistring
// ***********************************************

PintaHeapObject   **pinta_multistring_get_value(PintaHeapObject *object);
PintaHeapObject    *pinta_multistring_get_item(PintaHeapObject *object, u32 index);
u32                 pinta_multistring_get_length(PintaHeapObject *object);
void                pinta_multistring_set_value(PintaHeapObject *object, PintaHeapObject **value);
void                pinta_multistring_set_item(PintaHeapObject *object, u32 index, PintaHeapObject *value);
PintaHeapObject   **pinta_multistring_ref_get_value(PintaReference *object);
PintaHeapObject    *pinta_multistring_ref_get_item(PintaReference *object, u32 index);
u32                 pinta_multistring_ref_get_length(PintaReference *object);
void                pinta_multistring_ref_set_value(PintaReference *object, PintaHeapObject **value);
void                pinta_multistring_ref_set_item(PintaReference *object, u32 index, PintaReference *value);
void                pinta_multistring_ref_set_null(PintaReference *object, u32 index);
void                pinta_multistring_ref_set_item_object(PintaReference *object, u32 index, PintaHeapObject *value);

void                pinta_multistring_init_type(PintaType *type);
u32                 pinta_multistring_walk(PintaHeapObject *object, PintaHeapObjectWalkerState *state, PintaHeapObject **result);
void                pinta_multistring_relocate(PintaHeapObject *object, PintaHeapReloc *reloc, u32 count);
PintaHeapObject    *pinta_multistring_alloc_object(PintaCore *core);
PintaHeapObject    *pinta_multistring_alloc_object_value(PintaCore *core, PintaReference *value);

PintaException      pinta_lib_multistring_alloc(PintaCore *core, PintaReference *result);
PintaException      pinta_lib_multistring_alloc_value(PintaCore *core, PintaReference *value, PintaReference *result);
PintaException      pinta_lib_multistring_to_string(PintaCore *core, PintaReference *value, PintaReference *result);
PintaException      pinta_lib_multistring_append(PintaCore *core, PintaReference *left, PintaReference *right, PintaReference *result);
PintaException      pinta_lib_multistring_prepend(PintaCore *core, PintaReference *left, PintaReference *right, PintaReference *result);
PintaException      pinta_lib_multistring_get_length(PintaCore *core, PintaReference *value, u32 *result);
PintaException      pinta_lib_multistring_get_charcode(PintaCore *core, PintaReference *string, u32 index, PintaReference *result);
PintaException      pinta_lib_multistring_get_char(PintaCore *core, PintaReference *string, u32 index, wchar *result);
PintaException      pinta_lib_multistring_get_member(PintaCore *core, PintaReference *string, PintaReference *name, u8 *is_accessor, PintaReference *result);
PintaException      pinta_lib_multistring_get_item(PintaCore *core, PintaReference *string, u32 index, PintaReference *result);
PintaException      pinta_lib_multistring_to_bool(PintaCore *core, PintaReference *string, u8 *result);
PintaException      pinta_lib_multistring_debug_write(PintaCore *core, PintaReference *value, u32 max_depth, PintaJsonWriter *writer);

// ***********************************************
// Char
// ***********************************************

wchar               pinta_char_get_value(PintaHeapObject *object);
void                pinta_char_set_value(PintaHeapObject *object, wchar value);
u32                 pinta_char_get_length(PintaHeapObject *object);
wchar               pinta_char_ref_get_value(PintaReference *reference);
void                pinta_char_ref_set_value(PintaReference *reference, wchar value);
u32                 pinta_char_ref_get_length(PintaReference *reference);

void                pinta_char_init_type(PintaType *type);
PintaHeapObject    *pinta_char_alloc_object_value(PintaCore *core, wchar value);

PintaException      pinta_lib_char_alloc_value(PintaCore *core, wchar value, PintaReference *result);
PintaException      pinta_lib_char_to_string(PintaCore *core, PintaReference *value, PintaReference *result);
PintaException      pinta_lib_char_get_length(PintaCore *core, PintaReference *value, u32 *result);
PintaException      pinta_lib_char_get_charcode(PintaCore *core, PintaReference *string, u32 index, PintaReference *result);
PintaException      pinta_lib_char_get_char(PintaCore *core, PintaReference *string, u32 index, wchar *result);
PintaException      pinta_lib_char_get_member(PintaCore *core, PintaReference *string, PintaReference *name, u8 *is_accessor, PintaReference *result);
PintaException      pinta_lib_char_get_item(PintaCore *core, PintaReference *string, u32 index, PintaReference *result);
PintaException      pinta_lib_char_debug_write(PintaCore *core, PintaReference *value, u32 max_depth, PintaJsonWriter *writer);

// ***********************************************
// Array
// ***********************************************

PintaHeapObject   **pinta_array_get_data(PintaHeapObject *array);
u32                 pinta_array_get_length(PintaHeapObject *array);
PintaHeapObject    *pinta_array_get_item(PintaHeapObject *array, u32 index);
void                pinta_array_set_length(PintaHeapObject *array, u32 length);
void                pinta_array_set_item(PintaHeapObject *array, u32 index, PintaHeapObject *value);
PintaHeapObject   **pinta_array_ref_get_data(PintaReference *array);
u32                 pinta_array_ref_get_length(PintaReference *array);
PintaHeapObject    *pinta_array_ref_get_item(PintaReference *array, u32 index);
void                pinta_array_ref_set_length(PintaReference *array, u32 length);
void                pinta_array_ref_set_item(PintaReference *array, u32 index, PintaReference *value);
void                pinta_array_ref_set_item_null(PintaReference *array, u32 index);

void                pinta_array_init_type(PintaType *type);
u32                 pinta_array_walk(PintaHeapObject *object, PintaHeapObjectWalkerState *state, PintaHeapObject **result);
void                pinta_array_relocate(PintaHeapObject *object, PintaHeapReloc *reloc, u32 count);
PintaHeapObject    *pinta_array_alloc_object(PintaCore *core, u32 array_length);
PintaHeapObject    *pinta_array_alloc_object_value(PintaCore *core, u32 array_length, PintaReference *source);
void                pinta_array_copy(PintaHeapObject *destination_array, u32 destination_offset, PintaReference *source_data, u32 length);
void                pinta_array_copy_array(PintaHeapObject *destination_array, u32 destination_offset, PintaHeapObject *source_array, u32 source_offset, u32 length);
void                pinta_array_ref_copy(PintaReference *destination_array, u32 destination_offset, PintaReference *source_data, u32 length);
void                pinta_array_ref_copy_array(PintaReference *destination_array, u32 destination_offset, PintaReference *source_array, u32 source_offset, u32 length);

PintaException      pinta_array_get_index(PintaHeapObject *array, u32 index, PintaReference *result);
PintaException      pinta_array_set_index(PintaHeapObject *array, u32 index, PintaHeapObject *value);

PintaException      pinta_lib_array_alloc(PintaCore *core, u32 array_length, PintaReference *result);
PintaException      pinta_lib_array_alloc_value(PintaCore *core, u32 array_length, PintaReference *source, PintaReference *result);
PintaException      pinta_lib_array_get_member(PintaCore *core, PintaReference *object, PintaReference *name, u8 *is_accessor, PintaReference *result);
PintaException      pinta_lib_array_set_member(PintaCore *core, PintaReference *object, PintaReference *name, PintaReference *value, u8 *is_accessor, PintaReference *result);
PintaException      pinta_lib_array_get_length(PintaCore *core, PintaReference *array, u32 *result);
PintaException      pinta_lib_array_get_item(PintaCore *core, PintaReference *array, u32 index, PintaReference *result);
PintaException      pinta_lib_array_set_item(PintaCore *core, PintaReference *array, u32 index, PintaReference *value);
PintaException      pinta_lib_array_debug_write(PintaCore *core, PintaReference *value, u32 max_depth, PintaJsonWriter *writer);

// ***********************************************
// Blob
// ***********************************************

u32                 pinta_blob_block_length(u32 blob_length);
void                pinta_blob_set_length(PintaHeapObject *buffer, u32 length);
void                pinta_blob_set_data(PintaHeapObject *buffer, u8 *data);
void                pinta_blob_set_item(PintaHeapObject *buffer, u32 index, u8 value);
u32                 pinta_blob_get_length(PintaHeapObject *buffer);
u8                 *pinta_blob_get_data(PintaHeapObject *buffer);
u8                  pinta_blob_get_item(PintaHeapObject *buffer, u32 index);
u32                 pinta_blob_ref_get_length(PintaReference *reference);
u8                 *pinta_blob_ref_get_data(PintaReference *reference);
u8                  pinta_blob_ref_get_item(PintaReference *reference, u32 index);
void                pinta_blob_ref_set_length(PintaReference *reference, u32 length);
void                pinta_blob_ref_set_item(PintaReference *reference, u32 index, u8 value);

void                pinta_blob_init_type(PintaType *type);
void                pinta_blob_relocate(PintaHeapObject *object, PintaHeapReloc *reloc, u32 count);
PintaHeapObject    *pinta_blob_alloc_object_value(PintaCore *core, PintaReference *value, u32 blob_length);
PintaHeapObject    *pinta_blob_alloc_object(PintaCore *core, u32 blob_length);

PintaException      pinta_blob_write_byte_value(PintaHeapObject *blob, u32 position, u8 value);
PintaException      pinta_blob_write_byte(PintaHeapObject *blob, u32 position, PintaReference *value);
PintaException      pinta_blob_write_short_value(PintaHeapObject *blob, u32 position, u16 value);
PintaException      pinta_blob_write_short(PintaHeapObject *blob, u32 position, PintaReference *value);
PintaException      pinta_blob_write_char_value(PintaHeapObject *blob, u32 position, wchar value);
PintaException      pinta_blob_write_char(PintaHeapObject *blob, u32 position, PintaReference *value);
PintaException      pinta_blob_write_integer_value(PintaHeapObject *blob, u32 position, u32 value);
PintaException      pinta_blob_write_integer(PintaHeapObject *blob, u32 position, PintaReference *value);
PintaException      pinta_blob_write_string(PintaHeapObject *blob, u32 position, u32 data_offset, u32 data_length, PintaHeapObject *value);
PintaException      pinta_blob_write_blob(PintaHeapObject *blob, u32 position, u32 data_offset, u32 data_length, PintaHeapObject *value);
PintaException      pinta_blob_write_data(PintaHeapObject *blob, u32 position, u8 *data, u32 data_length);
PintaException      pinta_blob_read_byte(PintaHeapObject *blob, u32 position, i32 *result);
PintaException      pinta_blob_read_short(PintaHeapObject *blob, u32 position, i32 *result);
PintaException      pinta_blob_read_integer(PintaHeapObject *blob, u32 position, i32 *result);

PintaException      pinta_lib_blob_alloc(PintaCore *core, u32 blob_length, PintaReference *result);
PintaException      pinta_lib_blob_set_member(PintaCore *core, PintaReference *blob, PintaReference *name, PintaReference *value, u8 *is_accessor, PintaReference *result);
PintaException      pinta_lib_blob_get_member(PintaCore *core, PintaReference *blob, PintaReference *name, u8 *is_accessor, PintaReference *result);
PintaException      pinta_lib_blob_get_length(PintaCore *core, PintaReference *blob, u32 *result);
PintaException      pinta_lib_blob_resize(PintaCore *core, PintaReference *blob, u32 blob_length, PintaReference *result);
PintaException      pinta_lib_blob_read_string_integer(PintaCore *core, PintaReference *blob, u32 position, u32 string_length, PintaReference *result);
PintaException      pinta_lib_blob_read_blob_integer(PintaCore *core, PintaReference *blob, u32 position, u32 data_length, PintaReference *result);
PintaException      pinta_lib_blob_write_byte(PintaCore *core, PintaReference *blob, u32 position, PintaReference *value);
PintaException      pinta_lib_blob_read_byte(PintaCore *core, PintaReference *blob, u32 position, PintaReference *result);
PintaException      pinta_lib_blob_in_place_to_string(PintaCore *core, PintaReference *blob, u32 string_length, PintaReference *result);
PintaException      pinta_lib_blob_debug_write(PintaCore *core, PintaReference *value, u32 max_depth, PintaJsonWriter *writer);

// ***********************************************
// Buffer
// ***********************************************

void                pinta_buffer_set_length(PintaHeapObject *buffer, u32 length);
void                pinta_buffer_set_position(PintaHeapObject *buffer, u32 position);
void                pinta_buffer_set_blob(PintaHeapObject *buffer, PintaHeapObject *blob);
u32                 pinta_buffer_get_length(PintaHeapObject *buffer);
u32                 pinta_buffer_get_position(PintaHeapObject *buffer);
PintaHeapObject    *pinta_buffer_get_blob(PintaHeapObject *buffer);
void                pinta_buffer_ref_set_length(PintaReference *reference, u32 length);
void                pinta_buffer_ref_set_position(PintaReference *reference, u32 position);
void                pinta_buffer_ref_set_blob(PintaReference *reference, PintaReference *blob);
void                pinta_buffer_ref_set_blob_null(PintaReference *reference);
u32                 pinta_buffer_ref_get_length(PintaReference *reference);
u32                 pinta_buffer_ref_get_position(PintaReference *reference);
PintaHeapObject    *pinta_buffer_ref_get_blob(PintaReference *reference);

void                pinta_buffer_init_type(PintaType *type);
u32                 pinta_buffer_walk(PintaHeapObject *object, PintaHeapObjectWalkerState *state, PintaHeapObject **result);
void                pinta_buffer_relocate(PintaHeapObject *object, PintaHeapReloc *reloc, u32 count);
PintaHeapObject    *pinta_buffer_alloc_object(PintaCore *core);

PintaException      pinta_lib_buffer_alloc(PintaCore *core, PintaReference *result);
PintaException      pinta_lib_buffer_alloc_value(PintaCore *core, u32 capacity, PintaReference *result);
PintaException      pinta_lib_buffer_get_member(PintaCore *core, PintaReference *blob, PintaReference *name, u8 *is_accessor, PintaReference *result);
PintaException      pinta_lib_buffer_set_member(PintaCore *core, PintaReference *blob, PintaReference *name, PintaReference *value, u8 *is_accessor, PintaReference *result);
PintaException      pinta_lib_buffer_get_length(PintaCore *core, PintaReference *buffer, u32 *result);
PintaException      pinta_lib_buffer_get_position(PintaCore *core, PintaReference *buffer, PintaReference *result);
PintaException      pinta_lib_buffer_get_blob(PintaCore *core, PintaReference *buffer, PintaReference *result);
PintaException      pinta_lib_buffer_set_length_integer(PintaCore *core, PintaReference *buffer, u32 length);
PintaException      pinta_lib_buffer_set_length_position(PintaCore *core, PintaReference *buffer);
PintaException      pinta_lib_buffer_set_length(PintaCore *core, PintaReference *buffer, PintaReference *value);
PintaException      pinta_lib_buffer_set_position(PintaCore *core, PintaReference *buffer, PintaReference *value);

PintaException      pinta_lib_buffer_ensure_write(PintaCore *core, PintaReference *buffer, u32 write_length);
PintaException      pinta_lib_buffer_write_byte_value(PintaCore *core, PintaReference *buffer, u8 value);
PintaException      pinta_lib_buffer_write_byte_at(PintaCore *core, PintaReference *buffer, u32 position, PintaReference *value);
PintaException      pinta_lib_buffer_write_byte(PintaCore *core, PintaReference *buffer, PintaReference *value);
PintaException      pinta_lib_buffer_write_short_value(PintaCore *core, PintaReference *buffer, u16 value);
PintaException      pinta_lib_buffer_write_short(PintaCore *core, PintaReference *buffer, PintaReference *value);
PintaException      pinta_lib_buffer_write_char_value(PintaCore *core, PintaReference *buffer, wchar value);
PintaException      pinta_lib_buffer_write_char(PintaCore *core, PintaReference *buffer, PintaReference *value);
PintaException      pinta_lib_buffer_write_integer_value(PintaCore *core, PintaReference *buffer, u32 value);
PintaException      pinta_lib_buffer_write_string(PintaCore *core, PintaReference *buffer, u32 offset, u32 length, PintaReference *value);
PintaException      pinta_lib_buffer_write_buffer(PintaCore *core, PintaReference *buffer, u32 offset, u32 length, PintaReference *value);
PintaException      pinta_lib_buffer_write_blob(PintaCore *core, PintaReference *buffer, u32 offset, u32 length, PintaReference *value);
PintaException      pinta_lib_buffer_write_data(PintaCore *core, PintaReference *buffer, u8 *data, u32 data_length);
PintaException      pinta_lib_buffer_write(PintaCore *core, PintaReference *buffer, PintaReference *value);

PintaException      pinta_lib_buffer_read_byte_at(PintaCore *core, PintaReference *buffer, u32 position, PintaReference *result);
PintaException      pinta_lib_buffer_read_byte(PintaCore *core, PintaReference *buffer, PintaReference *result);
PintaException      pinta_lib_buffer_read_short(PintaCore *core, PintaReference *buffer, PintaReference *result);
PintaException      pinta_lib_buffer_read_integer(PintaCore *core, PintaReference *buffer, PintaReference *result);
PintaException      pinta_lib_buffer_read_string(PintaCore *core, PintaReference *buffer, u32 length, PintaReference *result);
PintaException      pinta_lib_buffer_read_blob(PintaCore *core, PintaReference *buffer, u32 length, PintaReference *result);
PintaException      pinta_lib_buffer_read_buffer(PintaCore *core, PintaReference *buffer, u32 length, PintaReference *result);
PintaException      pinta_lib_buffer_in_place_to_string(PintaCore *core, PintaReference *buffer, PintaReference *result);
PintaException      pinta_lib_buffer_debug_write(PintaCore *core, PintaReference *value, u32 max_depth, PintaJsonWriter *writer);

// ***********************************************
// Weak
// ***********************************************

PintaHeapObject    *pinta_weak_get_target(PintaHeapObject *weak);
PintaHeapObject    *pinta_weak_get_next(PintaHeapObject *weak);
void                pinta_weak_set_target(PintaHeapObject *weak, PintaHeapObject *target);
void                pinta_weak_set_next(PintaHeapObject *weak, PintaHeapObject *next);
PintaHeapObject    *pinta_weak_ref_get_target(PintaReference *weak);
PintaHeapObject    *pinta_weak_ref_get_next(PintaReference *weak);
void                pinta_weak_ref_set_target(PintaReference *weak, PintaReference *target);
void                pinta_weak_ref_set_target_null(PintaReference *weak);
void                pinta_weak_ref_set_next(PintaReference *weak, PintaReference *next);
void                pinta_weak_ref_set_next_null(PintaReference *weak);

void                pinta_weak_init_type(PintaType *type);
void                pinta_weak_relocate(PintaHeapObject *object, PintaHeapReloc *reloc, u32 count);
PintaHeapObject    *pinta_weak_alloc_object_value(PintaCore *core, PintaReference *target);

PintaException      pinta_lib_weak_alloc(PintaCore *core, PintaReference *result);
PintaException      pinta_lib_weak_alloc_value(PintaCore *core, PintaReference *target, PintaReference *result);
PintaException      pinta_lib_weak_get_target(PintaCore *core, PintaReference *weak, PintaReference *result);
PintaException      pinta_lib_weak_set_target(PintaCore *core, PintaReference *weak, PintaReference *target);
PintaException      pinta_lib_weak_debug_write(PintaCore *core, PintaReference *value, u32 max_depth, PintaJsonWriter *writer);

// ***********************************************
// Function body
// ***********************************************
PintaHeapObject    *pinta_function_body_get_binding(PintaFunctionBody *function_body);
PintaHeapObject    *pinta_function_body_get_closure(PintaFunctionBody *function_body);
PintaHeapObject    *pinta_function_body_get_object(PintaFunctionBody *function_body);
PintaHeapObject    *pinta_function_body_get_prototype(PintaFunctionBody *function_body);
void                pinta_function_body_set_binding(PintaFunctionBody *function_body, PintaHeapObject *function_binding);
void                pinta_function_body_set_closure(PintaFunctionBody *function_body, PintaHeapObject *function_closure);
void                pinta_function_body_set_object(PintaFunctionBody *function_body, PintaHeapObject *function_object);
void                pinta_function_body_set_prototype(PintaFunctionBody *function_body, PintaHeapObject *function_prototype);

u32                 pinta_function_body_walk(PintaFunctionBody *function_body, PintaHeapObjectWalkerState *state, PintaHeapObject **result);
void                pinta_function_body_relocate(PintaFunctionBody *function_body, PintaHeapReloc *reloc, u32 count);

// ***********************************************
// Function managed
// ***********************************************

PintaModuleDomain  *pinta_function_managed_get_domain(PintaHeapObject *function);
u32                 pinta_function_managed_get_token(PintaHeapObject *function);
PintaFunctionBody  *pinta_function_managed_get_body_value(PintaHeapObject *function);
void                pinta_function_managed_get_body(PintaHeapObject *function, PintaReference *function_binding, PintaReference *function_closure, PintaReference *function_prototype, PintaReference *function_object);
void                pinta_function_managed_set_domain(PintaHeapObject *function, PintaModuleDomain *domain);
void                pinta_function_managed_set_token(PintaHeapObject *function, u32 token);
void                pinta_function_managed_set_body(PintaHeapObject *function, PintaReference *function_binding, PintaReference *function_closure, PintaReference *function_prototype, PintaReference *function_object);
PintaModuleDomain  *pinta_function_managed_ref_get_domain(PintaReference *function);
u32                 pinta_function_managed_ref_get_token(PintaReference *function);
void                pinta_function_managed_ref_get_body(PintaReference *function, PintaReference *function_binding, PintaReference *function_closure, PintaReference *function_prototype, PintaReference *function_object);
void                pinta_function_managed_ref_set_domain(PintaReference *function, PintaModuleDomain *domain);
void                pinta_function_managed_ref_set_token(PintaReference *function, u32 token);
void                pinta_function_managed_ref_set_body(PintaReference *function, PintaReference *function_binding, PintaReference *function_closure, PintaReference *function_prototype, PintaReference *function_object);

void                pinta_function_managed_init_type(PintaType *type);
u32                 pinta_function_managed_walk(PintaHeapObject *object, PintaHeapObjectWalkerState *state, PintaHeapObject **result);
void                pinta_function_managed_relocate(PintaHeapObject *object, PintaHeapReloc *reloc, u32 count);
PintaHeapObject    *pinta_function_managed_alloc_object(PintaCore *core, PintaModuleDomain *function_domain, u32 function_token, PintaReference *function_binding, PintaReference *function_closure, PintaReference *function_object, PintaReference *function_prototype);

PintaException      pinta_lib_function_managed_alloc(PintaCore *core, PintaModuleDomain *function_domain, u32 function_token, PintaReference *function_binding, PintaReference *function_closure, PintaReference *function_object, PintaReference *function_prototype, PintaReference *result);
PintaException      pinta_lib_function_managed_alloc_closure(PintaCore *core, PintaReference *outer_closure, PintaReference *outer_arguments, PintaReference *outer_locals, PintaReference *result);
PintaException      pinta_lib_function_managed_get_member(PintaCore *core, PintaReference *object, PintaReference *name, u8 *is_accessor, PintaReference *result);
PintaException      pinta_lib_function_managed_set_member(PintaCore *core, PintaReference *object, PintaReference *name, PintaReference *value, u8 *is_accessor, PintaReference *setter);
PintaException      pinta_lib_function_managed_get_body(PintaCore *core, PintaReference *function, PintaReference *function_binding, PintaReference *function_closure, PintaReference *function_prototype, PintaReference *function_object);
PintaException      pinta_lib_function_managed_set_body(PintaCore *core, PintaReference *function, PintaReference *function_binding, PintaReference *function_closure, PintaReference *function_prototype, PintaReference *function_object);
PintaException      pinta_lib_function_managed_debug_write(PintaCore *core, PintaReference *value, u32 max_depth, PintaJsonWriter *writer);

// ***********************************************
// Function native
// ***********************************************

PintaFunctionDelegate 
                    pinta_function_native_get_delegate(PintaHeapObject *function);
u32                 pinta_function_native_get_tag(PintaHeapObject *function);
PintaFunctionBody  *pinta_function_native_get_body_value(PintaHeapObject *function);
void                pinta_function_native_get_body(PintaHeapObject *function, PintaReference *function_binding, PintaReference *function_closure, PintaReference *function_prototype, PintaReference *function_object);
void                pinta_function_native_set_delegate(PintaHeapObject *function, PintaFunctionDelegate function_delegate);
void                pinta_function_native_set_tag(PintaHeapObject *function, u32 function_tag);
void                pinta_function_native_set_body(PintaHeapObject *function, PintaReference *function_binding, PintaReference *function_closure, PintaReference *function_prototype, PintaReference *function_object);
PintaFunctionDelegate 
                    pinta_function_native_ref_get_delegate(PintaReference *function);
u32                 pinta_function_native_ref_get_tag(PintaReference *function);
void                pinta_function_native_ref_get_body(PintaReference *function, PintaReference *function_binding, PintaReference *function_closure, PintaReference *function_prototype, PintaReference *function_object);
void                pinta_function_native_ref_set_delegate(PintaReference *function, PintaFunctionDelegate function_delegate);
void                pinta_function_native_ref_set_tag(PintaReference *function, u32 function_tag);
void                pinta_function_native_ref_set_body(PintaReference *function, PintaReference *function_binding, PintaReference *function_closure, PintaReference *function_prototype, PintaReference *function_object);

void                pinta_function_native_init_type(PintaType *type);
u32                 pinta_function_native_walk(PintaHeapObject *object, PintaHeapObjectWalkerState *state, PintaHeapObject **result);
void                pinta_function_native_relocate(PintaHeapObject *object, PintaHeapReloc *reloc, u32 count);
PintaHeapObject    *pinta_function_native_alloc_object(PintaCore *core, PintaFunctionDelegate function_delegate, u32 function_tag, PintaReference *function_binding, PintaReference *function_closure, PintaReference *function_object, PintaReference *function_prototype);

PintaException      pinta_lib_function_native_alloc(PintaCore *core, PintaFunctionDelegate function_delegate, u32 function_tag, PintaReference *function_binding, PintaReference *function_closure, PintaReference *function_object, PintaReference *function_prototype, PintaReference *result);
PintaException      pinta_lib_function_native_get_member(PintaCore *core, PintaReference *object, PintaReference *name, u8 *is_accessor, PintaReference *result);
PintaException      pinta_lib_function_native_set_member(PintaCore *core, PintaReference *object, PintaReference *name, PintaReference *value, u8 *is_accessor, PintaReference *setter);
PintaException      pinta_lib_function_native_get_body(PintaCore *core, PintaReference *function, PintaReference *function_binding, PintaReference *function_closure, PintaReference *function_prototype, PintaReference *function_object);
PintaException      pinta_lib_function_native_set_body(PintaCore *core, PintaReference *function, PintaReference *function_binding, PintaReference *function_closure, PintaReference *function_prototype, PintaReference *function_object);
PintaException      pinta_lib_function_native_debug_write(PintaCore *core, PintaReference *value, u32 max_depth, PintaJsonWriter *writer);

// ***********************************************
// Property table
// ***********************************************

u32                 pinta_property_is_free(PintaProperty *property);
u32                 pinta_property_is_enumerable(PintaProperty *property);
u32                 pinta_property_is_configurable(PintaProperty *property);
u32                 pinta_property_is_writeable_or_native(PintaProperty *property);
u32                 pinta_property_is_accessor(PintaProperty *property);
u32                 pinta_property_get_hash_code(PintaProperty *property);
PintaHeapObject    *pinta_property_get_key(PintaProperty *property);
PintaHeapObject    *pinta_property_get_value(PintaProperty *property);
PintaHeapObject    *pinta_property_get_accessor_get(PintaProperty *property);
PintaHeapObject    *pinta_property_get_accessor_set(PintaProperty *property);
PintaPropertyNativeDelegate
                    pinta_property_get_native_delegate(PintaProperty *property);
u32                 pinta_property_get_native_token(PintaProperty *property);
void                pinta_property_set_hash_code(PintaProperty *property, u8 is_enumerable, u8 is_configurable, u8 is_writeable, u8 is_accessor, u32 hash_code);
void                pinta_property_set_key(PintaProperty *property, PintaHeapObject *key);
void                pinta_property_set_value(PintaProperty *property, PintaHeapObject *value);
void                pinta_property_set_accessor_get(PintaProperty *property, PintaHeapObject *get);
void                pinta_property_set_accessor_set(PintaProperty *property, PintaHeapObject *set);
void                pinta_property_set_native_delegate(PintaProperty *property, PintaPropertyNativeDelegate delegate);
void                pinta_property_set_native_token(PintaProperty *property, u32 token);

u32                 pinta_property_table_get_capacity(PintaHeapObject *object);
u32                 pinta_property_table_get_length(PintaHeapObject *object);
PintaProperty      *pinta_property_table_get_table(PintaHeapObject *object);
PintaProperty      *pinta_property_table_get(PintaHeapObject *object, u32 property_id);
void                pinta_property_table_set_capacity(PintaHeapObject *object, u32 value);
void                pinta_property_table_set_length(PintaHeapObject *object, u32 value);
u32                 pinta_property_table_ref_get_capacity(PintaReference *reference);
u32                 pinta_property_table_ref_get_length(PintaReference *reference);
PintaProperty      *pinta_property_table_ref_get_table(PintaReference *reference);
void                pinta_property_table_ref_set_capacity(PintaReference *reference, u32 value);
void                pinta_property_table_ref_set_length(PintaReference *reference, u32 value);

void                pinta_property_table_init_type(PintaType *type);
u32                 pinta_property_table_walk(PintaHeapObject *object, PintaHeapObjectWalkerState *state, PintaHeapObject **result);
void                pinta_property_table_relocate(PintaHeapObject *object, PintaHeapReloc *reloc, u32 count);
PintaHeapObject    *pinta_property_table_alloc_object(PintaCore *core, u32 table_capacity);
PintaHeapObject    *pinta_property_table_alloc_object_value(PintaCore *core, PintaReference *value, u32 table_capacity);
PintaProperty      *pinta_property_table_find(PintaProperty *table, u32 table_capacity, PintaReference *key, u32 key_hash_code, PintaPropertySlot *property_slot);
void                pinta_property_table_copy_property(PintaProperty *destination_table, u32 destination_capacity, PintaProperty *value);

PintaException      pinta_lib_property_table_alloc(PintaCore *core, u32 capacity, PintaReference *result);
PintaException      pinta_lib_property_table_alloc_value(PintaCore *core, PintaReference *table, u32 capacity, PintaReference *result);
PintaException      pinta_lib_property_table_get_hash_code(PintaCore *core, PintaReference *key, u32 *hash_code);
PintaException      pinta_lib_property_table_find(PintaCore *core, PintaReference *property_table, PintaReference *property_key, PintaPropertySlot *result);
PintaException      pinta_lib_property_table_add_value(PintaCore *core, PintaReference *property_table, PintaReference *property_key, PintaPropertySlot *property_slot, u8 property_is_enumerable, u8 property_is_configurable, u8 property_is_writeable, PintaReference *property_value);
PintaException      pinta_lib_property_table_add_accessor(PintaCore *core, PintaReference *property_table, PintaReference *property_key, PintaPropertySlot *property_slot, u8 property_is_enumerable, u8 property_is_configurable, PintaReference *property_accessor_get, PintaReference *property_accessor_set);
PintaException      pinta_lib_property_table_add_native(PintaCore *core, PintaReference *property_table, PintaReference *property_key, PintaPropertySlot *property_slot, u8 property_is_enumerable, u8 property_is_configurable, PintaPropertyNativeDelegate property_native_delegate, u32 property_native_token);
PintaException      pinta_lib_property_table_get_value(PintaCore *core, PintaReference *property_table, PintaPropertySlot *property_slot, PintaReference *result);
PintaException      pinta_lib_property_table_set_value(PintaCore *core, PintaReference *property_table, PintaPropertySlot *property_slot, PintaReference *value);
PintaException      pinta_lib_property_table_get_accessor(PintaCore *core, PintaReference *property_table, PintaPropertySlot *property_slot, PintaReference *property_accessor_get, PintaReference *property_accessor_set);
PintaException      pinta_lib_property_table_set_accessor(PintaCore *core, PintaReference *property_table, PintaPropertySlot *property_slot, PintaReference *property_accessor_get, PintaReference *property_accessor_set);
PintaException      pinta_lib_property_table_get_native(PintaCore *core, PintaReference *property_table, PintaPropertySlot *property_slot, PintaPropertyNativeDelegate *property_native_delegate, u32 *property_native_token);
PintaException      pinta_lib_property_table_set_native(PintaCore *core, PintaReference *property_table, PintaPropertySlot *property_slot, PintaPropertyNativeDelegate property_native_delegate, u32 property_native_token);
PintaException      pinta_lib_property_table_debug_write(PintaCore *core, PintaReference *value, u32 max_depth, PintaJsonWriter *writer);

// ***********************************************
// Object
// ***********************************************
PintaHeapObject    *pinta_object_get_prototype(PintaHeapObject *object);
PintaHeapObject    *pinta_object_get_body(PintaHeapObject *object);
void                pinta_object_set_body(PintaHeapObject *object, PintaHeapObject *object_body);
void                pinta_object_set_prototype(PintaHeapObject *object, PintaHeapObject *object_prototype);
PintaHeapObject    *pinta_object_ref_get_prototype(PintaReference *reference);
PintaHeapObject    *pinta_object_ref_get_body(PintaReference *reference);
void                pinta_object_ref_set_body(PintaReference *reference, PintaReference *object_body);
void                pinta_object_ref_set_prototype(PintaReference *reference, PintaReference *object_prototype);

void                pinta_object_init_type(PintaType *type);
u32                 pinta_object_walk(PintaHeapObject *object, PintaHeapObjectWalkerState *state, PintaHeapObject **result);
void                pinta_object_relocate(PintaHeapObject *object, PintaHeapReloc *reloc, u32 count);
PintaHeapObject    *pinta_object_alloc(PintaCore *core, PintaReference *body, PintaReference *prototype);

PintaException      pinta_object_throw_engine(PintaCore *core, PintaReference *value, PintaReference *result);
PintaException      pinta_object_to_integer_value(PintaCore *core, PintaReference *value, i32 *result);
PintaException      pinta_object_to_bool(PintaCore *core, PintaReference *value, u8 *result);
PintaException      pinta_object_to_zero(PintaCore *core, PintaReference *result);
PintaException      pinta_object_get_item(PintaCore *core, PintaReference *array, u32 index, PintaReference *result);
PintaException      pinta_object_set_item(PintaCore *core, PintaReference *array, u32 index, PintaReference *result);

PintaException      pinta_lib_object_alloc(PintaCore *core, PintaReference *body, PintaReference *prototype, PintaReference *result);
PintaException      pinta_lib_object_get_property(PintaCore *core, PintaReference *object, PintaReference *name, PintaReference *property_owner, PintaPropertySlot *property_slot);
PintaException      pinta_lib_object_define_property(PintaCore *core, PintaReference *object, PintaReference *name, u8 is_enumerable, u8 is_configurable, u8 is_writeable, PintaReference *value, PintaReference *get, PintaReference *set);
PintaException      pinta_lib_object_define_property_native(PintaCore *core, PintaReference *object, PintaReference *name, u8 is_enumerable, u8 is_configurable, PintaPropertyNativeDelegate property_native_delegate, u32 property_native_token);
PintaException      pinta_lib_object_get_member(PintaCore *core, PintaReference *object, PintaReference *name, u8 *is_accessor, PintaReference *result);
PintaException      pinta_lib_object_set_member(PintaCore *core, PintaReference *object, PintaReference *name, PintaReference *value, u8 *is_accessor, PintaReference *setter);
PintaException      pinta_lib_object_debug_write(PintaCore *core, PintaReference *value, u32 max_depth, PintaJsonWriter *writer);

// ***********************************************
// Global Object
// ***********************************************

PintaModuleDomain  *pinta_global_object_get_domain(PintaHeapObject *global);
PintaHeapObject    *pinta_global_object_get_body(PintaHeapObject *global);
void                pinta_global_object_set_domain(PintaHeapObject *global, PintaModuleDomain *domain);
void                pinta_global_object_set_body(PintaHeapObject *global, PintaHeapObject *body);
PintaModuleDomain  *pinta_global_object_ref_get_domain(PintaReference *global);
PintaHeapObject    *pinta_global_object_ref_get_body(PintaReference *global);
void                pinta_global_object_ref_set_domain(PintaReference *global, PintaModuleDomain *domain);
void                pinta_global_object_ref_set_body(PintaReference *global, PintaReference *body);
void                pinta_global_object_ref_set_body_null(PintaReference *global);

void                pinta_global_object_init_type(PintaType *type);
u32                 pinta_global_object_walk(PintaHeapObject *object, PintaHeapObjectWalkerState *state, PintaHeapObject **result);
void                pinta_global_object_relocate(PintaHeapObject *object, PintaHeapReloc *reloc, u32 count);
PintaHeapObject    *pinta_global_object_alloc(PintaCore *core, PintaModuleDomain *domain);

PintaException      pinta_lib_global_object_alloc(PintaCore *core, PintaModuleDomain *domain, PintaReference *result);
PintaException      pinta_lib_global_object_get_member(PintaCore *core, PintaReference *object, PintaReference *name, u8 *is_accessor, PintaReference *result);
PintaException      pinta_lib_global_object_set_member(PintaCore *core, PintaReference *object, PintaReference *name, PintaReference *value, u8 *is_accessor, PintaReference *result);
PintaException      pinta_lib_global_object_debug_write(PintaCore *core, PintaReference *value, u32 max_depth, PintaJsonWriter *writer);

// ***********************************************
// Code
// ***********************************************

PintaException      pinta_code_to_string(PintaThread *thread, PintaReference *value, PintaReference *result);
PintaException      pinta_code_string_concat(PintaThread *thread, PintaReference *left, PintaReference *right, PintaReference *result);
PintaException      pinta_code_common_arithmetic(PintaThread *thread, u8 code, PintaReference *left, PintaReference *right, PintaReference *result);
PintaException      pinta_code_to_bool(PintaThread *thread, PintaReference *value, u8 *result);
PintaException      pinta_code_next(PintaThread *thread, u8 *result);
PintaException      pinta_code_integer(PintaThread *thread, i32 *result);
PintaException      pinta_code_token(PintaThread *thread, u32 *result);
PintaException      pinta_code_nop(PintaThread *thread);
PintaException      pinta_code_load_null(PintaThread *thread);
PintaException      pinta_code_load_integer(PintaThread *thread, i32 value);
PintaException      pinta_code_load_integer_zero(PintaThread *thread);
PintaException      pinta_code_load_decimal_zero(PintaThread *thread);
PintaException      pinta_code_load_integer_one(PintaThread *thread);
PintaException      pinta_code_load_decimal_one(PintaThread *thread);
PintaException      pinta_code_load_string(PintaThread *thread, u32 token);
PintaException      pinta_code_add(PintaThread *thread);
PintaException      pinta_code_arithmetic(PintaThread *thread, u8 code);
PintaException      pinta_code_exclusive_or(PintaThread *thread);
PintaException      pinta_code_bitwise(PintaThread *thread, u8 code);
PintaException      pinta_code_unary(PintaThread *thread, u8 code);
PintaException      pinta_code_compare(PintaThread *thread, u8 code);
PintaException      pinta_code_new_array(PintaThread *thread);
PintaException      pinta_code_concat(PintaThread *thread);
PintaException      pinta_code_substring(PintaThread *thread);
PintaException      pinta_code_jump(PintaThread *thread, i32 offset);
PintaException      pinta_code_jump_conditional(PintaThread *thread, u8 code, i32 offset);
PintaException      pinta_code_call(PintaThread *thread, u32 token, u32 arguments_count);
PintaException      pinta_code_call_internal(PintaThread *thread, u32 token, u32 arguments_count);
PintaException      pinta_code_return(PintaThread *thread);
PintaException      pinta_code_store_local(PintaThread *thread, u32 index);
PintaException      pinta_code_store_global(PintaThread *thread, u32 index);
PintaException      pinta_code_store_argument(PintaThread *thread, u32 index);
PintaException      pinta_code_store_item(PintaThread *thread);
PintaException      pinta_code_load_local(PintaThread *thread, u32 index);
PintaException      pinta_code_load_global(PintaThread *thread, u32 index);
PintaException      pinta_code_load_argument(PintaThread *thread, u32 index);
PintaException      pinta_code_load_item(PintaThread *thread);
PintaException      pinta_code_duplicate(PintaThread *thread, u32 count);
PintaException      pinta_code_pop(PintaThread *thread);
PintaException      pinta_code_get_length(PintaThread *thread);
PintaException      pinta_code_exit(PintaThread *thread);
PintaException      pinta_code_load_blob(PintaThread *thread, u32 token);
PintaException      pinta_code_error(PintaThread *thread);
PintaException      pinta_code_new_function(PintaThread *thread, u32 token);
PintaException      pinta_code_invoke(PintaThread *thread, u32 arguments_count, u8 has_this);
PintaException      pinta_code_invoke_function_managed(PintaThread *thread, u32 arguments_count, u8 has_this);
PintaException      pinta_code_invoke_function_native(PintaThread *thread, u32 arguments_count, u8 has_this);
PintaException      pinta_code_load_closure(PintaThread *thread, u32 closure_index, u32 value_index);
PintaException      pinta_code_store_closure(PintaThread *thread, u32 closure_index, u32 value_index);
PintaException      pinta_code_new(PintaThread *thread, u32 arguments_count);
PintaException      pinta_code_construct_function(PintaThread *thread, u32 arguments_count);
PintaException      pinta_code_match(PintaThread *thread, u8 match_flags);
PintaException      pinta_code_step(PintaThread *thread);
PintaException      pinta_code_execute(PintaCore *core);
PintaException      pinta_code_execute_module(PintaCore *core, PintaModuleDomain *domain);

PintaException      pinta_code_rt_format(PintaCore *core, PintaReference *arguments, PintaReference *return_value);
PintaException      pinta_code_rt_require(PintaCore *core, PintaThread *thread, PintaReference *function, PintaReference *function_this, PintaReference *function_arguments, u8 *discard_return_value, PintaReference *return_value);

// ***********************************************
// Stack
// ***********************************************

PintaStackFrame    *pinta_frame_init(PintaNativeMemory *memory, u32 length_in_bytes);
PintaStackFrame    *pinta_frame_push(PintaStackFrame *frame);
PintaStackFrame    *pinta_frame_pop(PintaStackFrame *frame);

PintaException      pinta_frame_stack_push(PintaStackFrame *frame, PintaHeapObject *value);
PintaException      pinta_frame_stack_push_null(PintaStackFrame *frame);
PintaException      pinta_frame_stack_pop(PintaStackFrame *frame, PintaReference *result);
PintaException      pinta_frame_stack_duplicate(PintaStackFrame *frame, u32 count);
PintaException      pinta_frame_stack_get_reference(PintaStackFrame *frame, u32 offset, PintaReference **start);
PintaException      pinta_frame_stack_discard(PintaStackFrame *frame, u32 length);

PintaException      pinta_lib_frame_push(PintaThread *thread);
PintaException      pinta_lib_frame_pop(PintaThread *thread);

PintaException      pinta_stack_push(PintaThread *thread, PintaHeapObject *value);
PintaException      pinta_lib_stack_push(PintaThread *thread, PintaReference *value);
PintaException      pinta_lib_stack_pop(PintaThread *thread, PintaReference *result);
PintaException      pinta_lib_stack_push_null(PintaThread *thread);
PintaException      pinta_lib_stack_duplicate(PintaThread *thread, u32 count);
PintaException      pinta_lib_stack_get_reference(PintaThread *thread, u32 offset, PintaReference **start);
PintaException      pinta_lib_stack_discard(PintaThread *thread, u32 length);

// ***********************************************
// Module
// ***********************************************

PintaModule        *pinta_module_init(void *module_data, u32 module_length);
u8                  pinta_module_is_compressed_data(PintaModuleDomain *domain);
PintaException      pinta_module_get_string(PintaModuleDomain *domain, u32 token, PintaCodeString *result);
PintaException      pinta_module_get_blob(PintaModuleDomain *domain, u32 token, PintaCodeBlob *result);
PintaException      pinta_module_get_function(PintaModuleDomain *domain, u32 token, PintaCodeFunction *result);
PintaException      pinta_module_get_global_name_token(PintaModuleDomain *domain, u32 global_token, u32 *name_token);

PintaException      pinta_lib_module_get_global_token(PintaCore *core, PintaModuleDomain *domain, PintaReference *name, u32 *token);
PintaException      pinta_lib_module_get_global_value(PintaCore *core, PintaModuleDomain *domain, PintaReference *name, PintaReference *result);
PintaException      pinta_lib_module_set_global_value(PintaCore *core, PintaModuleDomain *domain, PintaReference *name, PintaReference *value);
PintaException      pinta_lib_module_get_string(PintaCore *core, PintaModuleDomain *domain, u32 token, PintaReference *result);
// ***********************************************
// Formatting strings
// ***********************************************

u8                  pinta_format_parse_flag(wchar flag, PintaFormat *format);
u8                  pinta_format_parse_number(wchar digit, i32 *result);
void                pinta_format_parse(PintaReference *format_string, i32 start, PintaFormat *format);
PintaException      pinta_format_decimal(PintaCore *core, PintaReference *buffer, PintaReference *value, PintaFormat *format);
PintaException      pinta_format_string(PintaCore *core, PintaReference *buffer, PintaReference *value, PintaFormat *format);

PintaException      pinta_lib_format_convert_integer(PintaCore *core, PintaReference *value, PintaReference *result);
PintaException      pinta_lib_format_convert_decimal(PintaCore *core, PintaReference *value, PintaReference *result);
PintaException      pinta_lib_format_convert_string(PintaCore *core, PintaReference *value, PintaReference *result);
PintaException      pinta_lib_format_buffer(PintaCore *core, PintaReference *buffer, PintaFormatOptions *options, PintaReference *format_string, PintaReference *format_arguments, u32 format_arguments_start);
PintaException      pinta_lib_format(PintaCore *core, PintaFormatOptions *options, PintaReference *format_string, PintaReference *format_arguments, u32 format_arguments_start, PintaReference *result);

// ***********************************************
// Encoding
// ***********************************************

PintaException      pinta_lib_decode_utf8(PintaCore *core, u8 *data, u32 data_length, PintaReference *result);
PintaException      pinta_lib_decode_string_c(PintaCore *core, char *data, u32 data_length, PintaReference *result);
PintaException      pinta_lib_decode_mutf8(PintaCore *core, u8 *data, u32 string_length, PintaReference *result);

PintaException      pinta_lib_buffer_write_bom_utf8(PintaCore *core, PintaReference *buffer);
PintaException      pinta_lib_buffer_write_string_utf8(PintaCore *core, PintaReference *buffer, PintaReference *string);
PintaException      pinta_lib_buffer_write_bom_mutf8(PintaCore *core, PintaReference *buffer);
PintaException      pinta_lib_buffer_write_string_mutf8(PintaCore *core, PintaReference *buffer, PintaReference *string);
PintaException      pinta_lib_buffer_write_bom_utf16(PintaCore *core, PintaReference *buffer);

// ***********************************************
// Pattern
// ***********************************************
PintaException      pinta_pattern_simple(PintaReference *string, PintaReference *pattern, u32 offset, u32 length, u32 *result);
PintaException      pinta_pattern_pan(PintaReference *string, PintaReference *pattern, u32 length, u32 *result);

PintaException      pinta_lib_pattern_simple(PintaCore *core, PintaReference *string, PintaReference *pattern, PintaReference *offset, PintaReference *length, u32 *result);
PintaException      pinta_lib_pattern_pan(PintaCore *core, PintaReference *string, PintaReference *pattern, PintaReference *length, u32 *result);

// ***********************************************
// JSON
// ***********************************************

PintaException      pinta_json_write_skipped(PintaCore *core, PintaJsonWriter *writer);
PintaException      pinta_json_write_null(PintaCore *core, PintaJsonWriter *writer);
PintaException      pinta_json_write_bool(PintaCore *core, PintaJsonWriter *writer, u32 value);
PintaException      pinta_json_write_i32(PintaCore *core, PintaJsonWriter *writer, i32 value);
PintaException      pinta_json_write_u32(PintaCore *core, PintaJsonWriter *writer, u32 value);
PintaException      pinta_json_write_decimal(PintaCore *core, PintaJsonWriter *writer, decimal value);
PintaException      pinta_json_write_string(PintaCore *core, PintaJsonWriter *writer, wchar *data, u32 length);
PintaException      pinta_json_write_string_c(PintaCore *core, PintaJsonWriter *writer, char *data, u32 length);
PintaException      pinta_json_write_binary(PintaCore *core, PintaJsonWriter *writer, u8 *data, u32 length);
PintaException      pinta_json_write_array_start(PintaCore *core, PintaJsonWriter *writer);
PintaException      pinta_json_write_object_start(PintaCore *core, PintaJsonWriter *writer);
PintaException      pinta_json_write_end(PintaCore *core, PintaJsonWriter *writer);
PintaException      pinta_json_write_property_name(PintaCore *core, PintaJsonWriter *writer, wchar *name);
PintaException      pinta_json_write_property_name_c(PintaCore *core, PintaJsonWriter *writer, char *name);
PintaException      pinta_json_write_property_string(PintaCore *core, PintaJsonWriter *writer, wchar *name, wchar *value);
PintaException      pinta_json_write_property_i32(PintaCore *core, PintaJsonWriter *writer, wchar *name, i32 value);
PintaException      pinta_json_write_property_u32(PintaCore *core, PintaJsonWriter *writer, wchar *name, u32 value);
PintaException      pinta_json_write_property_bool(PintaCore *core, PintaJsonWriter *writer, wchar *name, u32 value);
PintaException      pinta_json_write_property_binary(PintaCore *core, PintaJsonWriter *writer, wchar *name, u8 *data, u32 length);
PintaException      pinta_json_write_property_null(PintaCore *core, PintaJsonWriter *writer, wchar *name);

// ***********************************************
// Debugger
// ***********************************************

#if PINTA_DEBUG
void                pinta_debug_raise_tick(PintaCore *core);
void                pinta_debug_raise_exit(PintaCore *core, PintaException exception);
void                pinta_debug_raise_domain(PintaCore *core, PintaModule *module, PintaModuleDomain *domain);
void                pinta_debug_raise_step(PintaThread *thread);
void                pinta_debug_raise_exception(PintaThread *thread, PintaException exception);
void                pinta_debug_raise_before_call(PintaThread *thread, u32 token, u32 arguments_count);
void                pinta_debug_raise_after_call(PintaThread *thread);
void                pinta_debug_raise_before_call_internal(PintaThread *thread, u32 token, u32 arguments_count);
void                pinta_debug_raise_after_call_internal(PintaThread *thread);
void                pinta_debug_raise_before_invoke(PintaThread *thread, u32 arguments_count, u8 has_this);
void                pinta_debug_raise_after_invoke(PintaThread *thread);
void                pinta_debug_raise_before_return(PintaThread *thread);
void                pinta_debug_raise_after_return(PintaThread *thread);
void                pinta_debug_raise_break(PintaThread *thread, u8 *code);
#else
#define             pinta_debug_raise_tick(core)                                            ((void)0)
#define             pinta_debug_raise_exit(core, exception)                                 ((void)0)
#define             pinta_debug_raise_domain(core, module, domain)                          ((void)0)
#define             pinta_debug_raise_step(thread)                                          ((void)0)
#define             pinta_debug_raise_exception(thread, exception)                          ((void)0)
#define             pinta_debug_raise_before_call(thread, token, arguments_count)           ((void)0)
#define             pinta_debug_raise_after_call(thread);                                   ((void)0)
#define             pinta_debug_raise_before_call_internal(thread, token, arguments_count)  ((void)0)
#define             pinta_debug_raise_after_call_internal(thread);                          ((void)0)
#define             pinta_debug_raise_before_invoke(thread, arguments_count, has_this)      ((void)0)
#define             pinta_debug_raise_after_invoke(thread)                                  ((void)0)
#define             pinta_debug_raise_before_return(thread)                                 ((void)0)
#define             pinta_debug_raise_after_return(thread)                                  ((void)0)
#define             pinta_debug_raise_break(thread, code)                                   ((void)0)
#endif /* PINTA_DEBUG */

PintaException      pinta_lib_debug_write(PintaCore *core, PintaReference *value, u32 max_depth, PintaJsonWriter *writer);

// ***********************************************
// API
// ***********************************************

#include "pinta-api.h"

void               *pinta_api_load_module(PintaApi *api, PintaApiString *name);
uint32_t            pinta_api_set_global_string(PintaApi *api, void *module, PintaApiString *name, PintaApiString *value);
uint32_t            pinta_api_set_global_integer(PintaApi *api, void *module, PintaApiString *name, int32_t value);
uint32_t            pinta_api_set_global_null(PintaApi *api, void *module, PintaApiString *name);
uint32_t            pinta_api_execute_module(PintaApi *api, void *module);
uint32_t            pinta_api_get_global_string(PintaApi *core, void *module, PintaApiString *name, PintaApiEncoding encoding, uint32_t *string_length, void **string_data);
uint32_t            pinta_api_get_output_buffer(PintaApi *core, uint32_t *buffer_length, void **buffer_data);
uint32_t            pinta_api_get_output_string(PintaApi *core, PintaApiEncoding encoding, uint32_t *string_length, void **string_data);
                                       
PintaException      pinta_api_read_file(PintaCore *core, void *handle, u32 length, void *result);
PintaException      pinta_api_string_decode(PintaCore *core, PintaApiString *string, PintaReference *result);
PintaException      pinta_api_string_encode(PintaCore *core, PintaReference *string, PintaApiEncoding encoding, uint32_t *string_length, void **string_data);
PintaException      pinta_api_internal_out(PintaCore *core, PintaReference *arguments, PintaReference *return_value);

PintaException      pinta_api_core_file_open(PintaCore *core, PintaReference *file_name, void **file_handle);
PintaException      pinta_api_core_file_size(PintaCore *core, void *file_handle, u32 *file_size);
PintaException      pinta_api_core_file_read(PintaCore *core, void *file_handle, PintaReference *buffer, u32 length, u32 *actual_read);
PintaException      pinta_api_core_file_close(PintaCore *core, void *file_handle);
PintaException      pinta_api_core_load_module(PintaCore *core, PintaReference *module_name, PintaModuleDomain **domain);
PintaException      pinta_api_load_module_handle(PintaCore *core, void *handle, PintaModuleDomain **domain);

#ifdef __cplusplus
}
#endif


#endif /* PINTA_H */
