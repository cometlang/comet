#include <string.h>

#include "variables.h"
#include "compiler_defs.h"
#include "expressions.h"
#include "emitter.h"

static int addConstant(Chunk *chunk, Value value)
{
    push(main_thread, value);
    writeValueArray(&chunk->constants, value);
    pop(main_thread);
    return chunk->constants.count - 1;
}

uint8_t makeConstant(Parser *parser, Value value)
{
    int constant = addConstant(currentChunk(current), value);
    if (constant > UINT8_MAX)
    {
        error(parser, "Too many constants in one chunk.");
        return 0;
    }

    return (uint8_t)constant;
}

uint8_t identifierConstant(Parser *parser, Token *name)
{
    return makeConstant(parser, copyString(main_thread, name->start, name->length));
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
            if (local->depth == UNINITIALIZED_LOCAL_SCOPE)
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
        error(parser, "Too many closure variables in function.");
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

void addLocal(Parser *parser, Token name)
{
    if (current->localCount == MAX_VAR_COUNT)
    {
        error(parser, "Too many local variables in function.");
        return;
    }
    Local *local = &current->locals[current->localCount++];
    local->name = name;
    local->depth = UNINITIALIZED_LOCAL_SCOPE;
    local->isCaptured = false;
}

int resolveModuleVariable(Compiler UNUSED(*compiler), Parser UNUSED(*parser), Token UNUSED(*name))
{
    return UNRESOLVED_VARIABLE_INDEX;
}

void declareVariable(Parser *parser)
{
    // Global variables are implicitly declared.
    if (current->scopeDepth == GLOBAL_SCOPE)
        return;

    Token *name = &parser->previous;
    for (int i = current->localCount - 1; i >= 0; i--)
    {
        Local *local = &current->locals[i];
        if (local->depth != UNINITIALIZED_LOCAL_SCOPE && local->depth < current->scopeDepth)
        {
            break;
        }

        if (identifiersEqual(name, &local->name))
        {
            error(parser, "Variable with this name already declared in this scope.");
        }
    }
    addLocal(parser, *name);
}

uint8_t parseVariable(Parser *parser, const char *errorMessage)
{
    consume(parser, TOKEN_IDENTIFIER, errorMessage);

    declareVariable(parser);
    if (current->scopeDepth > GLOBAL_SCOPE)
        return 0;

    return identifierConstant(parser, &parser->previous);
}

void markInitialized()
{
    if (current->scopeDepth == GLOBAL_SCOPE)
        return;
    current->locals[current->localCount - 1].depth = current->scopeDepth;
}

void defineVariable(Parser *parser, uint8_t global)
{
    if (current->scopeDepth > GLOBAL_SCOPE)
    {
        markInitialized();
        return;
    }
    emitBytes(parser, OP_DEFINE_GLOBAL, global);
}

void namedVariable(Parser *parser, Token name, bool canAssign)
{
    uint8_t getOp, setOp;
    int arg = resolveLocal(parser, current, &name);
    if (arg != UNRESOLVED_VARIABLE_INDEX)
    {
        getOp = OP_GET_LOCAL;
        setOp = OP_SET_LOCAL;
    }
    else if ((arg = resolveUpvalue(current, parser, &name)) != UNRESOLVED_VARIABLE_INDEX)
    {
        getOp = OP_GET_UPVALUE;
        setOp = OP_SET_UPVALUE;
    }
    else if ((arg = resolveModuleVariable(current, parser, &name)) != UNRESOLVED_VARIABLE_INDEX)
    {
        getOp = OP_GET_MODULE_VAR;
        setOp = OP_SET_MODULE_VAR;
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
    }

    emitBytes(parser, getOp, (uint8_t)arg);
}

void variable(Parser *parser, bool canAssign)
{
    namedVariable(parser, parser->previous, canAssign);
}
