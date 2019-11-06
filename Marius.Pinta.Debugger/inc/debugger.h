#ifndef PINTA_DEBUGGER_H
#define PINTA_DEBUGGER_H

#include "pinta.h"
#include "web.h"

typedef struct PintaDebuggerSession PintaDebuggerSession;
typedef struct PintaWebServer PintaWebServer;
typedef struct PintaWebContext PintaWebContext;
typedef struct PintaWebFrame PintaWebFrame;
typedef struct PintaWebConfiguration PintaWebConfiguration;

struct PintaDebuggerSession
{
    PintaWebServer server;
    void *memory;

    PintaWebContext *context;
    u32 mode;
    u32 is_processing_commands;
};

// ***********************************************
// Debugger
// ***********************************************

PintaException      pinta_debugger_command_execute(PintaCore *core, PintaWebContext *context, u8 *buffer, u32 buffer_length);

PintaException      pinta_debugger_start(PintaCore *core, const char *address, u16 port);
PintaException      pinta_debugger_stop(PintaCore *core);
PintaException      pinta_debugger_wait(PintaCore *core);

void                pinta_debugger_send_frame_stack(PintaCore *core, PintaWebContext *context, u32 index);
void                pinta_debugger_send_frame_this(PintaCore * core, PintaWebContext * context, u32 index);
void                pinta_debugger_send_frame_arguments(PintaCore * core, PintaWebContext * context, u32 index);
void                pinta_debugger_send_frame_locals(PintaCore * core, PintaWebContext * context, u32 index);
void                pinta_debugger_send_frame_closure(PintaCore * core, PintaWebContext * context, u32 index);
void                pinta_debugger_send_frame_body(PintaCore *core, PintaWebContext *context, u32 index);
void                pinta_debugger_send_module_strings(PintaCore * core, PintaWebContext * context);
void                pinta_debugger_send_module_globals(PintaCore * core, PintaWebContext * context);

void                pinta_debugger_step(PintaCore *core, PintaWebContext *context);
void                pinta_debugger_run(PintaCore *core, PintaWebContext *context);

void                pinta_debugger_on_tick(PintaCore *core);
void                pinta_debugger_on_exit(PintaCore *core, PintaException exception);
void                pinta_debugger_on_domain(PintaCore *core, PintaModule *module, PintaModuleDomain *domain);
void                pinta_debugger_on_step(PintaCore *core, PintaThread *thread);
void                pinta_debugger_on_exception(PintaCore *core, PintaThread *thread, PintaException exception);
void                pinta_debugger_on_before_call(PintaCore *core, PintaThread *thread, u32 token, u32 arguments_count);
void                pinta_debugger_on_after_call(PintaCore *core, PintaThread *thread, u32 is_tail_call);
void                pinta_debugger_on_before_call_internal(PintaCore *core, PintaThread *thread, u32 token, u32 arguments_count);
void                pinta_debugger_on_after_call_internal(PintaCore *core, PintaThread *thread, u32 is_tail_call);
void                pinta_debugger_on_before_invoke(PintaCore *core, PintaThread *thread, u32 arguments_count, u8 has_this);
void                pinta_debugger_on_after_invoke(PintaCore *core, PintaThread *thread, u32 is_tail_call);
void                pinta_debugger_on_before_return(PintaCore *core, PintaThread *thread);
void                pinta_debugger_on_after_return(PintaCore *core, PintaThread *thread);
void                pinta_debugger_on_break(PintaCore *core, PintaThread *thread, u8 *code);

// ***********************************************
// API
// ***********************************************

PintaException      pinta_api_debugger_start(PintaApi *api, const char *address, u16 port);
PintaException      pinta_api_debugger_stop(PintaApi *api);
PintaException      pinta_api_debugger_wait(PintaApi *api);

#endif // PINTA_DEBUGGER_H