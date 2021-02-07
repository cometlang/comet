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
    if (!check(parser, TOKEN_EOF))
    {
        consume(parser, TOKEN_EOL, "Only one statement per line allowed");
    }
    emitByte(parser, OP_POP);
}

void forStatement(Parser *parser)
{
    beginScope();
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
    LoopCompiler loop;
    loop.startAddress = currentChunk(current)->count;
    loop.exitAddress = -1;
    loop.enclosing = parser->currentLoop;
    loop.loopScopeDepth = current->scopeDepth;
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

        int incrementStart = currentChunk(current)->count;
        expression(parser);
        emitByte(parser, OP_POP);
        consume(parser, TOKEN_RIGHT_PAREN, "Expect ')' after for clauses.");

        emitLoop(parser);
        loop.startAddress = incrementStart;
        patchJump(parser, bodyJump);
    }

    statement(parser);

    emitLoop(parser);

    if (loop.exitAddress != -1)
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
    emitByte(parser, 0);
}

void foreachStatement(Parser *parser)
{
    beginScope();
    consume(parser, TOKEN_LEFT_PAREN, "Expect '(' after 'foreach'.");
    consume(parser, TOKEN_VAR, "Expect 'var' to declare foreach loop variable");
    Token loop_var_name = parser->current;
    uint8_t loop_var = parseVariable(parser, "Expect a variable name in the foreach loop");
    emitByte(parser, OP_NIL);
    defineVariable(parser, loop_var);

    consume(parser, TOKEN_IN, "Expect 'in' keyword in foreach loop");

    expression(parser);
    consume(parser, TOKEN_RIGHT_PAREN, "Expect ')' after 'foreach' condition.");

    Token iter_var_name = syntheticToken("");
    addLocal(parser, iter_var_name);
    markInitialized();
    uint8_t ex_var = resolveLocal(parser, current, &iter_var_name);

    syntheticMethodCall(parser, "iterator");
    defineVariable(parser, ex_var);

    LoopCompiler loop;
    loop.startAddress = currentChunk(current)->count;
    loop.exitAddress = -1;
    loop.enclosing = parser->currentLoop;
    loop.loopScopeDepth = current->scopeDepth;
    parser->currentLoop = &loop;

    emitBytes(parser, OP_GET_LOCAL, ex_var);
    syntheticMethodCall(parser, "has_next?");
    loop.exitAddress = emitJump(parser, OP_JUMP_IF_FALSE);
    emitByte(parser, OP_POP);

    emitBytes(parser, OP_GET_LOCAL, ex_var);
    syntheticMethodCall(parser, "get_next");
    int variable = resolveLocal(parser, current, &loop_var_name);
    emitBytes(parser, OP_SET_LOCAL, (uint8_t) variable);

    statement(parser);
    emitByte(parser, OP_POP);

    emitLoop(parser);

    patchJump(parser, loop.exitAddress);
    endScope(parser);
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

    if (match(parser, TOKEN_ELSE))
        statement(parser);
    patchJump(parser, elseJump);
}

void returnStatement(Parser *parser)
{
    if (current->type == TYPE_SCRIPT)
    {
        error(parser, "Cannot return from top-level code.");
    }
    if (match(parser, TOKEN_EOL))
    {
        emitReturn(parser);
    }
    else
    {
        if (current->type == TYPE_INITIALIZER)
        {
            error(parser, "Cannot return a value from an initializer.");
        }
        expression(parser);
        consume(parser, TOKEN_EOL, "Only one statement per line allowed");
        emitByte(parser, OP_RETURN);
    }
}

void whileStatement(Parser *parser)
{
    LoopCompiler loop;
    loop.startAddress = currentChunk(current)->count;
    loop.exitAddress = -1;
    loop.enclosing = parser->currentLoop;
    loop.loopScopeDepth = current->scopeDepth;
    parser->currentLoop = &loop;
    consume(parser, TOKEN_LEFT_PAREN, "Expect '(' after 'while'.");
    expression(parser);
    consume(parser, TOKEN_RIGHT_PAREN, "Expect ')' after condition.");

    loop.exitAddress = emitJump(parser, OP_JUMP_IF_FALSE);

    emitByte(parser, OP_POP);
    statement(parser);

    emitLoop(parser);

    patchJump(parser, loop.exitAddress);
    emitByte(parser, OP_POP);
    parser->currentLoop = parser->currentLoop->enclosing;
}

static void patchAddress(int offset)
{
    currentChunk(current)->code[offset] = (currentChunk(current)->count >> 8) & 0xff;
    currentChunk(current)->code[offset + 1] = currentChunk(current)->count & 0xff;
}

void tryStatement(Parser *parser)
{
    emitByte(parser, OP_PUSH_EXCEPTION_HANDLER);
    int exceptionType = currentChunk(current)->count;
    emitByte(parser, 0xff);
    int handlerAddress = currentChunk(current)->count;
    emitBytes(parser, 0xff, 0xff);
    int finallyAddress = currentChunk(current)->count;
    emitBytes(parser, 0xff, 0xff);

    statement(parser);

    emitByte(parser, OP_POP_EXCEPTION_HANDLER);
    int successJump = emitJump(parser, OP_JUMP);
    match(parser, TOKEN_EOL);

    if (match(parser, TOKEN_CATCH))
    {
        beginScope();
        consume(parser, TOKEN_LEFT_PAREN, "Expect '(' after catch");
        consume(parser, TOKEN_IDENTIFIER, "Expect type name to catch");
        uint8_t name = identifierConstant(parser, &parser->previous);
        currentChunk(current)->code[exceptionType] = name;
        patchAddress(handlerAddress);
        if (match(parser, TOKEN_AS))
        {
            consume(parser, TOKEN_IDENTIFIER, "Expect identifier for exception instance");
            addLocal(parser, parser->previous);
            markInitialized();
            uint8_t ex_var = resolveLocal(parser, current, &parser->previous);
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

        patchAddress(finallyAddress);
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

void importStatement(Parser *parser)
{
    consume(parser, TOKEN_STRING, "Import needs a module to import");
    import_path(parser->previous.start, parser->previous.length);
}

void nextStatement(Parser *parser)
{
    if (parser->currentLoop == NULL) {
        error(parser, "Can't use 'next' outside of a loop.");
    }

    // Discard any locals created inside the loop.
    for (int i = current->localCount - 1;
        i >= 0 && current->locals[i].depth > parser->currentLoop->loopScopeDepth;
        i--) {
        emitByte(parser, OP_POP);
    }

    // Jump to top of current innermost loop.
    emitLoop(parser);
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
    else if (match(parser, TOKEN_THROW))
    {
        throwStatement(parser);
    }
    else if (match(parser, TOKEN_LEFT_BRACE))
    {
        beginScope();
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
    else
    {
        expressionStatement(parser);
    }
}
