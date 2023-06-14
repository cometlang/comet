#include <string.h>

#include "variables.h"
#include "compiler_defs.h"
#include "expressions.h"
#include "emitter.h"

#define GLOBAL_SCOPE 0
#define UNINITIALIZED_SCOPE -1
#define UNRESOLVED_VARIABLE_INDEX -1

static int addConstant(Parser *parser, Chunk *chunk, Value value)
{
    push(parser->compilation_thread, value);
    writeValueArray(&chunk->constants, value);
    pop(parser->compilation_thread);
    return chunk->constants.count - 1;
}

uint8_t makeConstant(Parser *parser, Value value)
{
    int constant = addConstant(parser, currentChunk(parser->currentFunction), value);
    if (constant > UINT8_MAX)
    {
        error(parser, "Too many constants in one chunk.");
        return 0;
    }

    return (uint8_t)constant;
}

uint8_t identifierConstant(Parser *parser, Token *name)
{
    return makeConstant(parser, copyString(parser->compilation_thread, name->start, name->length));
}

bool identifiersEqual(Token *a, Token *b)
{
    if (a->length != b->length)
        return false;
    return memcmp(a->start, b->start, a->length) == 0;
}

int resolveLocal(Parser *parser, Compiler *compiler, Token *name)
{
    for (int i = compiler->localCount - 1; i >= 0; i--)
    {
        Local *local = &compiler->locals[i];
        if (identifiersEqual(name, &local->name))
        {
            if (local->depth == UNINITIALIZED_SCOPE)
            {
                error(parser, "Cannot read local variable in its own initializer.");
            }
            return i;
        }
    }

    return UNRESOLVED_VARIABLE_INDEX;
}

static int addUpvalue(Compiler *compiler, Parser *parser, uint8_t index, bool isLocal)
{
    int upvalueCount = compiler->function->upvalueCount;

    for (int i = 0; i < upvalueCount; i++)
    {
        Upvalue *upvalue = &compiler->upvalues[i];
        if (upvalue->index == index && upvalue->isLocal == isLocal)
        {
            return i;
        }
    }

    if (upvalueCount == MAX_VAR_COUNT)
    {
        error(parser, "Too many closure variables in the function.");
        return 0;
    }

    compiler->upvalues[upvalueCount].isLocal = isLocal;
    compiler->upvalues[upvalueCount].index = index;
    return compiler->function->upvalueCount++;
}

int resolveUpvalue(Compiler *compiler, Parser *parser, Token *name)
{
    if (compiler->enclosing == NULL)
        return UNRESOLVED_VARIABLE_INDEX;

    int local = resolveLocal(parser, compiler->enclosing, name);
    if (local != UNRESOLVED_VARIABLE_INDEX)
    {
        compiler->enclosing->locals[local].isCaptured = true;
        return addUpvalue(compiler, parser, (uint8_t)local, true);
    }

    int upvalue = resolveUpvalue(compiler->enclosing, parser, name);
    if (upvalue != UNRESOLVED_VARIABLE_INDEX)
    {
        return addUpvalue(compiler, parser, (uint8_t)upvalue, false);
    }

    return UNRESOLVED_VARIABLE_INDEX;
}

int addLocal(Parser *parser, Token name)
{
    if (parser->currentFunction->localCount == MAX_VAR_COUNT)
    {
        error(parser, "Too many local variables in function.");
        return UNRESOLVED_VARIABLE_INDEX;
    }
    int localIndex = parser->currentFunction->localCount;
    Local *local = &parser->currentFunction->locals[localIndex];
    parser->currentFunction->localCount++;
    local->name = name;
    local->depth = UNINITIALIZED_SCOPE;
    local->isCaptured = false;
    return localIndex;
}

void declareVariable(Parser *parser)
{
    // Global variables are implicitly declared.
    if (parser->currentFunction->scopeDepth == GLOBAL_SCOPE)
        return;

    Token *name = &parser->previous;
    for (int i = parser->currentFunction->localCount - 1; i >= 0; i--)
    {
        Local *local = &parser->currentFunction->locals[i];
        if (local->depth != UNINITIALIZED_SCOPE && local->depth < parser->currentFunction->scopeDepth)
        {
            addLocal(parser, *name);
            return;
        }

        if (identifiersEqual(name, &local->name))
        {
            error(parser, "Variable with this name already declared in this scope.");
            return;
        }
    }
}

uint8_t parseVariable(Parser *parser, const char *errorMessage)
{
    consume(parser, TOKEN_IDENTIFIER, errorMessage);

    declareVariable(parser);
    if (parser->currentFunction->scopeDepth > GLOBAL_SCOPE)
        return 0;

    return identifierConstant(parser, &parser->previous);
}

void markInitialized(Parser *parser)
{
    if (parser->currentFunction->scopeDepth == GLOBAL_SCOPE)
        return;
    parser->currentFunction->locals[parser->currentFunction->localCount - 1].depth = parser->currentFunction->scopeDepth;
}

void defineVariable(Parser *parser, uint8_t global)
{
    if (parser->currentFunction->scopeDepth > GLOBAL_SCOPE)
    {
        markInitialized(parser);
        return;
    }
    emitBytes(parser, OP_DEFINE_GLOBAL, global);
}

void namedVariable(Parser *parser, Token name, bool canAssign)
{
    OpCode getOp, setOp;
    int arg = resolveLocal(parser, parser->currentFunction, &name);
    if (arg != UNRESOLVED_VARIABLE_INDEX)
    {
        getOp = OP_GET_LOCAL;
        setOp = OP_SET_LOCAL;
    }
    else if ((arg = resolveUpvalue(parser->currentFunction, parser, &name)) != UNRESOLVED_VARIABLE_INDEX)
    {
        getOp = OP_GET_UPVALUE;
        setOp = OP_SET_UPVALUE;
    }
    else
    {
        arg = identifierConstant(parser, &name);
        getOp = OP_GET_GLOBAL;
        setOp = OP_SET_GLOBAL;
    }

    if (canAssign)
    {
        if (match(parser, TOKEN_EQUAL))
        {
            expression(parser);
            emitBytes(parser, setOp, (uint8_t)arg);
            return;
        }
        else if (match(parser, TOKEN_PLUS_EQUAL))
        {
            emitBytes(parser, getOp, (uint8_t)arg);
            expression(parser);
            emitByte(parser, OP_ADD);
            emitBytes(parser, setOp, (uint8_t)arg);
            return;
        }
        else if (match(parser, TOKEN_STAR_EQUAL))
        {
            emitBytes(parser, getOp, (uint8_t)arg);
            expression(parser);
            emitByte(parser, OP_MULTIPLY);
            emitBytes(parser, setOp, (uint8_t)arg);
            return;
        }
        else if (match(parser, TOKEN_MINUS_EQUAL))
        {
            emitBytes(parser, getOp, (uint8_t)arg);
            expression(parser);
            emitByte(parser, OP_SUBTRACT);
            emitBytes(parser, setOp, (uint8_t)arg);
            return;
        }
        else if (match(parser, TOKEN_SLASH_EQUAL))
        {
            emitBytes(parser, getOp, (uint8_t)arg);
            expression(parser);
            emitByte(parser, OP_DIVIDE);
            emitBytes(parser, setOp, (uint8_t)arg);
            return;
        }
        else if (match(parser, TOKEN_PERCENT_EQUAL))
        {
            emitBytes(parser, getOp, (uint8_t)arg);
            expression(parser);
            emitByte(parser, OP_MODULO);
            emitBytes(parser, setOp, (uint8_t)arg);
            return;
        }
    }

    emitBytes(parser, getOp, (uint8_t)arg);
}

void variable(Parser *parser, bool canAssign)
{
    namedVariable(parser, parser->previous, canAssign);
}
