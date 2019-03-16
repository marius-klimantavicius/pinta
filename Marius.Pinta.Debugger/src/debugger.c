#include "debugger.h"

#include <stdio.h>

/*

Goals:
* Support simple debugging with inspection and stepping through code
* If debugger is disabled the performance must not be affected

Debugger support:
* Emit debugger events on predefined locations
* During event handling a debugger can request any kind of data

Events:
* Step - before fetching next code
* Invoke/Call function - function has been called. Only frame has been created, code has not been executed.
* Return - just after the frame has been destroyed and return value (unless discarded) pushed into parent frame
* Module loaded
* Exception
*/

char command[8192];
u8 socket_buffer[8192];
u32 socket_offset;

char *pinta_code_names[] = {
    "NOP",
    "ADD",
    "SUBTRACT",
    "MULTIPLY",
    "DIVIDE",
    "REMAINDER",
    "BITWISE_AND",
    "BITWISE_OR",
    "EXCLUSIVE_OR",
    "BITWISE_EXCLUSIVE_OR",
    "NOT",
    "BITWISE_NOT",
    "NEGATE",
    "COMPARE_EQUAL",
    "COMPARE_LESS_THAN",
    "COMPARE_MORE_THAN",
    "COMPARE_NULL",
    "CONVERT_INTEGER",
    "CONVERT_DECIMAL",
    "CONVERT_STRING",
    "NEW_ARRAY",
    "CONCAT",
    "SUBSTRING",
    "JUMP",
    "JUMP_ZERO",
    "JUMP_NOT_ZERO",
    "CALL",
    "CALL_INTERNAL",
    "RETURN",
    "LOAD_NULL",
    "LOAD_INTEGER_ZERO",
    "LOAD_DECIMAL_ZERO",
    "LOAD_INTEGER_ONE",
    "LOAD_DECIMAL_ONE",
    "LOAD_INTEGER",
    "LOAD_STRING",
    "STORE_LOCAL",
    "STORE_GLOBAL",
    "STORE_ARGUMENT",
    "STORE_ITEM",
    "LOAD_LOCAL",
    "LOAD_GLOBAL",
    "LOAD_ARGUMENT",
    "LOAD_ITEM",
    "DUPLICATE",
    "POP",
    "EXIT",
    "GET_LENGTH",
    "ERROR",
    "LOAD_BLOB",
    "LOAD_CLOSURE",
    "LOAD_MEMBER",
    "LOAD_THIS",
    "STORE_CLOSURE",
    "STORE_MEMBER",
    "NEW",
    "NEW_FUNCTION",
    "NEW_OBJECT",
    "INVOKE",
    "INVOKE_MEMBER",
    "DUPLICATE_MULTIPLE",
    "MATCH",
    "BREAK"
};

char *pinta_exception_names[] = {
    "OK",
    "STACK_OVERFLOW",
    "STACK_UNDERFLOW",
    "TYPE_MISMATCH",
    "OUT_OF_MEMORY",
    "NULL_REFERENCE",
    "BAD_FORMAT",
    "OUT_OF_RANGE",
    "INVALID_OPERATION",
    "INVALID_OPCODE",
    "NOT_IMPLEMENTED",
    "ENGINE",
    "INDEX_OUT_OF_BOUNDS",
    "DIVISION_BY_ZERO",
    "INVALID_MODULE",
    "FILE_NOT_FOUND",
    "INVALID_ARGUMENTS",
    "PLATFORM",
    "NOT_REACHABLE",
    "INVALID_SEQUENCE"
};

char buffer[8192] = { 0 };

void pinta_debugger_context_flush(PintaWebContext *context)
{
    if (socket_offset > 0)
        pinta_web_context_response_write(context, socket_buffer, socket_offset);
    socket_offset = 0;
}

PintaException pinta_debugger_context_write(PintaCore *core, PintaJsonWriter *writer, wchar *string, u32 string_length)
{
    PintaWebContext *context = (PintaWebContext*)writer->data;

    PintaException exception = PINTA_OK;
    u32 index;
    u32 current;
    u32 next;
    u8 *data = &socket_buffer[socket_offset];
    u8 *end = &socket_buffer[sizeof(socket_buffer)];

    for (index = 0; index < string_length; index++)
    {
        current = (u32)string[index];

        if (current >= 0xD800 && current < 0xDC00)
        {
            if (index + 1 < string_length)
            {
                next = (u32)string[index + 1];
                if (next >= 0xDC00 && next < 0xC000)
                {
                    u32 high = current - 0xD800;
                    u32 low = next - 0xDC00;

                    current = ((high << 10) | low) + 0x10000;
                    index++;

                }
            }
        }

        if (current < 0x0080 && current != 0x0000)
        {
            if (data + 1 > end)
            {
                pinta_web_context_response_write(context, socket_buffer, socket_offset);
                data = &socket_buffer[0];
            }

            *data++ = (u8)current;
        }
        else if (current < 0x0800)
        {
            if (data + 2 > end)
            {
                pinta_web_context_response_write(context, socket_buffer, socket_offset);
                data = &socket_buffer[0];
            }

            *data++ = (u8)(((current >> 6) & 0x1F) | 0xC0);
            *data++ = (u8)((current & 0x3F) | 0x80);
        }
        else if (current < 0x10000)
        {
            if (data + 3 > end)
            {
                pinta_web_context_response_write(context, socket_buffer, socket_offset);
                data = &socket_buffer[0];
            }

            *data++ = (u8)(((current >> 12) & 0x0F) | 0xE0);
            *data++ = (u8)(((current >> 6) & 0x3F) | 0x80);
            *data++ = (u8)((current & 0x3F) | 0x80);
        }
        else
        {
            if (data + 4 > end)
            {
                pinta_web_context_response_write(context, socket_buffer, socket_offset);
                data = &socket_buffer[0];
            }

            *data++ = (u8)(((current >> 18) & 0x07) | 0xF0);
            *data++ = (u8)(((current >> 12) & 0x3F) | 0x80);
            *data++ = (u8)(((current >> 6) & 0x3F) | 0x80);
            *data++ = (u8)((current & 0x3F) | 0x80);
        }
    }

    socket_offset = (u32)(data - &socket_buffer[0]);

    return exception;
}

PintaDebuggerSession *pinta_debugger_get_session(void *user_data)
{
    PintaCore *core = (PintaCore*)user_data;
    PintaDebugger *debugger;

    if (core == NULL)
        return NULL;

    debugger = core->debugger;
    if (debugger == NULL)
        return NULL;

    return (PintaDebuggerSession*)debugger->session;
}

i32 pinta_debugger_on_request(PintaWebContext *context, void *user_data)
{
    PINTA_UNUSED(context);
    PINTA_UNUSED(user_data);

    return 1;
}

i32 pinta_debugger_on_ws_connect(PintaWebContext* context, void *user_data)
{
    PintaDebuggerSession *session = pinta_debugger_get_session(user_data);

    PINTA_UNUSED(context);

    if (session == NULL || session->context != NULL)
        return 1;

    return 0;
}

void pinta_debugger_on_ws_connected(PintaWebContext *context, void *user_data)
{
    PintaDebuggerSession *session = pinta_debugger_get_session(user_data);

    if (session == NULL || session->context != NULL)
        return;

    session->context = context;
}

void pinta_debugger_on_ws_closed(PintaWebContext *context, void *user_data)
{
    PintaDebuggerSession *session = pinta_debugger_get_session(user_data);

    if (session == NULL || session->context != context)
        return;

    session->context = NULL;
}

PintaJsonWriter pinta_debugger_get_writer(PintaWebContext *context)
{
    PintaJsonWriter writer = { 0 };

    writer.data = context;
    writer.stack = 0;
    writer.write = pinta_debugger_context_write;

    return writer;
}

void pinta_debugger_send_frame_stack(PintaCore *core, PintaWebContext *context, u32 index)
{
    PintaException exception = PINTA_OK;
    PintaThread *current = core->threads;
    PintaStackFrame *frame = current->frame;
    PintaJsonWriter writer = pinta_debugger_get_writer(context);

    while (index-- != 0 && frame != NULL)
        frame = frame->next;

    pinta_web_context_frame_begin(context, PINTA_WEB_WSOP_TEXT_FRAME);

    PINTA_CHECK(pinta_json_write_object_start(core, &writer));

    PINTA_CHECK(pinta_json_write_property_name_c(core, &writer, "event"));
    PINTA_CHECK(pinta_json_write_string_c(core, &writer, "frame stack", PINTA_JSON_LENGTH_AUTO));

    PINTA_CHECK(pinta_json_write_property_name_c(core, &writer, "data"));
    PINTA_CHECK(pinta_json_write_array_start(core, &writer));

    if (frame != NULL)
    {
        PintaReference *current = frame->stack_start;
        while (current <= frame->stack)
        {
            PINTA_CHECK(pinta_lib_debug_write(core, current, 2, &writer));
            current++;
        }
    }

    PINTA_CHECK(pinta_json_write_end(core, &writer));
    PINTA_CHECK(pinta_json_write_end(core, &writer));

PINTA_EXIT:

    pinta_debugger_context_flush(context);

    pinta_web_context_frame_end(context);
}

void pinta_debugger_send_frame_this(PintaCore *core, PintaWebContext *context, u32 index)
{
    PintaException exception = PINTA_OK;
    PintaThread *current = core->threads;
    PintaStackFrame *frame = current->frame;
    PintaJsonWriter writer = pinta_debugger_get_writer(context);

    while (index-- != 0 && frame != NULL)
        frame = frame->next;

    pinta_web_context_frame_begin(context, PINTA_WEB_WSOP_TEXT_FRAME);

    PINTA_CHECK(pinta_json_write_object_start(core, &writer));

    PINTA_CHECK(pinta_json_write_property_name_c(core, &writer, "event"));
    PINTA_CHECK(pinta_json_write_string_c(core, &writer, "frame this", PINTA_JSON_LENGTH_AUTO));

    PINTA_CHECK(pinta_json_write_property_name_c(core, &writer, "data"));

    if (frame != NULL)
    {
        PINTA_CHECK(pinta_lib_debug_write(core, &frame->function_this, 2, &writer));
    }
    else
    {
        PINTA_CHECK(pinta_json_write_null(core, &writer));
    }

    PINTA_CHECK(pinta_json_write_end(core, &writer));

PINTA_EXIT:

    pinta_debugger_context_flush(context);

    pinta_web_context_frame_end(context);
}

void pinta_debugger_send_frame_arguments(PintaCore *core, PintaWebContext *context, u32 index)
{
    PintaException exception = PINTA_OK;
    PintaThread *current = core->threads;
    PintaStackFrame *frame = current->frame;
    PintaJsonWriter writer = pinta_debugger_get_writer(context);

    while (index-- != 0 && frame != NULL)
        frame = frame->next;

    pinta_web_context_frame_begin(context, PINTA_WEB_WSOP_TEXT_FRAME);

    PINTA_CHECK(pinta_json_write_object_start(core, &writer));

    PINTA_CHECK(pinta_json_write_property_name_c(core, &writer, "event"));
    PINTA_CHECK(pinta_json_write_string_c(core, &writer, "frame arguments", PINTA_JSON_LENGTH_AUTO));

    PINTA_CHECK(pinta_json_write_property_name_c(core, &writer, "data"));

    if (frame != NULL)
    {
        PINTA_CHECK(pinta_lib_debug_write(core, &frame->function_arguments, 2, &writer));
    }
    else
    {
        PINTA_CHECK(pinta_json_write_null(core, &writer));
    }

    PINTA_CHECK(pinta_json_write_end(core, &writer));

PINTA_EXIT:

    pinta_debugger_context_flush(context);

    pinta_web_context_frame_end(context);
}

void pinta_debugger_send_frame_locals(PintaCore *core, PintaWebContext *context, u32 index)
{
    PintaException exception = PINTA_OK;
    PintaThread *current = core->threads;
    PintaStackFrame *frame = current->frame;
    PintaJsonWriter writer = pinta_debugger_get_writer(context);

    while (index-- != 0 && frame != NULL)
        frame = frame->next;

    pinta_web_context_frame_begin(context, PINTA_WEB_WSOP_TEXT_FRAME);

    PINTA_CHECK(pinta_json_write_object_start(core, &writer));

    PINTA_CHECK(pinta_json_write_property_name_c(core, &writer, "event"));
    PINTA_CHECK(pinta_json_write_string_c(core, &writer, "frame locals", PINTA_JSON_LENGTH_AUTO));

    PINTA_CHECK(pinta_json_write_property_name_c(core, &writer, "data"));

    if (frame != NULL)
    {
        PINTA_CHECK(pinta_lib_debug_write(core, &frame->function_locals, 2, &writer));
    }
    else
    {
        PINTA_CHECK(pinta_json_write_null(core, &writer));
    }

    PINTA_CHECK(pinta_json_write_end(core, &writer));

PINTA_EXIT:

    pinta_debugger_context_flush(context);

    pinta_web_context_frame_end(context);
}

void pinta_debugger_send_frame_closure(PintaCore *core, PintaWebContext *context, u32 index)
{
    PintaException exception = PINTA_OK;
    PintaThread *current = core->threads;
    PintaStackFrame *frame = current->frame;
    PintaJsonWriter writer = pinta_debugger_get_writer(context);

    while (index-- != 0 && frame != NULL)
        frame = frame->next;

    pinta_web_context_frame_begin(context, PINTA_WEB_WSOP_TEXT_FRAME);

    PINTA_CHECK(pinta_json_write_object_start(core, &writer));

    PINTA_CHECK(pinta_json_write_property_name_c(core, &writer, "event"));
    PINTA_CHECK(pinta_json_write_string_c(core, &writer, "frame closure", PINTA_JSON_LENGTH_AUTO));

    PINTA_CHECK(pinta_json_write_property_name_c(core, &writer, "data"));

    if (frame != NULL)
    {
        PINTA_CHECK(pinta_lib_debug_write(core, &frame->function_closure, 2, &writer));
    }
    else
    {
        PINTA_CHECK(pinta_json_write_null(core, &writer));
    }

    PINTA_CHECK(pinta_json_write_end(core, &writer));

PINTA_EXIT:

    pinta_debugger_context_flush(context);

    pinta_web_context_frame_end(context);
}

void pinta_debugger_send_frame_body(PintaCore *core, PintaWebContext *context, u32 index)
{
    PintaException exception = PINTA_OK;
    PintaThread *current = core->threads;
    PintaStackFrame *frame = current->frame;
    PintaJsonWriter writer = pinta_debugger_get_writer(context);

    while (index-- != 0 && frame != NULL)
        frame = frame->next;

    pinta_web_context_frame_begin(context, PINTA_WEB_WSOP_TEXT_FRAME);

    PINTA_CHECK(pinta_json_write_object_start(core, &writer));

    PINTA_CHECK(pinta_json_write_property_name_c(core, &writer, "event"));
    PINTA_CHECK(pinta_json_write_string_c(core, &writer, "frame body", PINTA_JSON_LENGTH_AUTO));

    PINTA_CHECK(pinta_json_write_property_name_c(core, &writer, "data"));

    if (frame != NULL)
    {
        PINTA_CHECK(pinta_json_write_binary(core, &writer, frame->code_start, frame->code_end - frame->code_start));
    }
    else
    {
        PINTA_CHECK(pinta_json_write_null(core, &writer));
    }

    PINTA_CHECK(pinta_json_write_end(core, &writer));

PINTA_EXIT:

    pinta_debugger_context_flush(context);

    pinta_web_context_frame_end(context);
}

void pinta_debugger_send_module_strings(PintaCore *core, PintaWebContext *context)
{
    PintaException exception = PINTA_OK;
    PintaThread *current = core->threads;
    PintaModuleDomain *domain = current->domain;
    u32 count = 0;
    PintaHeapObject *blob;
    u8 *data;
    u32 length;
    PintaJsonWriter writer = pinta_debugger_get_writer(context);

    struct
    {
        PintaReference string;
        PintaReference buffer;
    } gc;

    PINTA_GC_ENTER(core, gc);

    if (domain == NULL)
        domain = core->domains;

    if (domain != NULL && domain->module != NULL)
        count = domain->module->strings_length;

    pinta_web_context_frame_begin(context, PINTA_WEB_WSOP_TEXT_FRAME);

    PINTA_CHECK(pinta_json_write_object_start(core, &writer));

    PINTA_CHECK(pinta_json_write_property_name_c(core, &writer, "event"));
    PINTA_CHECK(pinta_json_write_string_c(core, &writer, "strings", PINTA_JSON_LENGTH_AUTO));

    PINTA_CHECK(pinta_json_write_property_name_c(core, &writer, "data"));
    PINTA_CHECK(pinta_json_write_array_start(core, &writer));

    for (u32 token = 0; token < count; token++)
    {
        PINTA_CHECK(pinta_lib_buffer_alloc(core, &gc.buffer));
        PINTA_CHECK(pinta_lib_module_get_string(core, domain, token, &gc.string));
        PINTA_CHECK(pinta_lib_buffer_write_string_utf8(core, &gc.buffer, &gc.string));

        blob = pinta_buffer_ref_get_blob(&gc.buffer);

        if (!blob)
            continue;

        pinta_core_pin(blob);

        data = pinta_blob_get_data(blob);
        length = pinta_buffer_ref_get_length(&gc.buffer);

        exception = pinta_json_write_string_c(core, &writer, data, length);

        pinta_core_unpin(blob);

        if (exception != PINTA_OK)
            goto PINTA_EXIT;
    }

    PINTA_CHECK(pinta_json_write_end(core, &writer));
    PINTA_CHECK(pinta_json_write_end(core, &writer));

PINTA_EXIT:

    pinta_debugger_context_flush(context);

    pinta_web_context_frame_end(context);

    PINTA_GC_EXIT(core);
}

void pinta_debugger_send_module_globals(PintaCore *core, PintaWebContext *context)
{
    PintaException exception = PINTA_OK;
    PintaThread *current = core->threads;
    PintaModuleDomain *domain = current->domain;
    PintaJsonWriter writer = pinta_debugger_get_writer(context);
    u32 count = 0;
    u32 name_token = 0;
    PintaHeapObject *blob;
    u8 *data;
    u32 length;

    struct
    {
        PintaReference string;
        PintaReference buffer;
    } gc;

    PINTA_GC_ENTER(core, gc);

    if (domain == NULL)
        domain = core->domains;

    if (domain != NULL && domain->globals.reference != NULL)
        count = pinta_array_ref_get_length(&domain->globals);

    pinta_web_context_frame_begin(context, PINTA_WEB_WSOP_TEXT_FRAME);

    PINTA_CHECK(pinta_json_write_object_start(core, &writer));

    PINTA_CHECK(pinta_json_write_property_name_c(core, &writer, "event"));
    PINTA_CHECK(pinta_json_write_string_c(core, &writer, "globals", PINTA_JSON_LENGTH_AUTO));

    PINTA_CHECK(pinta_json_write_property_name_c(core, &writer, "data"));
    PINTA_CHECK(pinta_json_write_array_start(core, &writer));

    for (u32 global_token = 0; global_token < count; global_token++)
    {
        PINTA_CHECK(pinta_json_write_object_start(core, &writer));
        PINTA_CHECK(pinta_json_write_property_name_c(core, &writer, "name"));

        PINTA_CHECK(pinta_module_get_global_name_token(domain, global_token, &name_token));

        PINTA_CHECK(pinta_lib_buffer_alloc(core, &gc.buffer));
        PINTA_CHECK(pinta_lib_module_get_string(core, domain, name_token, &gc.string));
        PINTA_CHECK(pinta_lib_buffer_write_string_utf8(core, &gc.buffer, &gc.string));

        blob = pinta_buffer_ref_get_blob(&gc.buffer);

        if (!blob)
            continue;

        pinta_core_pin(blob);

        data = pinta_blob_get_data(blob);
        length = pinta_buffer_ref_get_length(&gc.buffer);

        exception = pinta_json_write_string_c(core, &writer, data, length);

        pinta_core_unpin(blob);

        if (exception != PINTA_OK)
            goto PINTA_EXIT;

        PINTA_CHECK(pinta_json_write_property_name_c(core, &writer, "value"));
        PINTA_CHECK(pinta_lib_array_get_item(core, &domain->globals, global_token, &gc.string));
        PINTA_CHECK(pinta_lib_debug_write(core, &gc.string, 2, &writer));

        PINTA_CHECK(pinta_json_write_end(core, &writer));
    }

    PINTA_CHECK(pinta_json_write_end(core, &writer));
    PINTA_CHECK(pinta_json_write_end(core, &writer));

PINTA_EXIT:

    pinta_debugger_context_flush(context);

    pinta_web_context_frame_end(context);

    PINTA_GC_EXIT(core);
}

void pinta_debugger_step(PintaCore *core, PintaWebContext *context)
{
    PintaDebuggerSession *session = pinta_debugger_get_session(core);
    PintaThread *current = core->threads;

    session->is_processing_commands = 0;
    while (current)
    {
        current->code_is_suspended = 0;
        current = current->next;
    }
}

void pinta_debugger_run(PintaCore *core, PintaWebContext *context)
{
    PintaDebuggerSession *session = pinta_debugger_get_session(core);
    PintaThread *current = core->threads;

    session->is_processing_commands = 0;
    session->mode = 0;
    while (current)
    {
        current->code_is_suspended = 0;
        current = current->next;
    }
}

void pinta_debugger_process_commands(PintaDebuggerSession *session)
{
    if ((session->mode & 2) != 0)
        session->is_processing_commands = 1;

    while (session->is_processing_commands)
        pinta_web_server_update(&session->server);
}

i32 pinta_debugger_on_ws_frame(PintaWebContext *context, const PintaWebFrame *frame, void *user_data)
{
    PintaException exception = PINTA_OK;
    PintaCore *core = (PintaCore*)user_data;
    PintaDebuggerSession *session = pinta_debugger_get_session(user_data);

    if (session == NULL || session->context != context)
        return 1;

    if (frame->opcode != PINTA_WEB_WSOP_TEXT_FRAME)
        return 0;

    if (pinta_web_context_request_read(context, command, frame->payload_length) != PINTA_WEB_OK)
        return 1;

    command[frame->payload_length] = 0;

    exception = pinta_debugger_command_execute(core, context, command, frame->payload_length + 1);
    if (exception != PINTA_OK)
        return 1;

    return 0;
}

void pinta_debugger_on_tick(PintaCore *core)
{
    PintaDebugger *debugger;
    PintaDebuggerSession *session;
    debugger = core->debugger;
    session = (PintaDebuggerSession *)debugger->session;

    if (session->context == NULL)
        return;

    pinta_web_server_update(&session->server);
}

void pinta_debugger_on_exit(PintaCore *core, PintaException exception)
{
    PintaDebugger *debugger;
    PintaDebuggerSession *session;
    char *name;
    PintaJsonWriter writer;

    debugger = core->debugger;
    session = (PintaDebuggerSession *)debugger->session;

    if (session->context == NULL)
        return;

    name = pinta_exception_names[exception];

    writer = pinta_debugger_get_writer(session->context);

    pinta_web_context_frame_begin(session->context, PINTA_WEB_WSOP_TEXT_FRAME);

    pinta_json_write_object_start(core, &writer);

    pinta_json_write_property_name_c(core, &writer, "event");
    pinta_json_write_string_c(core, &writer, "exit", PINTA_JSON_LENGTH_AUTO);

    pinta_json_write_property_name_c(core, &writer, "exception");
    pinta_json_write_string_c(core, &writer, name, string_get_length_c(name));

    pinta_json_write_end(core, &writer);

    pinta_debugger_context_flush(session->context);

    pinta_web_context_frame_end(session->context);

    pinta_debugger_process_commands(session);
}

void pinta_debugger_on_domain(PintaCore *core, PintaModule *module, PintaModuleDomain *domain)
{
    PintaException exception = PINTA_OK;
    PintaDebugger *debugger;
    PintaDebuggerSession *session;
    PintaJsonWriter writer = { 0 };

    debugger = core->debugger;
    session = (PintaDebuggerSession *)debugger->session;

    if (session->context == NULL)
        return;

    writer = pinta_debugger_get_writer(session->context);

    pinta_web_context_frame_begin(session->context, PINTA_WEB_WSOP_TEXT_FRAME);

    pinta_json_write_object_start(core, &writer);

    pinta_json_write_property_name_c(core, &writer, "event");
    pinta_json_write_string_c(core, &writer, "domain", PINTA_JSON_LENGTH_AUTO);

    pinta_json_write_property_name_c(core, &writer, "id");
    pinta_json_write_i32(core, &writer, (i32)domain);

    pinta_json_write_end(core, &writer);

    pinta_debugger_context_flush(session->context);

    pinta_web_context_frame_end(session->context);

    pinta_debugger_process_commands(session);
}

void pinta_debugger_on_step(PintaCore *core, PintaThread *thread)
{
    PintaDebugger *debugger;
    PintaDebuggerSession *session;
    i32 offset;
    u8 code;
    u32 token;
    u32 count;
    char *name;
    u8 *code_pointer;
    u8 flags;
    PintaJsonWriter writer = { 0 };

    debugger = core->debugger;
    session = (PintaDebuggerSession *)debugger->session;

    if (session->context == NULL)
        return;

    code_pointer = thread->code_next_pointer;

    pinta_code_next(thread, &code);

    pinta_web_context_frame_begin(session->context, PINTA_WEB_WSOP_TEXT_FRAME);

    name = pinta_code_names[code];

    writer = pinta_debugger_get_writer(session->context);

    pinta_json_write_object_start(core, &writer);

    pinta_json_write_property_name_c(core, &writer, "event");
    pinta_json_write_string_c(core, &writer, "step", PINTA_JSON_LENGTH_AUTO);

    pinta_json_write_property_name_c(core, &writer, "opcode");
    pinta_json_write_string_c(core, &writer, name, string_get_length_c(name));

    pinta_json_write_property_name_c(core, &writer, "arguments");
    pinta_json_write_array_start(core, &writer);

    switch (code)
    {
    case PINTA_CODE_JUMP:
    case PINTA_CODE_JUMP_ZERO:
    case PINTA_CODE_JUMP_NOT_ZERO:
    case PINTA_CODE_LOAD_INTEGER:
        pinta_code_integer(thread, &offset);
        pinta_json_write_i32(core, &writer, offset);
        break;
    case PINTA_CODE_CALL:
    case PINTA_CODE_CALL_INTERNAL:
    case PINTA_CODE_LOAD_CLOSURE:
    case PINTA_CODE_STORE_CLOSURE:
        pinta_code_token(thread, &token);
        pinta_code_token(thread, &count);
        pinta_json_write_i32(core, &writer, (i32)token);
        pinta_json_write_i32(core, &writer, (i32)count);
        break;
    case PINTA_CODE_LOAD_STRING:
    case PINTA_CODE_STORE_LOCAL:
    case PINTA_CODE_STORE_GLOBAL:
    case PINTA_CODE_STORE_ARGUMENT:
    case PINTA_CODE_LOAD_LOCAL:
    case PINTA_CODE_LOAD_GLOBAL:
    case PINTA_CODE_LOAD_ARGUMENT:
    case PINTA_CODE_LOAD_BLOB:
    case PINTA_CODE_NEW:
    case PINTA_CODE_NEW_FUNCTION:
    case PINTA_CODE_INVOKE:
    case PINTA_CODE_INVOKE_MEMBER:
    case PINTA_CODE_DUPLICATE_MULTIPLE:
        pinta_code_token(thread, &token);
        pinta_json_write_i32(core, &writer, (i32)token);
        break;
    case PINTA_CODE_MATCH:
        pinta_code_next(thread, &flags);
        pinta_json_write_i32(core, &writer, (i32)flags);
        break;
    }

    pinta_json_write_end(core, &writer);

    pinta_json_write_end(core, &writer);

    pinta_debugger_context_flush(session->context);

    pinta_web_context_frame_end(session->context);

    thread->code_next_pointer = code_pointer;

    if ((session->mode & 1) != 0)
        thread->code_is_suspended = 1;
}

void pinta_debugger_on_exception(PintaCore *core, PintaThread *thread, PintaException exception)
{
    PintaDebugger *debugger;
    PintaDebuggerSession *session;
    char *name;
    PintaJsonWriter writer = { 0 };

    debugger = core->debugger;
    session = debugger->session;

    if (session->context == NULL)
        return;

    name = pinta_exception_names[exception];

    writer = pinta_debugger_get_writer(session->context);

    pinta_web_context_frame_begin(session->context, PINTA_WEB_WSOP_TEXT_FRAME);

    pinta_json_write_object_start(core, &writer);

    pinta_json_write_property_name_c(core, &writer, "event");
    pinta_json_write_string_c(core, &writer, "exception", PINTA_JSON_LENGTH_AUTO);

    pinta_json_write_property_name_c(core, &writer, "type");
    pinta_json_write_string_c(core, &writer, name, string_get_length_c(name));

    pinta_debugger_context_flush(session->context);

    pinta_web_context_frame_end(session->context);

    pinta_debugger_process_commands(session);
}

void pinta_debugger_on_before_call(PintaCore *core, PintaThread *thread, u32 token, u32 arguments_count)
{
    PintaException exception = PINTA_OK;
    PintaDebugger *debugger;
    PintaDebuggerSession *session;
    PintaJsonWriter writer = { 0 };

    debugger = core->debugger;
    session = (PintaDebuggerSession *)debugger->session;

    if (session->context == NULL)
        return;

    writer = pinta_debugger_get_writer(session->context);

    pinta_web_context_frame_begin(session->context, PINTA_WEB_WSOP_TEXT_FRAME);

    pinta_json_write_object_start(core, &writer);

    pinta_json_write_property_name_c(core, &writer, "event");
    pinta_json_write_string_c(core, &writer, "before call", PINTA_JSON_LENGTH_AUTO);

    pinta_json_write_property_name_c(core, &writer, "token");
    pinta_json_write_i32(core, &writer, (i32)token);

    pinta_json_write_property_name_c(core, &writer, "agrument_count");
    pinta_json_write_i32(core, &writer, (i32)arguments_count);

    pinta_json_write_end(core, &writer);

    pinta_debugger_context_flush(session->context);

    pinta_web_context_frame_end(session->context);

    pinta_debugger_process_commands(session);
}

void pinta_debugger_on_after_call(PintaCore *core, PintaThread *thread)
{
    PintaException exception = PINTA_OK;
    PintaDebugger *debugger;
    PintaDebuggerSession *session;
    PintaJsonWriter writer = { 0 };

    debugger = core->debugger;
    session = (PintaDebuggerSession *)debugger->session;

    if (session->context == NULL)
        return;

    writer = pinta_debugger_get_writer(session->context);

    pinta_web_context_frame_begin(session->context, PINTA_WEB_WSOP_TEXT_FRAME);

    pinta_json_write_object_start(core, &writer);

    pinta_json_write_property_name_c(core, &writer, "event");
    pinta_json_write_string_c(core, &writer, "after call", PINTA_JSON_LENGTH_AUTO);

    pinta_json_write_end(core, &writer);

    pinta_debugger_context_flush(session->context);

    pinta_web_context_frame_end(session->context);

    pinta_debugger_process_commands(session);
}

void pinta_debugger_on_before_call_internal(PintaCore *core, PintaThread *thread, u32 token, u32 arguments_count)
{
    PintaException exception = PINTA_OK;
    PintaDebugger *debugger;
    PintaDebuggerSession *session;
    PintaJsonWriter writer = { 0 };

    debugger = core->debugger;
    session = (PintaDebuggerSession *)debugger->session;

    if (session->context == NULL)
        return;

    writer = pinta_debugger_get_writer(session->context);

    pinta_web_context_frame_begin(session->context, PINTA_WEB_WSOP_TEXT_FRAME);

    pinta_json_write_object_start(core, &writer);

    pinta_json_write_property_name_c(core, &writer, "event");
    pinta_json_write_string_c(core, &writer, "before call internal", PINTA_JSON_LENGTH_AUTO);

    pinta_json_write_property_name_c(core, &writer, "token");
    pinta_json_write_i32(core, &writer, (i32)token);

    pinta_json_write_property_name_c(core, &writer, "agrument_count");
    pinta_json_write_i32(core, &writer, (i32)arguments_count);

    pinta_json_write_end(core, &writer);

    pinta_debugger_context_flush(session->context);

    pinta_web_context_frame_end(session->context);

    pinta_debugger_process_commands(session);
}

void pinta_debugger_on_after_call_internal(PintaCore *core, PintaThread *thread)
{
    PintaException exception = PINTA_OK;
    PintaDebugger *debugger;
    PintaDebuggerSession *session;
    PintaJsonWriter writer = { 0 };

    debugger = core->debugger;
    session = (PintaDebuggerSession *)debugger->session;

    if (session->context == NULL)
        return;

    writer = pinta_debugger_get_writer(session->context);

    pinta_web_context_frame_begin(session->context, PINTA_WEB_WSOP_TEXT_FRAME);

    pinta_json_write_object_start(core, &writer);

    pinta_json_write_property_name_c(core, &writer, "event");
    pinta_json_write_string_c(core, &writer, "after call internal", PINTA_JSON_LENGTH_AUTO);

    pinta_json_write_end(core, &writer);

    pinta_debugger_context_flush(session->context);

    pinta_web_context_frame_end(session->context);

    pinta_debugger_process_commands(session);
}

void pinta_debugger_on_before_invoke(PintaCore *core, PintaThread *thread, u32 arguments_count, u8 has_this)
{
    PintaException exception = PINTA_OK;
    PintaDebugger *debugger;
    PintaDebuggerSession *session;
    PintaJsonWriter writer = { 0 };

    debugger = core->debugger;
    session = (PintaDebuggerSession *)debugger->session;

    if (session->context == NULL)
        return;

    writer = pinta_debugger_get_writer(session->context);

    pinta_web_context_frame_begin(session->context, PINTA_WEB_WSOP_TEXT_FRAME);

    pinta_json_write_object_start(core, &writer);

    pinta_json_write_property_name_c(core, &writer, "event");
    pinta_json_write_string_c(core, &writer, "before invoke", PINTA_JSON_LENGTH_AUTO);

    pinta_json_write_property_name_c(core, &writer, "agrument_count");
    pinta_json_write_i32(core, &writer, (i32)arguments_count);

    pinta_json_write_property_name_c(core, &writer, "has_this");
    pinta_json_write_bool(core, &writer, has_this);

    pinta_json_write_end(core, &writer);

    pinta_debugger_context_flush(session->context);

    pinta_web_context_frame_end(session->context);

    pinta_debugger_process_commands(session);
}

void pinta_debugger_on_after_invoke(PintaCore *core, PintaThread *thread)
{
    PintaException exception = PINTA_OK;
    PintaDebugger *debugger;
    PintaDebuggerSession *session;
    PintaJsonWriter writer = { 0 };

    debugger = core->debugger;
    session = (PintaDebuggerSession *)debugger->session;

    if (session->context == NULL)
        return;

    writer = pinta_debugger_get_writer(session->context);

    pinta_web_context_frame_begin(session->context, PINTA_WEB_WSOP_TEXT_FRAME);

    pinta_json_write_object_start(core, &writer);

    pinta_json_write_property_name_c(core, &writer, "event");
    pinta_json_write_string_c(core, &writer, "after invoke", PINTA_JSON_LENGTH_AUTO);

    pinta_json_write_end(core, &writer);

    pinta_debugger_context_flush(session->context);

    pinta_web_context_frame_end(session->context);

    pinta_debugger_process_commands(session);
}

void pinta_debugger_on_before_return(PintaCore *core, PintaThread *thread)
{
    PintaException exception = PINTA_OK;
    PintaDebugger *debugger;
    PintaDebuggerSession *session;
    PintaJsonWriter writer = { 0 };

    debugger = core->debugger;
    session = (PintaDebuggerSession *)debugger->session;

    if (session->context == NULL)
        return;

    writer = pinta_debugger_get_writer(session->context);

    pinta_web_context_frame_begin(session->context, PINTA_WEB_WSOP_TEXT_FRAME);

    pinta_json_write_object_start(core, &writer);

    pinta_json_write_property_name_c(core, &writer, "event");
    pinta_json_write_string_c(core, &writer, "before return", PINTA_JSON_LENGTH_AUTO);

    pinta_json_write_end(core, &writer);

    pinta_debugger_context_flush(session->context);

    pinta_web_context_frame_end(session->context);

    pinta_debugger_process_commands(session);
}

void pinta_debugger_on_after_return(PintaCore *core, PintaThread *thread)
{
    PintaException exception = PINTA_OK;
    PintaDebugger *debugger;
    PintaDebuggerSession *session;
    PintaJsonWriter writer = { 0 };

    debugger = core->debugger;
    session = (PintaDebuggerSession *)debugger->session;

    if (session->context == NULL)
        return;

    writer = pinta_debugger_get_writer(session->context);

    pinta_web_context_frame_begin(session->context, PINTA_WEB_WSOP_TEXT_FRAME);

    pinta_json_write_object_start(core, &writer);

    pinta_json_write_property_name_c(core, &writer, "event");
    pinta_json_write_string_c(core, &writer, "after return", PINTA_JSON_LENGTH_AUTO);

    pinta_json_write_end(core, &writer);

    pinta_debugger_context_flush(session->context);

    pinta_web_context_frame_end(session->context);

    pinta_debugger_process_commands(session);
}

void pinta_debugger_on_break(PintaCore *core, PintaThread *thread, u8 *code)
{
    PintaException exception = PINTA_OK;
    PintaDebugger *debugger;
    PintaDebuggerSession *session;
    PintaJsonWriter writer = { 0 };

    PINTA_UNUSED(code);

    debugger = core->debugger;
    session = (PintaDebuggerSession *)debugger->session;

    if (session->context == NULL)
        return;

    writer = pinta_debugger_get_writer(session->context);

    pinta_web_context_frame_begin(session->context, PINTA_WEB_WSOP_TEXT_FRAME);

    pinta_json_write_object_start(core, &writer);

    pinta_json_write_property_name_c(core, &writer, "event");
    pinta_json_write_string_c(core, &writer, "break", PINTA_JSON_LENGTH_AUTO);

    pinta_json_write_end(core, &writer);

    pinta_debugger_context_flush(session->context);

    pinta_web_context_frame_end(session->context);

    pinta_debugger_process_commands(session);
}

PintaException pinta_api_debugger_start(PintaApi *api, const char* address, u16 port)
{
    PintaCore *core;

    if (api == NULL)
        return PINTA_EXCEPTION_NULL_REFERENCE;

    core = (PintaCore*)api->core;
    if (core == NULL)
        return PINTA_EXCEPTION_NULL_REFERENCE;

    return pinta_debugger_start(core, address, port);
}

PintaException pinta_api_debugger_stop(PintaApi *api)
{
    PintaCore *core;

    if (api == NULL)
        return PINTA_EXCEPTION_NULL_REFERENCE;

    core = (PintaCore*)api->core;
    if (core == NULL)
        return PINTA_EXCEPTION_NULL_REFERENCE;

    return pinta_debugger_stop(core);
}

PintaException pinta_api_debugger_wait(PintaApi *api)
{
    PintaCore *core;

    if (api == NULL)
        return PINTA_EXCEPTION_NULL_REFERENCE;

    core = (PintaCore*)api->core;
    if (core == NULL)
        return PINTA_EXCEPTION_NULL_REFERENCE;

    return pinta_debugger_wait(core);
}

void pinta_debugger_trace(const char *message)
{
    printf("%s\r\n", message);
}

PintaException pinta_debugger_start(PintaCore *core, const char *address, u16 port)
{
    PintaException exception = PINTA_OK;
    PintaDebugger *debugger = NULL;
    PintaDebuggerSession *session = NULL;
    uint32_t needed_memory = 0;
    void *memory = NULL;
    PintaWebConfiguration configuration = { 0 };

    if (core == NULL)
        return PINTA_EXCEPTION_NULL_REFERENCE;

    if (core->debugger != NULL)
        return PINTA_EXCEPTION_INVALID_OPERATION;

    configuration.address = address;
    configuration.port = port;
    configuration.connection_max = 4;
    configuration.request_buffer_length = 2048;
    configuration.io_buffer_length = 8192;
    configuration.on_request = pinta_debugger_on_request;
    configuration.on_ws_connect = pinta_debugger_on_ws_connect;
    configuration.on_ws_connected = pinta_debugger_on_ws_connected;
    configuration.on_ws_frame = pinta_debugger_on_ws_frame;
    configuration.on_ws_closed = pinta_debugger_on_ws_closed;
    configuration.log = pinta_debugger_trace;
    configuration.user_data = core;

    debugger = (PintaDebugger*)pinta_memory_alloc(core->memory, sizeof(PintaDebugger));
    if (debugger == NULL)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_MEMORY);

    debugger->on_after_call = pinta_debugger_on_after_call;
    debugger->on_after_call_internal = pinta_debugger_on_after_call_internal;
    debugger->on_after_invoke = pinta_debugger_on_after_invoke;
    debugger->on_after_return = pinta_debugger_on_after_return;
    debugger->on_before_call = pinta_debugger_on_before_call;
    debugger->on_before_call_internal = pinta_debugger_on_before_call_internal;
    debugger->on_before_invoke = pinta_debugger_on_before_invoke;
    debugger->on_before_return = pinta_debugger_on_before_return;
    debugger->on_break = pinta_debugger_on_break;
    debugger->on_domain = pinta_debugger_on_domain;
    debugger->on_exception = pinta_debugger_on_exception;
    debugger->on_step = pinta_debugger_on_step;
    debugger->on_tick = pinta_debugger_on_tick;
    debugger->on_exit = pinta_debugger_on_exit;
    debugger->session = NULL;

    session = (PintaDebuggerSession*)pinta_memory_alloc(core->memory, sizeof(PintaDebuggerSession));
    if (session == NULL)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_MEMORY);

    session->context = NULL;
    session->mode = 0;

    /* compute and allocate needed memory and start server */
    pinta_web_server_init(&session->server, &configuration, &needed_memory);

    memory = pinta_memory_alloc(core->memory, needed_memory);
    if (memory == NULL)
        PINTA_THROW(PINTA_EXCEPTION_OUT_OF_MEMORY);

    pinta_web_server_start(&session->server, memory);

    session->memory = memory;
    debugger->session = session;
    core->debugger = debugger;

PINTA_EXIT:

    if (exception != PINTA_OK)
    {
        core->debugger = NULL;

        if (memory != NULL)
            pinta_memory_free(core->memory, memory);

        if (session != NULL)
            pinta_memory_free(core->memory, session);

        if (debugger != NULL)
            pinta_memory_free(core->memory, debugger);
    }

    return exception;
}

PintaException pinta_debugger_stop(PintaCore *core)
{
    PintaDebugger *debugger = NULL;
    PintaDebuggerSession *session = NULL;

    if (core == NULL)
        return PINTA_EXCEPTION_NULL_REFERENCE;

    debugger = core->debugger;
    if (debugger == NULL)
        return PINTA_OK;

    session = debugger->session;
    if (session == NULL)
        return PINTA_EXCEPTION_INVALID_OPERATION;

    core->debugger = NULL;

    pinta_web_server_stop(&session->server);

    if (session->memory != NULL)
        pinta_memory_free(core->memory, session->memory);

    pinta_memory_free(core->memory, session);
    pinta_memory_free(core->memory, debugger);

    return PINTA_OK;
}

PintaException pinta_debugger_wait(PintaCore *core)
{
    PintaDebugger *debugger = NULL;
    PintaDebuggerSession *session = NULL;

    if (core == NULL)
        return PINTA_EXCEPTION_NULL_REFERENCE;

    debugger = core->debugger;
    if (debugger == NULL)
        return PINTA_OK;

    session = debugger->session;
    if (session == NULL)
        return PINTA_EXCEPTION_INVALID_OPERATION;

    while (session->context == NULL)
        pinta_web_server_update(&session->server);

    session->mode = 3;

    return PINTA_OK;
}