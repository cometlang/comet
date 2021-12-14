#ifndef _COMPILER_DEFS_H_
#define _COMPILER_DEFS_H_

#include "scanner.h"
#include "object_defs.h"
#include "compiler.h"

typedef struct ClassCompiler
{
    struct ClassCompiler *enclosing;

    Token name;
    bool hasSuperclass;
} ClassCompiler;

typedef struct LoopCompiler
{
    struct LoopCompiler *enclosing;
    int startAddress;
    int exitAddress;
    int loopScopeDepth;
} LoopCompiler;

typedef enum
{
    PREC_NONE,
    PREC_ASSIGNMENT,  // =
    PREC_OR,          // ||
    PREC_AND,         // &&
    PREC_EQUALITY,    // == !=
    PREC_BITSHIFT,    // << >>
    PREC_XOR,         // ^
    PREC_BITWISE_OR,  // |
    PREC_BITWISE_AND, // &
    PREC_COMPARISON,  // < > <= >=
    PREC_TERM,        // + -
    PREC_FACTOR,      // * / %
    PREC_UNARY,       // ! - ~
    PREC_CALL,        // . () []
    PREC_INSTANCEOF,  // instanceof
    PREC_PRIMARY,
} Precedence;

typedef struct
{
    Token name;
    int depth;
    bool isCaptured;
} Local;

typedef struct
{
    uint8_t index;
    bool isLocal;
} Upvalue;

typedef enum
{
    TYPE_FUNCTION,
    TYPE_INITIALIZER,
    TYPE_METHOD,
    TYPE_SCRIPT,
    TYPE_LAMBDA,
} FunctionType;

typedef struct Compiler
{
    struct Compiler *enclosing;
    ObjFunction *function;
    FunctionType type;
    Local locals[MAX_VAR_COUNT];
    int localCount;
    Upvalue upvalues[MAX_VAR_COUNT];
    int scopeDepth;
} Compiler;

typedef struct
{
    Token current;
    Token previous;
    bool hadError;
    bool panicMode;
    const char *filename;
    Scanner *scanner;
    ClassCompiler *currentClass;
    LoopCompiler *currentLoop;
    VALUE currentModule;
    Compiler *currentFunction;
    VM *compilation_thread;
} Parser;

typedef void (*ParseFn)(Parser *parser, bool canAssign);

typedef struct
{
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
} ParseRule;

Token syntheticToken(const char *text);
Chunk *currentChunk(Compiler *compiler);
int getCurrentOffset(Compiler *compiler);
void setCodeOffset(Compiler *compiler, int offset, uint8_t value);

void synchronize(Parser *parser);

void initCompiler(Compiler *compiler, FunctionType type, Parser *parser);
ObjFunction *endCompiler(Parser *parser);

void patchJump(Parser *parser, int offset);

void beginScope(Parser *parser);
void endScope(Parser *parser);

void advance(Parser *parser);
void consume(Parser *parser, TokenType_t type, const char *message);
bool check(Parser *parser, TokenType_t type);
bool match(Parser *parser, TokenType_t type);
void error(Parser *parser, const char *message);
void errorAtCurrent(Parser *parser, const char *message);

#endif
