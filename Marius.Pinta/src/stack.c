#include "pinta.h"

PintaStackFrame *pinta_frame_init(PintaNativeMemory *memory, u32 length_in_bytes)
{
    u8 *stack_start, *stack_end;
    PintaStackFrame *frame;

    if (memory == NULL)
        return NULL;

    if (length_in_bytes < sizeof(PintaStackFrame))
        return NULL;

    frame = (PintaStackFrame*)pinta_memory_alloc(memory, sizeof(PintaStackFrame));

    stack_start = (u8*)pinta_memory_alloc(memory, length_in_bytes);
    stack_end = &stack_start[length_in_bytes];

    frame->return_address = NULL;
    frame->stack_start = (PintaReference*)stack_start;
    frame->stack = &frame->stack_start[-1];
    frame->stack_end = (PintaReference*)stack_end;
    frame->function_this.reference = NULL;
    frame->function_closure.reference = NULL;
    frame->function_arguments.reference = NULL;
    frame->function_locals.reference = NULL;
    frame->next = frame->prev = NULL;
    frame->is_final_frame = 0;
    frame->discard_result = 0;
    frame->transform_result = NULL;
    frame->transform_tag = 0;
    frame->return_domain = NULL;

    return frame;
}

PintaStackFrame *pinta_frame_push(PintaStackFrame *frame)
{
    PintaStackFrame *result;
    PintaReference *stack;

    pinta_assert(frame != NULL);

    result = (PintaStackFrame*)&frame->stack[1]; // stack always points to the last valid entry
    if (result >= (PintaStackFrame*)frame->stack_end)
        return NULL;

    stack = (PintaReference*)&result[1];
    if (stack >= frame->stack_end)
        return NULL;

    result->is_final_frame = 0;
    result->discard_result = 0;
    result->transform_result = NULL;
    result->transform_tag = 0;

    result->return_domain = NULL;

    result->function_this.reference = NULL;
    result->function_closure.reference = NULL;
    result->function_arguments.reference = NULL;
    result->function_locals.reference = NULL;

    result->return_address = NULL;

    result->stack_start = stack;
    result->stack = &stack[-1];
    result->stack_end = frame->stack_end;

    frame->next = result;
    result->prev = frame;
    result->next = NULL;

    return result;
}

PintaStackFrame *pinta_frame_pop(PintaStackFrame *frame)
{
    PintaStackFrame *result;

    pinta_assert(frame != NULL);

    result = frame->prev;
    if (result == NULL)
        return result;

    frame->prev = NULL;
    result->next = NULL;
    return result;
}

PintaException pinta_frame_stack_push(PintaStackFrame *frame, PintaHeapObject *value)
{
    PintaException exception = PINTA_OK;
    PintaReference *top;

    pinta_assert(frame != NULL);
    pinta_assert(frame->stack != NULL);

    top = &frame->stack[1];
    if (top >= frame->stack_end)
        PINTA_THROW(PINTA_EXCEPTION_STACK_OVERFLOW);

    frame->stack = top;
    top->reference = value;

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_frame_stack_push_null(PintaStackFrame *frame)
{
    PintaException exception = PINTA_OK;
    PintaReference *top;

    pinta_assert(frame != NULL);
    pinta_assert(frame->stack != NULL);

    top = &frame->stack[1];
    if (top >= frame->stack_end)
        PINTA_THROW(PINTA_EXCEPTION_STACK_OVERFLOW);

    frame->stack = top;
    top->reference = NULL;

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_frame_stack_pop(PintaStackFrame *frame, PintaReference *result)
{
    PintaException exception = PINTA_OK;
    PintaReference *top;

    pinta_assert(frame != NULL);

    if (frame->stack < frame->stack_start)
        PINTA_THROW(PINTA_EXCEPTION_STACK_UNDERFLOW);

    top = frame->stack;
    frame->stack = &top[-1];

    if (result != NULL)
        result->reference = top->reference;

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_frame_stack_duplicate(PintaStackFrame *frame, u32 count)
{
    PintaException exception = PINTA_OK;
    PintaReference *start;
    PintaReference *top;
    PintaReference *end;
    u32 i = 0;

    pinta_assert(frame != NULL);
    pinta_assert(frame->stack != NULL);
    pinta_assert(count != PINTA_CODE_TOKEN_EMPTY);

    if (count == 1)
    {
        start = frame->stack;
        top = &start[1];

        if (start < frame->stack_start)
            PINTA_THROW(PINTA_EXCEPTION_STACK_UNDERFLOW);

        if (top >= frame->stack_end)
            PINTA_THROW(PINTA_EXCEPTION_STACK_OVERFLOW);

        top->reference = start->reference;
        frame->stack = top;
        PINTA_RETURN();
    }

    end = &frame->stack[count];
    if (end >= frame->stack_end)
        PINTA_THROW(PINTA_EXCEPTION_STACK_OVERFLOW);

    PINTA_CHECK(pinta_frame_stack_get_reference(frame, count, &start));

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

PintaException pinta_frame_stack_get_reference(PintaStackFrame *frame, u32 offset, PintaReference **start)
{
    PintaException exception = PINTA_OK;
    PintaReference *top;
    u32 stack_length;

    pinta_assert(frame != NULL);

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

PintaException pinta_frame_stack_discard(PintaStackFrame *frame, u32 length)
{
    PintaException exception = PINTA_OK;
    PintaReference *top;
    u32 stack_length;

    pinta_assert(frame != NULL);

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

PintaException pinta_stack_push(PintaThread *thread, PintaHeapObject *value)
{
    pinta_assert(thread != NULL);

    return pinta_frame_stack_push(thread->frame, value);
}

PintaException pinta_lib_frame_push(PintaThread *thread)
{
    PintaException exception = PINTA_OK;
    PintaStackFrame *frame;

    pinta_assert(thread != NULL);
    pinta_assert(thread->frame != NULL);

    frame = pinta_frame_push(thread->frame);
    if (frame == NULL)
        PINTA_THROW(PINTA_EXCEPTION_STACK_OVERFLOW);

    thread->frame = frame;

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_frame_pop(PintaThread *thread)
{
    PintaException exception = PINTA_OK;
    PintaStackFrame *frame;

    pinta_assert(thread != NULL);
    pinta_assert(thread->frame != NULL);

    frame = pinta_frame_pop(thread->frame);
    if (frame == NULL)
        PINTA_THROW(PINTA_EXCEPTION_STACK_UNDERFLOW);

    thread->frame = frame;

PINTA_EXIT:
    return PINTA_EXCEPTION(exception);
}

PintaException pinta_lib_stack_push(PintaThread *thread, PintaReference *value)
{
    pinta_assert(thread != NULL);
    pinta_assert(value != NULL);

    return pinta_frame_stack_push(thread->frame, value->reference);
}

PintaException pinta_lib_stack_push_null(PintaThread *thread)
{
    pinta_assert(thread != NULL);

    return pinta_frame_stack_push_null(thread->frame);
}

PintaException pinta_lib_stack_pop(PintaThread *thread, PintaReference *result)
{
    pinta_assert(thread != NULL);

    return pinta_frame_stack_pop(thread->frame, result);
}

PintaException pinta_lib_stack_duplicate(PintaThread *thread, u32 count)
{
    pinta_assert(thread != NULL);
    pinta_assert(count != PINTA_CODE_TOKEN_EMPTY);

    return pinta_frame_stack_duplicate(thread->frame, count);
}

PintaException pinta_lib_stack_get_reference(PintaThread *thread, u32 offset, PintaReference **start)
{
    pinta_assert(thread != NULL);

    return pinta_frame_stack_get_reference(thread->frame, offset, start);
}

PintaException pinta_lib_stack_discard(PintaThread *thread, u32 length)
{
    pinta_assert(thread != NULL);

    return pinta_frame_stack_discard(thread->frame, length);
}
