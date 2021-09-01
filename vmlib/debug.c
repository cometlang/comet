#include <stdio.h>
#include <string.h>

#include "debug.h"
#include "objects.h"
#include "value.h"

void disassembleChunk(Chunk *chunk, const char *name)
{
    printf("== %s:%s ==\n", chunk->filename, name);

    for (int offset = 0; offset < chunk->count;)
    {
        offset = disassembleInstruction(chunk, offset);
    }
}

static int invokeInstruction(const char *name, Chunk *chunk,
                             int offset)
{
    uint8_t constant = chunk->code[offset + 1];
    uint8_t argCount = chunk->code[offset + 2];
    printf("%-16s (%d args) %4d '", name, argCount, constant);
    printObject(chunk->constants.values[constant]);
    printf("'\n");
    return offset + 3;
}

static int simpleInstruction(const char *name, int offset)
{
    printf("%s\n", name);
    return offset + 1;
}

static int byteInstruction(const char *name, Chunk *chunk, int offset)
{
    uint8_t slot = chunk->code[offset + 1];
    printf("%-16s %4d\n", name, slot);
    return offset + 2;
}

static int jumpInstruction(const char *name, int sign, Chunk *chunk,
                           int offset)
{
    uint16_t jump = (uint16_t)(chunk->code[offset + 1] << 8);
    jump |= chunk->code[offset + 2];
    printf("%-16s %4d -> %d\n", name, offset, offset + 3 + sign * jump);
    return offset + 3;
}

static int constantInstruction(const char *name, Chunk *chunk, int offset)
{
    uint8_t constant = chunk->code[offset + 1];
    printf("%-16s %4d '", name, constant);
    printObject(chunk->constants.values[constant]);
    printf("'\n");
    return offset + 2;
}

static int exceptionHandlerInstruction(const char *name, Chunk *chunk, int offset)
{
    uint8_t type = chunk->code[offset + 1];
    uint16_t handlerAddress = (uint16_t)(chunk->code[offset + 2] << 8);
    handlerAddress |= chunk->code[offset + 3];
    uint16_t finallyAddress = (uint16_t)(chunk->code[offset + 4] << 8);
    finallyAddress |= chunk->code[offset + 5];
    printf("%-16s %4d -> %d, %d\n", name, type, handlerAddress, finallyAddress);
    return offset + 6;
}

int disassembleInstruction(Chunk *chunk, int offset)
{
    printf("%04d ", offset);
    if (offset > 0 && chunk->lines[offset] == chunk->lines[offset - 1])
    {
        printf("   | ");
    }
    else
    {
        printf("%4d ", chunk->lines[offset]);
    }

    uint8_t instruction = chunk->code[offset];
    switch (instruction)
    {
    case OP_CONSTANT:
        return constantInstruction("OP_CONSTANT", chunk, offset);
    case OP_NIL:
        return simpleInstruction("OP_NIL", offset);
    case OP_TRUE:
        return simpleInstruction("OP_TRUE", offset);
    case OP_FALSE:
        return simpleInstruction("OP_FALSE", offset);
    case OP_POP:
        return simpleInstruction("OP_POP", offset);
    case OP_GET_LOCAL:
        return byteInstruction("OP_GET_LOCAL", chunk, offset);
    case OP_SET_LOCAL:
        return byteInstruction("OP_SET_LOCAL", chunk, offset);
    case OP_GET_GLOBAL:
        return constantInstruction("OP_GET_GLOBAL", chunk, offset);
    case OP_DEFINE_GLOBAL:
        return constantInstruction("OP_DEFINE_GLOBAL", chunk, offset);
    case OP_SET_GLOBAL:
        return constantInstruction("OP_SET_GLOBAL", chunk, offset);
    case OP_GET_UPVALUE:
        return byteInstruction("OP_GET_UPVALUE", chunk, offset);
    case OP_SET_UPVALUE:
        return byteInstruction("OP_SET_UPVALUE", chunk, offset);
    case OP_GET_PROPERTY:
        return constantInstruction("OP_GET_PROPERTY", chunk, offset);
    case OP_SET_PROPERTY:
        return constantInstruction("OP_SET_PROPERTY", chunk, offset);
    case OP_GET_SUPER:
        return constantInstruction("OP_GET_SUPER", chunk, offset);
    case OP_EQUAL:
        return simpleInstruction("OP_EQUAL", offset);
    case OP_GREATER:
        return simpleInstruction("OP_GREATER", offset);
    case OP_LESS:
        return simpleInstruction("OP_LESS", offset);
    case OP_ADD:
        return simpleInstruction("OP_ADD", offset);
    case OP_SUBTRACT:
        return simpleInstruction("OP_SUBTRACT", offset);
    case OP_MULTIPLY:
        return simpleInstruction("OP_MULTIPLY", offset);
    case OP_DIVIDE:
        return simpleInstruction("OP_DIVIDE", offset);
    case OP_MODULO:
        return simpleInstruction("OP_MODULO", offset);
    case OP_NOT:
        return simpleInstruction("OP_NOT", offset);
    case OP_NEGATE:
        return simpleInstruction("OP_NEGATE", offset);
    case OP_BITWISE_OR:
        return simpleInstruction("OP_BITWISE_OR", offset);
    case OP_BITWISE_AND:
        return simpleInstruction("OP_BITWISE_AND", offset);
    case OP_BITWISE_XOR:
        return simpleInstruction("OP_BITWISE_XOR", offset);
    case OP_BITSHIFT_LEFT:
        return simpleInstruction("OP_BITSHIFT_LEFT", offset);
    case OP_BITSHIFT_RIGHT:
        return simpleInstruction("OP_BITSHIFT_RIGHT", offset);
    case OP_JUMP:
        return jumpInstruction("OP_JUMP", 1, chunk, offset);
    case OP_JUMP_IF_FALSE:
        return jumpInstruction("OP_JUMP_IF_FALSE", 1, chunk, offset);
    case OP_LOOP:
        return jumpInstruction("OP_LOOP", -1, chunk, offset);
    case OP_CALL:
        return byteInstruction("OP_CALL", chunk, offset);
    case OP_INVOKE:
        return invokeInstruction("OP_INVOKE", chunk, offset);
    case OP_SUPER:
        return invokeInstruction("OP_SUPER_", chunk, offset);
    case OP_CLOSURE:
    {
        offset++;
        uint8_t constant = chunk->code[offset++];
        printf("%-16s %4d ", "OP_CLOSURE", constant);
        printObject(chunk->constants.values[constant]);
        printf("\n");

        ObjFunction *function = AS_FUNCTION(
            chunk->constants.values[constant]);
        for (int j = 0; j < function->upvalueCount; j++)
        {
            int isLocal = chunk->code[offset++];
            int index = chunk->code[offset++];
            printf("%04d      |                     %s %d\n",
                   offset - 2, isLocal ? "local" : "upvalue", index);
        }

        return offset;
    }
    case OP_CLOSE_UPVALUE:
        return simpleInstruction("OP_CLOSE_UPVALUE", offset);
    case OP_RETURN:
        return simpleInstruction("OP_RETURN", offset);
    case OP_CLASS:
        return constantInstruction("OP_CLASS", chunk, offset);
    case OP_INHERIT:
        return simpleInstruction("OP_INHERIT", offset);
    case OP_METHOD:
        return constantInstruction("OP_METHOD", chunk, offset);
    case OP_INDEX:
        return byteInstruction("OP_INDEX", chunk, offset);
    case OP_INDEX_ASSIGN:
        return byteInstruction("OP_INDEX_ASSIGN", chunk, offset);
    case OP_DEFINE_OPERATOR:
        return constantInstruction("OP_DEFINE_OPERATOR", chunk, offset);
    case OP_THROW:
        return simpleInstruction("OP_THROW", offset);
    case OP_DUP_TOP:
        return simpleInstruction("OP_DUP_TOP", offset);
    case OP_INSTANCEOF:
        return simpleInstruction("OP_INSTANCEOF", offset);
    case OP_PUSH_EXCEPTION_HANDLER:
        return exceptionHandlerInstruction("OP_PUSH_EXCEPTION_HANDLER", chunk, offset);
    case OP_POP_EXCEPTION_HANDLER:
        return simpleInstruction("OP_POP_EXCEPTION_HANDLER", offset);
    case OP_PROPAGATE_EXCEPTION:
        return simpleInstruction("OP_PROPAGATE_EXCEPTION", offset);
    case OP_IMPORT:
        return simpleInstruction("OP_IMPORT", offset);
    default:
        printf("Unknown opcode %d\n", instruction);
        return offset + 1;
    }
}