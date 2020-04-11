#include "pinta.h"

static wchar exports_literal[] = { 101, 120, 112, 111, 114, 116, 115, 0 };

PintaException pinta_code_rt_format(PintaCore *core, PintaReference *arguments, PintaReference *return_value)
{
    PintaException exception = PINTA_OK;
    u32 arguments_length;
    PintaReference format_string;

    pinta_assert(arguments != NULL);
    pinta_assert(return_value != NULL);
    pinta_assert(arguments->reference != NULL);
    pinta_assert(arguments->reference->block_kind == PINTA_KIND_ARRAY);
    pinta_debug_assert_code(core->threads);

    arguments_length = pinta_array_ref_get_length(arguments);
    if (arguments_length == 0)
    {
        return_value->reference = NULL;
        return (PINTA_OK);
    }

    PINTA_GC_ENTER(core, format_string);

    PINTA_CHECK(pinta_lib_array_get_item(core, arguments, 0, &format_string));

    if (format_string.reference == NULL)
        PINTA_THROW(PINTA_EXCEPTION_NULL_REFERENCE);

    PINTA_CHECK(pinta_lib_format(core, NULL, &format_string, arguments, 1, return_value));

    PINTA_GC_RETURN(core);
}

PintaException pinta_code_rt_require(PintaCore *core, PintaThread *thread, PintaReference *function, PintaReference *function_this, PintaReference *function_arguments, u8 *discard_return_value, PintaReference *return_value)
{
    PintaException exception = PINTA_OK;
    PintaModuleDomain *domain;
    PintaModule *module;
    u32 token;
    struct
    {
        PintaReference module_name;
        PintaReference start_function;
        PintaReference exports_name;
        PintaReference exports_object;
    } gc;

    pinta_assert(thread != NULL);
    pinta_assert(core != NULL);
    pinta_assert(core->environment != NULL);
    pinta_assert(core->environment->load_module != NULL);
    pinta_assert(function != NULL);
    pinta_assert(function_this != NULL);
    pinta_assert(function_arguments != NULL);
    pinta_assert(return_value != NULL);

    if (thread != NULL && thread->core != core)
        return (PINTA_EXCEPTION_ENGINE);

    PINTA_GC_ENTER(core, gc);

    PINTA_CHECK(pinta_lib_array_get_item(core, function_arguments, 0, &gc.module_name));

    PINTA_CHECK(core->environment->load_module(core, &gc.module_name, &domain));

    module = domain->module;
    PINTA_CHECK(pinta_lib_function_managed_alloc(core, domain, module->start_function_token, NULL, NULL, NULL, NULL, &gc.start_function));

    PINTA_CHECK(pinta_lib_string_alloc_value(core, exports_literal, PINTA_LITERAL_LENGTH(exports_literal), &gc.exports_name));
    PINTA_CHECK(pinta_lib_module_get_global_token(core, domain, &gc.exports_name, &token));
    
    if (token != PINTA_CODE_TOKEN_EMPTY)
    {
        PINTA_CHECK(pinta_lib_object_alloc(core, NULL, NULL, &gc.exports_object));
        PINTA_CHECK(pinta_lib_array_set_item(core, &domain->globals, token, &gc.exports_object));
        PINTA_CHECK(pinta_lib_domain_global_alloc(core, domain, token, &gc.exports_object));
        PINTA_CHECK(pinta_lib_stack_push(thread, &gc.exports_object));
    }
    else
    {
        PINTA_CHECK(pinta_lib_stack_push_null(thread));
    }

    PINTA_CHECK(pinta_lib_stack_push(thread, &gc.start_function));
    PINTA_CHECK(pinta_code_invoke(thread, 0, 0));

    pinta_frame_set_discard_result(thread->frame, 1);

    if (discard_return_value)
        *discard_return_value = 1;

    PINTA_GC_RETURN(core);
}
