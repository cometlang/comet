#include "emitter.h"
#include "variables.h"

void emitByte(Parser *parser, uint8_t byte)
{
    writeChunk(currentChunk(current), byte, parser->previous.line);
}

void emitBytes(Parser *parser, uint8_t byte1, uint8_t byte2)
{
    emitByte(parser, byte1);
    emitByte(parser, byte2);
}

void emitLoop(Parser *parser)
{
    emitByte(parser, OP_LOOP);

    int offset = currentChunk(current)->count - parser->currentLoop->startAddress + 2;
    if (offset > UINT16_MAX)
        error(parser, "Loop body too large.");

    emitByte(parser, (offset >> 8) & 0xff);
    emitByte(parser, offset & 0xff);
}

int emitJump(Parser *parser, uint8_t instruction)
{
    emitByte(parser, instruction);
    emitByte(parser, 0xff);
    emitByte(parser, 0xff);
    return currentChunk(current)->count - 2;
}

void emitReturn(Parser *parser)
{
    // An initializer automatically returns "self".
    if (current->type == TYPE_INITIALIZER)
    {
        emitBytes(parser, OP_GET_LOCAL, 0);
    }
    else
    {
        emitByte(parser, OP_NIL);
    }
    emitByte(parser, OP_RETURN);
}

void emitConstant(Parser *parser, Value value)
{
    emitBytes(parser, OP_CONSTANT, makeConstant(parser, value));
}
