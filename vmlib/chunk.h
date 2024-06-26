#ifndef clox_chunk_h
#define clox_chunk_h

#include "common.h"
#include "value.h"

typedef enum
{
    OP_CONSTANT,
    OP_NIL,
    OP_TRUE,
    OP_FALSE,
    OP_POP,
    OP_GET_LOCAL,
    OP_SET_LOCAL,
    OP_GET_GLOBAL,
    OP_DEFINE_GLOBAL,
    OP_SET_GLOBAL,
    OP_GET_UPVALUE,
    OP_SET_UPVALUE,
    OP_GET_PROPERTY,
    OP_SET_PROPERTY,
    OP_GET_SUPER,
    OP_EQUAL,
    OP_GREATER,
    OP_GREATER_EQUAL,
    OP_LESS,
    OP_LESS_EQUAL,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_MODULO,
    OP_NOT,
    OP_NEGATE,
    OP_JUMP,
    OP_JUMP_IF_FALSE,
    OP_LOOP,
    OP_CALL,
    OP_INVOKE,
    OP_SUPER,
    OP_CLOSURE,
    OP_CLOSE_UPVALUE,
    OP_RETURN,
    OP_CLASS,
    OP_INHERIT,
    OP_METHOD,
    OP_STATIC_METHOD,
    OP_INDEX,
    OP_INDEX_ASSIGN,
    OP_DEFINE_OPERATOR,
    OP_THROW,
    OP_RETHROW,
    OP_DUP_TOP,
    OP_DUP_TWO,
    OP_IS,
    OP_PUSH_EXCEPTION_HANDLER,
    OP_POP_EXCEPTION_HANDLER,
    OP_PROPAGATE_EXCEPTION,
    OP_IMPORT,
    OP_IMPORT_PARAMS,
    OP_BITWISE_OR,
    OP_BITWISE_AND,
    OP_BITWISE_XOR,
    OP_BITSHIFT_LEFT,
    OP_BITSHIFT_RIGHT,
    OP_SPLAT,
} OpCode;

typedef struct
{
    int count;
    int capacity;
    ValueArray constants;
    int *lines;
    uint16_t *execution_counts;
    uint8_t *code;
    char *filename;
} Chunk;

void initChunk(Chunk *chunk, const char *filename);

void writeChunk(Chunk *chunk, uint8_t byte, int line);

void freeChunk(Chunk *chunk);

void print_constants(Chunk *chunk);

void recordInstructionExecuted(Chunk *chunk, size_t instruction);

#endif
