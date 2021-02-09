#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "compiler.h"
#include "compiler_defs.h"
#include "statements.h"
#include "declarations.h"
#include "expressions.h"
#include "emitter.h"
#include "mem.h"
#include "scanner.h"
#include "comet.h"

#if DEBUG_PRINT_CODE
#include "debug.h"
#endif

Compiler *current = NULL;
VM *main_thread = NULL;

Chunk *currentChunk(Compiler *compiler)
{
    return &compiler->function->chunk;
}

int getCurrentOffset(Compiler *compiler)
{
    return compiler->function->chunk.count;
}

void setCodeOffset(Compiler *compiler, int offset, uint8_t value)
{
    currentChunk(compiler)->code[offset] = value;
}

static void errorAt(Parser *parser, Token *token, const char *message)
{
    if (parser->panicMode)
        return;
    parser->panicMode = true;
    fprintf(stderr, "[%s:%d] Error", parser->filename, token->line);

    if (token->type == TOKEN_EOF)
    {
        fprintf(stderr, " at end");
    }
    else if (token->type == TOKEN_ERROR)
    {
        // Nothing.
    }
    else
    {
        fprintf(stderr, " at '%.*s'", token->length, token->start);
    }

    fprintf(stderr, ": %s\n", message);
    parser->hadError = true;
}

void errorAtCurrent(Parser *parser, const char *message)
{
    errorAt(parser, &parser->current, message);
}

void error(Parser *parser, const char *message)
{
    errorAt(parser, &parser->previous, message);
}

void advance(Parser *parser)
{
    parser->previous = parser->current;

    for (;;)
    {
        parser->current = scanToken(parser->scanner);
        if (parser->current.type != TOKEN_ERROR)
            break;

        errorAtCurrent(parser, parser->current.start);
    }
}

void consume(Parser *parser, TokenType type, const char *message)
{
    if (parser->current.type == type)
    {
        advance(parser);
        return;
    }

    errorAtCurrent(parser, message);
}

bool check(Parser *parser, TokenType type)
{
    return parser->current.type == type;
}

bool match(Parser *parser, TokenType type)
{
    if (!check(parser, type))
        return false;
    advance(parser);
    return true;
}

void patchJump(Parser *parser, int offset)
{
    // -2 to adjust for the bytecode for the jump offset itself.
    int jump = getCurrentOffset(current) - offset - 2;

    if (jump > UINT16_MAX)
    {
        error(parser, "Too much code to jump over.");
    }
    setCodeOffset(current, offset, (jump >> 8) & 0xff);
    setCodeOffset(current, offset + 1, jump & 0xff);
}

void initCompiler(Compiler *compiler, FunctionType type, Parser *parser)
{
    compiler->enclosing = current;
    compiler->function = NULL;
    compiler->type = type;
    compiler->localCount = 0;
    compiler->scopeDepth = 0;
    compiler->function = newFunction(main_thread);
    compiler->function->chunk.filename = parser->filename;
    current = compiler;

    if (type != TYPE_SCRIPT && type != TYPE_LAMBDA)
    {
        current->function->name = copyString(main_thread, parser->previous.start,
                                             parser->previous.length);
    }

    Local *local = &current->locals[current->localCount++];
    local->depth = 0;
    local->isCaptured = false;
    if (type == TYPE_METHOD || type == TYPE_INITIALIZER)
    {
        // In a method, it holds the receiver, "self".
        local->name.start = "self";
        local->name.length = 4;
    }
    else
    {
        // In a function, it holds the function, but cannot be referenced,
        // so has no name.
        local->name.start = "";
        local->name.length = 0;
    }
}

ObjFunction *endCompiler(Parser *parser)
{
    emitReturn(parser);
    ObjFunction *function = current->function;
#if DEBUG_PRINT_CODE
    if (!parser->hadError)
    {
        disassembleChunk(currentChunk(current),
                         function->name != NIL_VAL ? string_get_cstr(function->name) : "<script>");
    }
#endif
    current = current->enclosing;
    return function;
}

void beginScope(void)
{
    current->scopeDepth++;
}

void endScope(Parser *parser)
{
    current->scopeDepth--;

    while (current->localCount > 0 &&
           current->locals[current->localCount - 1].depth > current->scopeDepth)
    {
        if (current->locals[current->localCount - 1].isCaptured)
        {
            emitByte(parser, OP_CLOSE_UPVALUE);
        }
        else
        {
            emitByte(parser, OP_POP);
        }
        current->localCount--;
    }
}

Token syntheticToken(const char *text)
{
    Token token;
    token.start = text;
    token.length = (int)strlen(text);
    return token;
}

void synchronize(Parser *parser)
{
    parser->panicMode = false;

    while (parser->current.type != TOKEN_EOF)
    {
        if (parser->previous.type == TOKEN_EOL)
            return;

        switch (parser->current.type)
        {
        case TOKEN_CLASS:
        case TOKEN_FUN:
        case TOKEN_VAR:
        case TOKEN_OPERATOR:
        case TOKEN_FOR:
        case TOKEN_FOREACH:
        case TOKEN_IF:
        case TOKEN_WHILE:
        case TOKEN_THROW:
        case TOKEN_RETURN:
        case TOKEN_STATIC:
        case TOKEN_TRY:
        case TOKEN_IMPORT:
        case TOKEN_ENUM:
            return;

        default:
            // Do nothing.
            ;
        }

        advance(parser);
    }
}

void initParser(Parser *parser, Scanner *scanner, const char *filename)
{
    parser->filename = filename;
    parser->hadError = false;
    parser->panicMode = false;
    parser->scanner = scanner;
    parser->currentClass = NULL;
    parser->currentLoop = NULL;
}

ObjFunction *compile(const SourceFile *source, VM *thread)
{
    Scanner scanner;
    initScanner(&scanner, source);
    main_thread = thread;

    Parser parser;
    initParser(&parser, &scanner, source->path);

    Compiler compiler;
    initCompiler(&compiler, TYPE_SCRIPT, &parser);

    // chicken-egg situation.
    advance(&parser);
    while (!match(&parser, TOKEN_EOF))
    {
        declaration(&parser);
    }
    ObjFunction *function = endCompiler(&parser);
    return parser.hadError ? NULL : function;
}

void markCompilerRoots(void)
{
    Compiler *compiler = current;
    while (compiler != NULL)
    {
        markObject((Obj *)compiler->function);
        compiler = compiler->enclosing;
    }
}