#include "pinta.h"

#if PINTA_DEBUG

void pinta_debug_raise_tick(PintaCore *core)
{
    PintaDebugger *debugger;

    pinta_assert(core != NULL);

    debugger = core->debugger;
    if (debugger != NULL)
        debugger->on_tick(core);
}

void pinta_debug_raise_exit(PintaCore *core, PintaException exception)
{
    PintaDebugger *debugger;

    pinta_assert(core != NULL);

    debugger = core->debugger;
    if (debugger != NULL)
        debugger->on_exit(core, exception);
}

void pinta_debug_raise_domain(PintaCore *core, PintaModule *module, PintaModuleDomain *domain)
{
    PintaDebugger *debugger;

    pinta_assert(core != NULL);

    debugger = core->debugger;
    if (debugger != NULL)
        debugger->on_domain(core, module, domain);
}

void pinta_debug_raise_step(PintaThread *thread)
{
    PintaCore *core;
    PintaDebugger *debugger;

    pinta_assert(thread != NULL);
    pinta_assert(thread->core != NULL);

    core = thread->core;

    debugger = core->debugger;
    if (debugger != NULL)
        debugger->on_step(core, thread);
}

void pinta_debug_raise_exception(PintaThread *thread, PintaException exception)
{
    PintaCore *core;
    PintaDebugger *debugger;

    pinta_assert(thread != NULL);
    pinta_assert(thread->core != NULL);

    if (exception == PINTA_OK)
        return;

    core = thread->core;

    debugger = core->debugger;
    if (debugger != NULL)
        debugger->on_exception(core, thread, exception);
}

void pinta_debug_raise_before_call(PintaThread *thread, u32 token, u32 arguments_count)
{
    PintaCore *core;
    PintaDebugger *debugger;

    pinta_assert(thread != NULL);
    pinta_assert(thread->core != NULL);

    core = thread->core;

    debugger = core->debugger;
    if (debugger != NULL)
        debugger->on_before_call(core, thread, token, arguments_count);
}

void pinta_debug_raise_after_call(PintaThread *thread, u32 is_tail_call)
{
    PintaCore *core;
    PintaDebugger *debugger;

    pinta_assert(thread != NULL);
    pinta_assert(thread->core != NULL);

    core = thread->core;

    debugger = core->debugger;
    if (debugger != NULL)
        debugger->on_after_call(core, thread, is_tail_call);
}

void pinta_debug_raise_before_call_internal(PintaThread *thread, u32 token, u32 arguments_count)
{
    PintaCore *core;
    PintaDebugger *debugger;

    pinta_assert(thread != NULL);
    pinta_assert(thread->core != NULL);

    core = thread->core;

    debugger = core->debugger;
    if (debugger != NULL)
        debugger->on_before_call_internal(core, thread, token, arguments_count);
}

void pinta_debug_raise_after_call_internal(PintaThread *thread)
{
    PintaCore *core;
    PintaDebugger *debugger;

    pinta_assert(thread != NULL);
    pinta_assert(thread->core != NULL);

    core = thread->core;

    debugger = core->debugger;
    if (debugger != NULL)
        debugger->on_after_call_internal(core, thread, 0);
}

void pinta_debug_raise_before_invoke(PintaThread *thread, u32 arguments_count, u8 has_this)
{
    PintaCore *core;
    PintaDebugger *debugger;

    pinta_assert(thread != NULL);
    pinta_assert(thread->core != NULL);

    core = thread->core;

    debugger = core->debugger;
    if (debugger != NULL)
        debugger->on_before_invoke(core, thread, arguments_count, has_this);
}

void pinta_debug_raise_after_invoke(PintaThread *thread, u32 is_tail_call)
{
    PintaCore *core;
    PintaDebugger *debugger;

    pinta_assert(thread != NULL);
    pinta_assert(thread->core != NULL);

    core = thread->core;

    debugger = core->debugger;
    if (debugger != NULL)
        debugger->on_after_invoke(core, thread, is_tail_call);
}

void pinta_debug_raise_before_return(PintaThread *thread)
{
    PintaCore *core;
    PintaDebugger *debugger;

    pinta_assert(thread != NULL);
    pinta_assert(thread->core != NULL);

    core = thread->core;

    debugger = core->debugger;
    if (debugger != NULL)
        debugger->on_before_return(core, thread);
}

void pinta_debug_raise_after_return(PintaThread *thread)
{
    PintaCore *core;
    PintaDebugger *debugger;

    pinta_assert(thread != NULL);
    pinta_assert(thread->core != NULL);

    core = thread->core;

    debugger = core->debugger;
    if (debugger != NULL)
        debugger->on_after_return(core, thread);
}

void pinta_debug_raise_break(PintaThread *thread, u8 *code)
{
    PintaCore *core;
    PintaDebugger *debugger;

    pinta_assert(thread != NULL);
    pinta_assert(thread->core != NULL);
    pinta_assert(code != NULL);

    core = thread->core;

    debugger = core->debugger;
    if (debugger != NULL)
        debugger->on_break(core, thread, code);
}

#endif /* PINTA_DEBUG */

PintaException pinta_lib_debug_write(PintaCore *core, PintaReference *value, u32 max_depth, PintaJsonWriter *writer)
{
    PintaType *type = pinta_core_get_type(core, value);
    return type->debug_write(core, value, max_depth, writer);
}
