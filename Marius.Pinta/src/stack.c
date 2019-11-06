#include "pinta.h"

PintaException pinta_frame_init(PintaThread *thread, PintaNativeMemory *memory, u32 length_in_bytes)
{
    u8 *stack_start, *stack_end;
    PintaStackFrame *frame;

    if (memory == NULL)
        return PINTA_EXCEPTION_INVALID_ARGUMENTS;

    if (length_in_bytes < sizeof(PintaStackFrame))
        return PINTA_EXCEPTION_INVALID_ARGUMENTS;

    frame = (PintaStackFrame*)pinta_memory_alloc(memory, sizeof(PintaStackFrame) + length_in_bytes);

    stack_start = (u8*)&frame[1];
    stack_end = ((u8*)frame) + (sizeof(PintaStackFrame) + length_in_bytes);
    
    thread->stack_end = (PintaReference*)stack_end;

    frame->return_address = NULL;
    frame->stack_start = (PintaReference*)stack_start;
    frame->stack = &frame->stack_start[-1];
    frame->code_start = NULL;
    frame->code_end = NULL;
    frame->function_this.reference = NULL;
    frame->function_closure.reference = NULL;
    frame->function_arguments.reference = NULL;
    frame->function_locals.reference = NULL;
    frame->prev = NULL;
    frame->is_final_frame = 0;
    frame->discard_result = 0;
    frame->return_domain = NULL;

    thread->frame = frame;
    thread->frame->code_end = NULL;
    thread->frame->code_start = NULL;
    thread->frame->is_final_frame = 1;

    return PINTA_OK;
}

u8 pinta_frame_stack_is_empty(PintaStackFrame *frame)
{
    pinta_assert(frame != NULL);

    return frame->stack < frame->stack_start;
}

PintaException pinta_stack_push(PintaThread *thread, PintaHeapObject *value)
{
    PintaException exception = PINTA_OK;
    PintaStackFrame *frame;
    PintaReference *top;

    pinta_assert(thread != NULL);
    pinta_assert(thread->frame != NULL);
    pinta_assert(thread->frame->stack != NULL);

    frame = thread->frame;
    top = &frame->stack[1];
    if (top >= thread->stack_end)
        PINTA_THROW(PINTA_EXCEPTION_STACK_OVERFLOW);

    frame->stack = top;
    top->reference = value;

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_frame_push(PintaThread *thread)
{
    PintaStackFrame *frame;
    PintaStackFrame *result;
    PintaReference *stack;

    pinta_assert(thread != NULL);
    pinta_assert(thread->frame != NULL);

    frame = thread->frame;
    result = (PintaStackFrame*)&frame->stack[1]; // stack always points to the last valid entry
    if (result >= (PintaStackFrame*)thread->stack_end)
        return PINTA_EXCEPTION_STACK_OVERFLOW;

    stack = (PintaReference*)&result[1];
    if (stack >= thread->stack_end)
        return PINTA_EXCEPTION_STACK_OVERFLOW;

    result->is_final_frame = 0;
    result->discard_result = 0;

    result->return_domain = NULL;

    result->function_this.reference = NULL;
    result->function_closure.reference = NULL;
    result->function_arguments.reference = NULL;
    result->function_locals.reference = NULL;

    result->return_address = NULL;

    result->stack_start = stack;
    result->stack = &stack[-1];

    result->prev = frame;
    thread->frame = result;

    return PINTA_OK;
}

PintaException pinta_lib_frame_pop(PintaThread *thread)
{
    PintaStackFrame *frame;
    PintaStackFrame *result;

    pinta_assert(thread != NULL);
    pinta_assert(thread->frame != NULL);

    frame = thread->frame;
    result = frame->prev;
    if (result == NULL)
        return PINTA_EXCEPTION_STACK_UNDERFLOW;

    frame->prev = NULL;
    thread->frame = result;

    return PINTA_OK;
}

u8 pinta_lib_stack_try_get_top(PintaThread *thread, PintaReference *result)
{
    PintaStackFrame *frame;
    PintaReference *top;

    pinta_assert(thread != NULL);
    pinta_assert(thread->frame != NULL);
    pinta_assert(thread->frame->stack != NULL);

    frame = thread->frame;
    if (frame->stack < frame->stack_start)
        return 0;

    top = frame->stack;
    if (result != NULL)
        result->reference = top->reference;

    return 1;
}

PintaException pinta_lib_stack_set_top(PintaThread *thread, PintaReference *value)
{
    PintaException exception = PINTA_OK;
    PintaStackFrame *frame;
    PintaReference *top;

    pinta_assert(thread != NULL);
    pinta_assert(thread->frame != NULL);
    pinta_assert(thread->frame->stack != NULL);
    pinta_assert(value != NULL);

    frame = thread->frame;
    if (frame->stack < frame->stack_start)
        PINTA_THROW(PINTA_EXCEPTION_STACK_UNDERFLOW);

    top = frame->stack;
    top->reference = value->reference;

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_stack_push(PintaThread *thread, PintaReference *value)
{
    PintaException exception = PINTA_OK;
    PintaStackFrame *frame;
    PintaReference *top;

    pinta_assert(thread != NULL);
    pinta_assert(thread->frame != NULL);
    pinta_assert(thread->frame->stack != NULL);
    pinta_assert(value != NULL);

    frame = thread->frame;
    top = &frame->stack[1];
    if (top >= thread->stack_end)
        PINTA_THROW(PINTA_EXCEPTION_STACK_OVERFLOW);

    frame->stack = top;
    top->reference = value->reference;

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_stack_push_null(PintaThread *thread)
{
    PintaException exception = PINTA_OK;
    PintaStackFrame *frame;
    PintaReference *top;

    pinta_assert(thread != NULL);
    pinta_assert(thread->frame != NULL);
    pinta_assert(thread->frame->stack != NULL);

    frame = thread->frame;
    top = &frame->stack[1];
    if (top >= thread->stack_end)
        PINTA_THROW(PINTA_EXCEPTION_STACK_OVERFLOW);

    frame->stack = top;
    top->reference = NULL;

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_stack_pop(PintaThread *thread, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    PintaStackFrame *frame;
    PintaReference *top;

    pinta_assert(thread != NULL);
    pinta_assert(thread->frame != NULL);

    frame = thread->frame;
    if (frame->stack < frame->stack_start)
        PINTA_THROW(PINTA_EXCEPTION_STACK_UNDERFLOW);

    top = frame->stack;
    frame->stack = &top[-1];

    if (result != NULL)
        result->reference = top->reference;

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_stack_duplicate(PintaThread *thread, u32 count)
{
    PintaException exception = PINTA_OK;
    PintaStackFrame *frame;
    PintaReference *start;
    PintaReference *top;
    PintaReference *end;
    u32 stack_length;
    u32 i = 0;

    pinta_assert(thread != NULL);
    pinta_assert(thread->frame != NULL);
    pinta_assert(thread->frame->stack != NULL);
    pinta_assert(count != PINTA_CODE_TOKEN_EMPTY);

    frame = thread->frame;
    if (count == 1)
    {
        start = frame->stack;
        top = &start[1];

        if (start < frame->stack_start)
            PINTA_THROW(PINTA_EXCEPTION_STACK_UNDERFLOW);

        if (top >= thread->stack_end)
            PINTA_THROW(PINTA_EXCEPTION_STACK_OVERFLOW);

        top->reference = start->reference;
        frame->stack = top;
        PINTA_RETURN();
    }

    end = &frame->stack[count];
    if (end >= thread->stack_end)
        PINTA_THROW(PINTA_EXCEPTION_STACK_OVERFLOW);

    if (frame->stack < frame->stack_start)
        stack_length = 0;
    else
        stack_length = frame->stack - frame->stack_start + 1;

    if (stack_length < count)
        PINTA_THROW(PINTA_EXCEPTION_STACK_UNDERFLOW);

    top = frame->stack;
    start = top - count + 1;

    top = &frame->stack[1];

    for (i = 0; i < count; i++)
    {
        top->reference = start->reference;
        top = &top[1];
        start = &start[1];
    }

    frame->stack = end;

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_stack_get_reference(PintaThread *thread, u32 offset, PintaReference **start)
{
    PintaException exception = PINTA_OK;
    PintaStackFrame *frame;
    PintaReference *top;
    u32 stack_length;

    pinta_assert(thread != NULL);
    pinta_assert(thread->frame != NULL);

    frame = thread->frame;
    if (frame->stack < frame->stack_start)
        stack_length = 0;
    else
        stack_length = frame->stack - frame->stack_start + 1;

    if (stack_length < offset)
        PINTA_THROW(PINTA_EXCEPTION_STACK_UNDERFLOW);

    top = frame->stack;
    top = top - offset + 1;

    if (start != NULL)
        *start = top;

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_stack_discard(PintaThread *thread, u32 length)
{
    PintaException exception = PINTA_OK;
    PintaStackFrame *frame;
    PintaReference *top;
    u32 stack_length;

    pinta_assert(thread != NULL);
    pinta_assert(thread->frame != NULL);

    frame = thread->frame;
    if (frame->stack < frame->stack_start)
        stack_length = 0;
    else
        stack_length = frame->stack - frame->stack_start + 1;

    if (stack_length < length)
        PINTA_THROW(PINTA_EXCEPTION_STACK_UNDERFLOW);

    top = frame->stack;
    frame->stack = top - length;

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}
