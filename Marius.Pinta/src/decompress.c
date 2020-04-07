#include "pinta.h"

void pinta_decompress_init(PintaDecompressor *decompressor, u8 *data, u32 length)
{
    decompressor->stack[0].data = data;
    decompressor->stack[0].length = length;
    decompressor->stack_start = &decompressor->stack[0];
    decompressor->stack_top = &decompressor->stack[0];
    decompressor->stack_end = &decompressor->stack[PINTA_DECOMPRESS_MAX_STACK - 1];
}

void pinta_decompress(u8 *data, u32 length, u8 *destination)
{
    PintaDecompressor decompressor_value, *decompressor = &decompressor_value;
    PintaDecompressState *state;
    u8 value;
    u32 offset;
    u8 *next;

    pinta_assert(data != NULL);
    pinta_assert(destination != NULL);

    pinta_decompress_init(decompressor, data, length);

    state = decompressor->stack_top;
    for(;;)
    {
        while (state->length == 0)
        {
            if (state == decompressor->stack_start)
                return;

            state--;
        }

        value = *state->data;
        state->data++;
        if (value == 0xFF)
        {
            value = (*state->data) | 0x80U;
            state->data++;
            state->length--;

            *destination = value;
            destination++;
        }
        else if (value & 0x80U)
        {
            if (state >= decompressor->stack_end)
                return;

            length = (value & 0x7FU) + 3;
            offset = *state->data;

            state->data++;

            if (length > state->length)
                length = state->length;
            state->length -= length;

            next = state->data - offset - 2;
            state++;
            state->data = next;
            state->length = length;
        }
        else
        {
            state->length--;

            *destination = value;
            destination++;
        }
    }
}
