#include <stdio.h>
#include "scanner.h"
#include "vm.h"
#include "compiler_defs.h"
#include "declarations.h"
#include "emitter.h"
#include "expressions.h"
#include "statements.h"
#include "import.h"
#include "variables.h"

void block(Parser *parser)
{
    match(parser, TOKEN_EOL);
    while (!check(parser, TOKEN_RIGHT_BRACE) && !check(parser, TOKEN_EOF))
    {
        declaration(parser);
    }

    match(parser, TOKEN_EOL); // optional end of line.
    consume(parser, TOKEN_RIGHT_BRACE, "Expect '}' after block.");
}

void expressionStatement(Parser *parser)
{
    expression(parser);
    if (!check(parser, TOKEN_EOF) && !check(parser, TOKEN_RIGHT_BRACE))
    {
        consume(parser, TOKEN_EOL, "Only one statement per line allowed");
    }
    emitByte(parser, OP_POP);
}

void forStatement(Parser *parser)
{
    beginScope(parser);
    consume(parser, TOKEN_LEFT_PAREN, "Expect '(' after 'for'.");
    if (check(parser, TOKEN_SEMI_COLON))
    {
        // No initializer.
    }
    else if (match(parser, TOKEN_VAR))
    {
        varDeclaration(parser);
    }
    else
    {
        expressionStatement(parser);
    }
    consume(parser, TOKEN_SEMI_COLON, "Expect ';' after loop intializer.");
    LoopCompiler loop = {0};
    loop.startAddress = getCurrentOffset(parser->currentFunction);
    loop.exitAddress = UNINITALISED_ADDRESS;
    loop.enclosing = parser->currentLoop;
    loop.loopScopeDepth = parser->currentFunction->scopeDepth;
    loop.breakJump = UNINITALISED_ADDRESS;
    parser->currentLoop = &loop;

    if (!match(parser, TOKEN_SEMI_COLON))
    {
        expression(parser);
        consume(parser, TOKEN_SEMI_COLON, "Expect ';' after loop condition.");

        // Jump out of the loop if the condition is false.
        loop.exitAddress = emitJump(parser, OP_JUMP_IF_FALSE);
        emitByte(parser, OP_POP); // Condition.
    }
    if (!match(parser, TOKEN_RIGHT_PAREN))
    {
        int bodyJump = emitJump(parser, OP_JUMP);

        int incrementStart = getCurrentOffset(parser->currentFunction);
        expression(parser);
        emitByte(parser, OP_POP);
        consume(parser, TOKEN_RIGHT_PAREN, "Expect ')' after for clauses.");

        emitLoop(parser);
        loop.startAddress = incrementStart;
        patchJump(parser, bodyJump);
    }

    statement(parser);

    emitLoop(parser);

    if (loop.breakJump != UNINITALISED_ADDRESS)
    {
        patchJump(parser, loop.breakJump);
    }
    if (loop.exitAddress != UNINITALISED_ADDRESS)
    {
        patchJump(parser, loop.exitAddress);
        emitByte(parser, OP_POP); // Condition.
    }

    endScope(parser);
    parser->currentLoop = parser->currentLoop->enclosing;
}

static void syntheticMethodCall(Parser *parser, const char *method_name)
{
    Token methodNameToken = syntheticToken(method_name);
    uint8_t constant = identifierConstant(parser, &methodNameToken);
    emitBytes(parser, OP_INVOKE, constant);
    emitByte(parser, 0); // zero arguments
}

void foreachStatement(Parser *parser)
{
    beginScope(parser);
    consume(parser, TOKEN_LEFT_PAREN, "Expect '(' after 'foreach'.");
    consume(parser, TOKEN_VAR, "Expect 'var' to declare foreach loop variable");
    Token loop_var_name = parser->current;
    uint8_t loop_var = parseVariable(parser, "Expect a variable name in the foreach loop");
    emitByte(parser, OP_NIL);
    defineVariable(parser, loop_var);

    consume(parser, TOKEN_IN, "Expect 'in' keyword in foreach loop");
    expression(parser);
    syntheticMethodCall(parser, "iterator");
    consume(parser, TOKEN_RIGHT_PAREN, "Expect ')' after 'foreach' condition.");

    Token iter_var_name = syntheticToken("");
    int iter_var = addLocal(parser, iter_var_name);
    emitBytes(parser, OP_SET_LOCAL, (uint8_t) iter_var);
    markInitialized(parser);

    LoopCompiler loop = {0};
    loop.startAddress = getCurrentOffset(parser->currentFunction);
    loop.exitAddress = UNINITALISED_ADDRESS;
    loop.breakJump = UNINITALISED_ADDRESS;
    loop.enclosing = parser->currentLoop;
    loop.loopScopeDepth = parser->currentFunction->scopeDepth;
    parser->currentLoop = &loop;

    emitBytes(parser, OP_GET_LOCAL, iter_var);
    syntheticMethodCall(parser, "has_next?");
    loop.exitAddress = emitJump(parser, OP_JUMP_IF_FALSE);
    emitByte(parser, OP_POP);

    emitBytes(parser, OP_GET_LOCAL, iter_var);
    syntheticMethodCall(parser, "get_next");
    int variable = resolveLocal(parser, parser->currentFunction, &loop_var_name);
    emitBytes(parser, OP_SET_LOCAL, (uint8_t) variable);
    emitByte(parser, OP_POP);

    statement(parser);

    emitLoop(parser);
    patchJump(parser, loop.exitAddress);
    if (loop.breakJump != UNINITALISED_ADDRESS)
    {
        patchJump(parser, loop.breakJump);
    }
    endScope(parser);
    emitByte(parser, OP_POP); // This feels weird, like I shouldn't need to do it.
    parser->currentLoop = parser->currentLoop->enclosing;
}

void ifStatement(Parser *parser)
{
    consume(parser, TOKEN_LEFT_PAREN, "Expect '(' after 'if'.");
    expression(parser);
    consume(parser, TOKEN_RIGHT_PAREN, "Expect ')' after condition.");

    int thenJump = emitJump(parser, OP_JUMP_IF_FALSE);
    emitByte(parser, OP_POP);
    statement(parser);

    int elseJump = emitJump(parser, OP_JUMP);
    patchJump(parser, thenJump);
    emitByte(parser, OP_POP);

    match(parser, TOKEN_EOL);
    if (match(parser, TOKEN_ELSE))
        statement(parser);
    patchJump(parser, elseJump);
}

void returnStatement(Parser *parser)
{
    if (parser->currentFunction->type == TYPE_SCRIPT)
    {
        error(parser, "Cannot return from top-level code.");
    }
    if (match(parser, TOKEN_EOL))
    {
        emitReturn(parser);
    }
    else
    {
        if (parser->currentFunction->type == TYPE_INITIALIZER)
        {
            error(parser, "Cannot return a value from an initializer.");
        }
        expression(parser);
        if (!check(parser, TOKEN_RIGHT_BRACE))
        {
            consume(parser, TOKEN_EOL, "Only one statement per line allowed");
        }
        emitByte(parser, OP_RETURN);
    }
}

void whileStatement(Parser *parser)
{
    LoopCompiler loop = {0};
    loop.startAddress = getCurrentOffset(parser->currentFunction);
    loop.exitAddress = UNINITALISED_ADDRESS;
    loop.enclosing = parser->currentLoop;
    loop.breakJump = UNINITALISED_ADDRESS;
    loop.loopScopeDepth = parser->currentFunction->scopeDepth;
    parser->currentLoop = &loop;
    consume(parser, TOKEN_LEFT_PAREN, "Expect '(' after 'while'.");
    expression(parser);
    consume(parser, TOKEN_RIGHT_PAREN, "Expect ')' after condition.");

    loop.exitAddress = emitJump(parser, OP_JUMP_IF_FALSE);

    emitByte(parser, OP_POP);
    statement(parser);

    emitLoop(parser);

    patchJump(parser, loop.exitAddress);
    if (loop.breakJump != UNINITALISED_ADDRESS)
    {
        patchJump(parser, loop.breakJump);
    }
    emitByte(parser, OP_POP);
    parser->currentLoop = parser->currentLoop->enclosing;
}

static void patchAddress(Parser *parser, int offset)
{
    int currentOffset = getCurrentOffset(parser->currentFunction);
    setCodeOffset(parser->currentFunction, offset, (currentOffset >> 8) & 0xff);
    setCodeOffset(parser->currentFunction, offset + 1, currentOffset & 0xff);
}

void tryStatement(Parser *parser)
{
    emitByte(parser, OP_PUSH_EXCEPTION_HANDLER);
    int exceptionType = getCurrentOffset(parser->currentFunction);
    emitByte(parser, 0xff);
    int handlerAddress = getCurrentOffset(parser->currentFunction);
    emitBytes(parser, 0xff, 0xff);
    int finallyAddress = getCurrentOffset(parser->currentFunction);
    emitBytes(parser, 0xff, 0xff);

    statement(parser);

    emitByte(parser, OP_POP_EXCEPTION_HANDLER);
    int successJump = emitJump(parser, OP_JUMP);
    match(parser, TOKEN_EOL);

    if (match(parser, TOKEN_CATCH))
    {
        beginScope(parser);
        consume(parser, TOKEN_LEFT_PAREN, "Expect '(' after catch");
        consume(parser, TOKEN_IDENTIFIER, "Expect type name to catch");
        uint8_t name = identifierConstant(parser, &parser->previous);
        setCodeOffset(parser->currentFunction, exceptionType, name);
        patchAddress(parser, handlerAddress);
        if (match(parser, TOKEN_AS))
        {
            consume(parser, TOKEN_IDENTIFIER, "Expect identifier for exception instance");
            uint8_t ex_var = addLocal(parser, parser->previous);
            markInitialized(parser);
            emitBytes(parser, OP_SET_LOCAL, ex_var);
        }
        consume(parser, TOKEN_RIGHT_PAREN, "Expect ')' after catch statement");
        emitByte(parser, OP_POP_EXCEPTION_HANDLER);
        statement(parser);
        match(parser, TOKEN_EOL);
        endScope(parser);
    }
    patchJump(parser, successJump);

    if (match(parser, TOKEN_FINALLY))
    {
        // If we arrive here from either the try or handler blocks, then we don't
        // want to continue propagating the exception
        emitByte(parser, OP_FALSE);

        patchAddress(parser, finallyAddress);
        statement(parser);

        int continueExecution = emitJump(parser, OP_JUMP_IF_FALSE);
        emitByte(parser, OP_POP); // Pop the bool off the stack
        emitByte(parser, OP_PROPAGATE_EXCEPTION);
        patchJump(parser, continueExecution);
        emitByte(parser, OP_POP);
    }
}

void throwStatement(Parser *parser)
{
    expression(parser);
    if (!check(parser, TOKEN_EOF))
        consume(parser, TOKEN_EOL, "Only one statement per line allowed");
    emitByte(parser, OP_THROW);
}

void rethrowStatement(Parser *parser)
{
    expression(parser);
    if (!check(parser, TOKEN_EOF))
        consume(parser, TOKEN_EOL, "Only one statement per line allowed");
    emitByte(parser, OP_PROPAGATE_EXCEPTION);
}

void importStatement(Parser *parser)
{
    expression(parser);
    consume(parser, TOKEN_AS, "Expected 'as' after the module to import");
    emitByte(parser, OP_IMPORT);
    // Imports are a function that return NIL, so ditch the nil from the stack
    emitByte(parser, OP_POP);
    uint8_t global = parseVariable(parser, "Expected a variable name for the imported module.");
    defineVariable(parser, global);
}

void nextStatement(Parser *parser)
{
    if (parser->currentLoop == NULL) {
        error(parser, "Can't use 'next' outside of a loop.");
        return;
    }

    // Discard any locals created inside the loop.
    for (int i = parser->currentFunction->localCount - 1;
        i >= 0 && parser->currentFunction->locals[i].depth > parser->currentLoop->loopScopeDepth;
        i--) {
        emitByte(parser, OP_POP);
    }

    // Jump to top of current innermost loop.
    emitLoop(parser);
}

static void breakStatement(Parser *parser)
{
    if (parser->currentLoop == NULL) {
        error(parser, "Can't use 'break' outside of a loop.");
        return;
    }

    if (parser->currentLoop->breakJump != UNINITALISED_ADDRESS)
    {
        error(parser, "Only one break statement per loop is supported.");
        return;
    }

    parser->currentLoop->breakJump = emitJump(parser, OP_JUMP);
}

void statement(Parser *parser)
{
    match(parser, TOKEN_EOL);
    if (match(parser, TOKEN_FOR))
    {
        forStatement(parser);
    }
    else if (match(parser, TOKEN_FOREACH))
    {
        foreachStatement(parser);
    }
    else if (match(parser, TOKEN_IF))
    {
        ifStatement(parser);
    }
    else if (match(parser, TOKEN_RETURN))
    {
        returnStatement(parser);
    }
    else if (match(parser, TOKEN_WHILE))
    {
        whileStatement(parser);
    }
    else if (match(parser, TOKEN_TRY))
    {
        tryStatement(parser);
    }
    else if (match(parser, TOKEN_RETHROW))
    {
        rethrowStatement(parser);
    }
    else if (match(parser, TOKEN_THROW))
    {
        throwStatement(parser);
    }
    else if (match(parser, TOKEN_LEFT_BRACE))
    {
        beginScope(parser);
        block(parser);
        endScope(parser);
    }
    else if (match(parser, TOKEN_IMPORT))
    {
        importStatement(parser);
    }
    else if (match(parser, TOKEN_NEXT))
    {
        nextStatement(parser);
    }
    else if (match(parser, TOKEN_BREAK)) {
        breakStatement(parser);
    }
    else
    {
        expressionStatement(parser);
    }
}
