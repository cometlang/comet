#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "compiler.h"
#include "import.h"
#include "mem.h"
#include "scanner.h"
#include "comet.h"

#if DEBUG_PRINT_CODE
#include "debug.h"
#endif

#define GLOBAL_SCOPE 0
#define UNINITIALIZED_LOCAL_SCOPE -1
#define UNRESOLVED_VARIABLE_INDEX -1

static void block();

typedef struct ClassCompiler
{
    struct ClassCompiler *enclosing;

    Token name;
    bool hasSuperclass;
} ClassCompiler;

typedef struct
{
    Token current;
    Token previous;
    bool hadError;
    bool panicMode;
    const char *filename;
    Scanner *scanner;
    ClassCompiler *currentClass;
} Parser;

typedef enum
{
    PREC_NONE,
    PREC_ASSIGNMENT, // =
    PREC_OR,         // ||
    PREC_AND,        // &&
    PREC_EQUALITY,   // == !=
    PREC_COMPARISON, // < > <= >=
    PREC_TERM,       // + -
    PREC_FACTOR,     // * / %
    PREC_UNARY,      // ! -
    PREC_INSTANCEOF, // instanceof
    PREC_CALL,       // . () []
    PREC_PRIMARY
} Precedence;

typedef void (*ParseFn)(Parser *parser, bool canAssign);

typedef struct
{
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
} ParseRule;

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

struct Compiler
{
    struct Compiler *enclosing;
    ObjFunction *function;
    FunctionType type;
    Local locals[MAX_VAR_COUNT];
    int localCount;
    Upvalue upvalues[MAX_VAR_COUNT];
    int scopeDepth;
};

static Compiler *current = NULL;

static VM *main_thread = NULL;

static Chunk *currentChunk(Compiler *compiler)
{
    return &compiler->function->chunk;
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

static void errorAtCurrent(Parser *parser, const char *message)
{
    errorAt(parser, &parser->current, message);
}

static void error(Parser *parser, const char *message)
{
    errorAt(parser, &parser->previous, message);
}

static void advance(Parser *parser)
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

static void consume(Parser *parser, TokenType type, const char *message)
{
    if (parser->current.type == type)
    {
        advance(parser);
        return;
    }

    errorAtCurrent(parser, message);
}

static bool check(Parser *parser, TokenType type)
{
    return parser->current.type == type;
}

static bool match(Parser *parser, TokenType type)
{
    if (!check(parser, type))
        return false;
    advance(parser);
    return true;
}

static void emitByte(Parser *parser, uint8_t byte)
{
    writeChunk(currentChunk(current), byte, parser->previous.line);
}

static void emitBytes(Parser *parser, uint8_t byte1, uint8_t byte2)
{
    emitByte(parser, byte1);
    emitByte(parser, byte2);
}

static void emitLoop(Parser *parser, int loopStart)
{
    emitByte(parser, OP_LOOP);

    int offset = currentChunk(current)->count - loopStart + 2;
    if (offset > UINT16_MAX)
        error(parser, "Loop body too large.");

    emitByte(parser, (offset >> 8) & 0xff);
    emitByte(parser, offset & 0xff);
}

static int emitJump(Parser *parser, uint8_t instruction)
{
    emitByte(parser, instruction);
    emitByte(parser, 0xff);
    emitByte(parser, 0xff);
    return currentChunk(current)->count - 2;
}

static void emitReturn(Parser *parser)
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

static int addConstant(Chunk *chunk, Value value)
{
    push(main_thread, value);
    writeValueArray(&chunk->constants, value);
    pop(main_thread);
    return chunk->constants.count - 1;
}

static uint8_t makeConstant(Parser *parser, Value value)
{
    int constant = addConstant(currentChunk(current), value);
    if (constant > UINT8_MAX)
    {
        error(parser, "Too many constants in one chunk.");
        return 0;
    }

    return (uint8_t)constant;
}

static void emitConstant(Parser *parser, Value value)
{
    emitBytes(parser, OP_CONSTANT, makeConstant(parser, value));
}

static void patchJump(Parser *parser, int offset)
{
    // -2 to adjust for the bytecode for the jump offset itself.
    int jump = currentChunk(current)->count - offset - 2;

    if (jump > UINT16_MAX)
    {
        error(parser, "Too much code to jump over.");
    }

    currentChunk(current)->code[offset] = (jump >> 8) & 0xff;
    currentChunk(current)->code[offset + 1] = jump & 0xff;
}

static void patchAddress(int offset)
{
    currentChunk(current)->code[offset] = (currentChunk(current)->count >> 8) & 0xff;
    currentChunk(current)->code[offset + 1] = currentChunk(current)->count & 0xff;
}

static void initCompiler(Compiler *compiler, FunctionType type, Parser *parser)
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
    if (type != TYPE_FUNCTION)
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

static ObjFunction *endCompiler(Parser *parser)
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

static void beginScope()
{
    current->scopeDepth++;
}

static void endScope(Parser *parser)
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

static void expression();
static void statement();
static void declaration();
static ParseRule *getRule(TokenType type);
static void parsePrecedence(Parser *parser, Precedence precedence);

static uint8_t identifierConstant(Parser *parser, Token *name)
{
    return makeConstant(parser, copyString(main_thread, name->start, name->length));
}

static bool identifiersEqual(Token *a, Token *b)
{
    if (a->length != b->length)
        return false;
    return memcmp(a->start, b->start, a->length) == 0;
}

static int resolveLocal(Parser *parser, Compiler *compiler, Token *name)
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

static int resolveUpvalue(Compiler *compiler, Parser *parser, Token *name)
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

static void addLocal(Parser *parser, Token name)
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

static void declareVariable(Parser *parser)
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

static uint8_t parseVariable(Parser *parser, const char *errorMessage)
{
    consume(parser, TOKEN_IDENTIFIER, errorMessage);

    declareVariable(parser);
    if (current->scopeDepth > GLOBAL_SCOPE)
        return 0;

    return identifierConstant(parser, &parser->previous);
}

static void markInitialized()
{
    if (current->scopeDepth == GLOBAL_SCOPE)
        return;
    current->locals[current->localCount - 1].depth = current->scopeDepth;
}

static void defineVariable(Parser *parser, uint8_t global)
{
    if (current->scopeDepth > GLOBAL_SCOPE)
    {
        markInitialized();
        return;
    }
    emitBytes(parser, OP_DEFINE_GLOBAL, global);
}

static uint8_t argumentList(Parser *parser, TokenType closingToken)
{
    uint8_t argCount = 0;
    if (!check(parser, closingToken))
    {
        do
        {
            expression(parser);
            if (argCount == 255)
            {
                error(parser, "Cannot have more than 255 arguments.");
            }
            argCount++;
        } while (match(parser, TOKEN_COMMA));
    }

    // Need to sort out a string for the closing token
    consume(parser, closingToken, "Expect ')' after arguments.");
    return argCount;
}

static void and_(Parser *parser, bool UNUSED(canAssign))
{
    int endJump = emitJump(parser, OP_JUMP_IF_FALSE);

    emitByte(parser, OP_POP);
    parsePrecedence(parser, PREC_AND);

    patchJump(parser, endJump);
}

static void binary(Parser *parser, bool UNUSED(canAssign))
{
    // Remember the operator.
    TokenType operatorType = parser->previous.type;

    // Compile the right operand.
    ParseRule *rule = getRule(operatorType);
    parsePrecedence(parser, (Precedence)(rule->precedence + 1));

    // Emit the operator instruction.
    switch (operatorType)
    {
    case TOKEN_BANG_EQUAL:
        emitBytes(parser, OP_EQUAL, OP_NOT);
        break;
    case TOKEN_EQUAL_EQUAL:
        emitByte(parser, OP_EQUAL);
        break;
    case TOKEN_GREATER:
        emitByte(parser, OP_GREATER);
        break;
    case TOKEN_GREATER_EQUAL:
        emitByte(parser, OP_GREATER_EQUAL);
        break;
    case TOKEN_LESS:
        emitByte(parser, OP_LESS);
        break;
    case TOKEN_LESS_EQUAL:
        emitByte(parser, OP_LESS_EQUAL);
        break;
    case TOKEN_PLUS:
        emitByte(parser, OP_ADD);
        break;
    case TOKEN_MINUS:
        emitByte(parser, OP_SUBTRACT);
        break;
    case TOKEN_STAR:
        emitByte(parser, OP_MULTIPLY);
        break;
    case TOKEN_SLASH:
        emitByte(parser, OP_DIVIDE);
        break;
    case TOKEN_INSTANCEOF:
        emitByte(parser, OP_INSTANCEOF);
        break;
    case TOKEN_PERCENT:
        emitByte(parser, OP_MODULO);
        break;
    default:
        return; // Unreachable.
    }
}

static void call(Parser *parser, bool UNUSED(canAssign))
{
    uint8_t argCount = argumentList(parser, TOKEN_RIGHT_PAREN);
    emitBytes(parser, OP_CALL, argCount);
}

static void dot(Parser *parser, bool canAssign)
{
    consume(parser, TOKEN_IDENTIFIER, "Expect property name after '.'.");
    uint8_t name = identifierConstant(parser, &parser->previous);

    if (canAssign && match(parser, TOKEN_EQUAL))
    {
        expression(parser);
        emitBytes(parser, OP_SET_PROPERTY, name);
    }
    else if (match(parser, TOKEN_LEFT_PAREN))
    {
        uint8_t argCount = argumentList(parser, TOKEN_RIGHT_PAREN);
        emitBytes(parser, OP_INVOKE, name);
        emitByte(parser, argCount);
    }
    else
    {
        emitBytes(parser, OP_GET_PROPERTY, name);
    }
}

static void literal(Parser *parser, bool UNUSED(canAssign))
{
    switch (parser->previous.type)
    {
    case TOKEN_FALSE:
        emitByte(parser, OP_FALSE);
        break;
    case TOKEN_NIL:
        emitByte(parser, OP_NIL);
        break;
    case TOKEN_TRUE:
        emitByte(parser, OP_TRUE);
        break;
    default:
        return; // Unreachable.
    }
}

static void grouping(Parser *parser, bool UNUSED(canAssign))
{
    expression(parser);
    consume(parser, TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

static void number(Parser *parser, bool UNUSED(canAssign))
{
    char number_chars[parser->previous.length + 1];
    int offset = 0;
    for (int i = 0; i < parser->previous.length; i++)
    {
        if (parser->previous.start[i] != '_')
            number_chars[offset++] = parser->previous.start[i];
    }
    number_chars[offset] = '\0';

    double value = strtod(number_chars, NULL);
    emitConstant(parser, create_number(main_thread, value));
}

static void or_(Parser *parser, bool UNUSED(canAssign))
{
    int elseJump = emitJump(parser, OP_JUMP_IF_FALSE);
    int endJump = emitJump(parser, OP_JUMP);

    patchJump(parser, elseJump);
    emitByte(parser, OP_POP);

    parsePrecedence(parser, PREC_OR);
    patchJump(parser, endJump);
}

static void string(Parser *parser, bool UNUSED(canAssign))
{
    emitConstant(parser, copyString(main_thread, parser->previous.start + 1,
                                    parser->previous.length - 2));
}

static void namedVariable(Parser *parser, Token name, bool canAssign)
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
    else
    {
        arg = identifierConstant(parser, &name);
        getOp = OP_GET_GLOBAL;
        setOp = OP_SET_GLOBAL;
    }

    if (canAssign && match(parser, TOKEN_EQUAL))
    {
        expression(parser);
        emitBytes(parser, setOp, (uint8_t)arg);
    }
    else
    {
        emitBytes(parser, getOp, (uint8_t)arg);
    }
}

static void variable(Parser *parser, bool canAssign)
{
    namedVariable(parser, parser->previous, canAssign);
}

static Token syntheticToken(const char *text)
{
    Token token;
    token.start = text;
    token.length = (int)strlen(text);
    return token;
}

static void pushSuperclass(Parser *parser)
{
    if (parser->currentClass == NULL)
        return;
    namedVariable(parser, syntheticToken("super"), false);
}

static void super_(Parser *parser, bool UNUSED(canAssign))
{
    if (parser->currentClass == NULL)
    {
        error(parser, "Cannot use 'super' outside of a class.");
    }
    else if (!parser->currentClass->hasSuperclass)
    {
        error(parser, "Cannot use 'super' in a class with no superclass.");
    }

    consume(parser, TOKEN_DOT, "Expect '.' after 'super'.");
    consume(parser, TOKEN_IDENTIFIER, "Expect superclass method name.");
    uint8_t name = identifierConstant(parser, &parser->previous);

    // Push the receiver.
    namedVariable(parser, syntheticToken("self"), false);

    if (match(parser, TOKEN_LEFT_PAREN))
    {
        uint8_t argCount = argumentList(parser, TOKEN_RIGHT_PAREN);

        pushSuperclass(parser);
        emitBytes(parser, OP_SUPER, argCount);
        emitByte(parser, name);
    }
    else
    {
        pushSuperclass(parser);
        emitBytes(parser, OP_GET_SUPER, name);
    }
}

static void self(Parser *parser, bool UNUSED(canAssign))
{
    if (parser->currentClass == NULL)
    {
        error(parser, "Cannot use 'self' outside of a class.");
    }
    else
    {
        variable(parser, false);
    }
}

static void unary(Parser *parser, bool UNUSED(canAssign))
{
    TokenType operatorType = parser->previous.type;

    // Compile the operand.
    parsePrecedence(parser, PREC_UNARY);

    // Emit the operator instruction.
    switch (operatorType)
    {
    case TOKEN_BANG:
        emitByte(parser, OP_NOT);
        break;
    case TOKEN_MINUS:
        emitByte(parser, OP_NEGATE);
        break;
    default:
        return; // Unreachable.
    }
}

static void literal_hash(Parser *parser, bool canAssign)
{
    namedVariable(parser, syntheticToken("Hash"), canAssign);
    emitBytes(parser, OP_CALL, 0);
    emitByte(parser, OP_DUP_TOP);
    match(parser, TOKEN_EOL);

    if (!check(parser, TOKEN_RIGHT_BRACE))
    {
        do {
            emitByte(parser, OP_DUP_TOP);
            match(parser, TOKEN_EOL);
            expression(parser);
            consume(parser, TOKEN_COLON, "':' expected between key and value of a literal hash");
            match(parser, TOKEN_EOL);
            expression(parser);
            Token addToken = syntheticToken("add");
            uint8_t name = identifierConstant(parser, &addToken);
            emitBytes(parser, OP_INVOKE, name);
            emitByte(parser, 2); // argCount
            emitByte(parser, OP_POP);
        } while (match(parser, TOKEN_COMMA));
    }

    emitByte(parser, OP_POP);
    match(parser, TOKEN_EOL);
    consume(parser, TOKEN_RIGHT_BRACE, "Expected '}' for a literal hash declaration");
}

static void literal_list(Parser *parser, bool canAssign)
{
    namedVariable(parser, syntheticToken("List"), canAssign);
    emitBytes(parser, OP_CALL, 0); // Create a list using the default constructor
    emitByte(parser, OP_DUP_TOP); // Duplicate the list instance, so the pop leaves it for the return value of assignment
    uint8_t argCount = argumentList(parser, TOKEN_RIGHT_SQ_BRACKET);
    if (argCount > 0)
    {
        Token addToken = syntheticToken("add");
        uint8_t name = identifierConstant(parser, &addToken);
        emitBytes(parser, OP_INVOKE, name);
        emitByte(parser, argCount);
    }
    emitByte(parser, OP_POP);
}

static void subscript(Parser *parser, bool UNUSED(canAssign))
{
    uint8_t argCount = argumentList(parser, TOKEN_RIGHT_SQ_BRACKET);
    emitBytes(parser, OP_INDEX, argCount);
}

static void lambda(Parser *parser, bool UNUSED(canAssign))
{
    Compiler compiler;
    initCompiler(&compiler, TYPE_LAMBDA, parser);
    beginScope();

    if (!check(parser, TOKEN_VBAR))
    {
        do
        {
            current->function->arity++;
            if (current->function->arity > 255)
            {
                errorAtCurrent(parser, "Cannot have more than 255 parameters.");
            }

            uint8_t paramConstant = parseVariable(parser, "Expect parameter name.");
            defineVariable(parser, paramConstant);
        } while (match(parser, TOKEN_COMMA));
    }
    consume(parser, TOKEN_VBAR, "Expect '|' after lambda parameters.");

    // The body.
    match(parser, TOKEN_EOL);
    consume(parser, TOKEN_LEFT_BRACE, "Expect '{' before lambda body.");
    block(parser);

    // Create the function object.
    ObjFunction *function = endCompiler(parser);
    emitBytes(parser, OP_CLOSURE, makeConstant(parser, OBJ_VAL(function)));

    for (int i = 0; i < function->upvalueCount; i++)
    {
        emitByte(parser, compiler.upvalues[i].isLocal ? 1 : 0);
        emitByte(parser, compiler.upvalues[i].index);
    }
}

ParseRule rules[NUM_TOKENS] = {
    {grouping, call, PREC_CALL},     // TOKEN_LEFT_PAREN
    {NULL, NULL, PREC_NONE},         // TOKEN_RIGHT_PAREN
    {literal_hash, NULL, PREC_NONE}, // TOKEN_LEFT_BRACE
    {NULL, NULL, PREC_NONE},         // TOKEN_RIGHT_BRACE
    {literal_list, subscript, PREC_CALL},// TOKEN_LEFT_SQ_BRACKET
    {NULL, NULL, PREC_NONE},         // TOKEN_RIGHT_SQ_BRACKET
    {NULL, NULL, PREC_NONE},         // TOKEN_COMMA
    {NULL, dot, PREC_CALL},          // TOKEN_DOT
    {unary, binary, PREC_TERM},      // TOKEN_MINUS
    {NULL, binary, PREC_TERM},       // TOKEN_PLUS
    {NULL, NULL, PREC_NONE},         // TOKEN_SEMI_COLON
    {NULL, binary, PREC_FACTOR},     // TOKEN_SLASH
    {NULL, binary, PREC_FACTOR},     // TOKEN_STAR
    {NULL, NULL, PREC_NONE},         // TOKEN_COLON
    {NULL, NULL, PREC_NONE},         // TOKEN_EOL
    {lambda, NULL, PREC_NONE},       // TOKEN_VBAR
    {NULL, binary, PREC_FACTOR},     // TOKEN_PERCENT
    {unary, NULL, PREC_NONE},        // TOKEN_BANG
    {NULL, binary, PREC_EQUALITY},   // TOKEN_BANG_EQUAL
    {NULL, NULL, PREC_NONE},         // TOKEN_EQUAL
    {NULL, binary, PREC_EQUALITY},   // TOKEN_EQUAL_EQUAL
    {NULL, binary, PREC_COMPARISON}, // TOKEN_GREATER
    {NULL, binary, PREC_COMPARISON}, // TOKEN_GREATER_EQUAL
    {NULL, binary, PREC_COMPARISON}, // TOKEN_LESS
    {NULL, binary, PREC_COMPARISON}, // TOKEN_LESS_EQUAL
    {NULL, or_, PREC_OR},            // TOKEN_LOGICAL_OR
    {NULL, and_, PREC_AND},          // TOKEN_LOGICAL_AND
    {variable, NULL, PREC_NONE},     // TOKEN_IDENTIFIER
    {string, NULL, PREC_NONE},       // TOKEN_STRING
    {number, NULL, PREC_NONE},       // TOKEN_NUMBER
    {NULL, NULL, PREC_NONE},         // TOKEN_AS
    {NULL, NULL, PREC_NONE},         // TOKEN_CLASS
    {NULL, NULL, PREC_NONE},         // TOKEN_ELSE
    {NULL, NULL, PREC_NONE},         // TOKEN_ENUM
    {literal, NULL, PREC_NONE},      // TOKEN_FALSE
    {NULL, NULL, PREC_NONE},         // TOKEN_FOR
    {NULL, NULL, PREC_NONE},         // TOKEN_FUN
    {NULL, NULL, PREC_NONE},         // TOKEN_IF
    {NULL, NULL, PREC_NONE},         // TOKEN_IMPORT
    {NULL, NULL, PREC_NONE},         // TOKEN_IN
    {NULL, binary, PREC_INSTANCEOF}, // TOKEN_INSTANCEOF
    {literal, NULL, PREC_NONE},      // TOKEN_NIL
    {NULL, NULL, PREC_NONE},         // TOKEN_OPERATOR
    {NULL, NULL, PREC_NONE},         // TOKEN_RETURN
    {self, NULL, PREC_NONE},         // TOKEN_SELF
    {super_, NULL, PREC_NONE},       // TOKEN_SUPER
    {literal, NULL, PREC_NONE},      // TOKEN_TRUE
    {NULL, NULL, PREC_NONE},         // TOKEN_VAR
    {NULL, NULL, PREC_NONE},         // TOKEN_WHILE
    {NULL, NULL, PREC_NONE},         // TOKEN_TRY
    {NULL, NULL, PREC_NONE},         // TOKEN_CATCH
    {NULL, NULL, PREC_NONE},         // TOKEN_THROW
    {NULL, NULL, PREC_NONE},         // TOKEN_PRIVATE
    {NULL, NULL, PREC_NONE},         // TOKEN_PROTECTED
    {NULL, NULL, PREC_NONE},         // TOKEN_PUBLIC
    {NULL, NULL, PREC_NONE},         // TOKEN_STATIC
    {NULL, NULL, PREC_NONE},         // TOKEN_ERROR
    {NULL, NULL, PREC_NONE},         // TOKEN_EOF
};

static void parsePrecedence(Parser *parser, Precedence precedence)
{
    advance(parser);
    ParseFn prefixRule = getRule(parser->previous.type)->prefix;
    if (prefixRule == NULL)
    {
        error(parser, "Expect expression.");
        return;
    }

    bool canAssign = precedence <= PREC_ASSIGNMENT;
    prefixRule(parser, canAssign);

    while (precedence <= getRule(parser->current.type)->precedence)
    {
        advance(parser);
        ParseFn infixRule = getRule(parser->previous.type)->infix;
        infixRule(parser, canAssign);
    }

    if (canAssign && match(parser, TOKEN_EQUAL))
    {
        error(parser, "Invalid assignment target.");
    }
}

static ParseRule *getRule(TokenType type)
{
    return &rules[type];
}

void expression(Parser *parser)
{
    parsePrecedence(parser, PREC_ASSIGNMENT);
}

static void block(Parser *parser)
{
    match(parser, TOKEN_EOL);
    while (!check(parser, TOKEN_RIGHT_BRACE) && !check(parser, TOKEN_EOF))
    {
        declaration(parser);
    }

    match(parser, TOKEN_EOL); // optional end of line.
    consume(parser, TOKEN_RIGHT_BRACE, "Expect '}' after block.");
}

static void function(Parser *parser, FunctionType type)
{
    Compiler compiler;
    initCompiler(&compiler, type, parser);
    beginScope();

    // Compile the parameter list.
    consume(parser, TOKEN_LEFT_PAREN, "Expect '(' after function name.");
    if (!check(parser, TOKEN_RIGHT_PAREN))
    {
        do
        {
            current->function->arity++;
            if (current->function->arity > 255)
            {
                errorAtCurrent(parser, "Cannot have more than 255 parameters.");
            }

            uint8_t paramConstant = parseVariable(parser, "Expect parameter name.");
            defineVariable(parser, paramConstant);
        } while (match(parser, TOKEN_COMMA));
    }
    consume(parser, TOKEN_RIGHT_PAREN, "Expect ')' after parameters.");

    // The body.
    match(parser, TOKEN_EOL);
    consume(parser, TOKEN_LEFT_BRACE, "Expect '{' before function body.");
    block(parser);

    // Create the function object.
    ObjFunction *function = endCompiler(parser);
    emitBytes(parser, OP_CLOSURE, makeConstant(parser, OBJ_VAL(function)));

    for (int i = 0; i < function->upvalueCount; i++)
    {
        emitByte(parser, compiler.upvalues[i].isLocal ? 1 : 0);
        emitByte(parser, compiler.upvalues[i].index);
    }
}

static void method(Parser *parser)
{
    bool isStatic = match(parser, TOKEN_STATIC);

    consume(parser, TOKEN_IDENTIFIER, "Expect method name.");
    uint8_t constant = identifierConstant(parser, &parser->previous);

    // If the method is named "init", it's an initializer.
    FunctionType type = TYPE_METHOD;
    if (parser->previous.length == 4 &&
        memcmp(parser->previous.start, "init", 4) == 0)
    {
        if (isStatic)
            error(parser, "Initializer can't be declared static");
        type = TYPE_INITIALIZER;
    }

    function(parser, type);

    if (isStatic)
    {
        emitBytes(parser, OP_STATIC_METHOD, constant);
    }
    else
    {
        emitBytes(parser, OP_METHOD, constant);
    }
}

static void operator(Parser *parser)
{
    OPERATOR op = getOperatorFromToken(parser->current.type);
    if (op == OPERATOR_UNKNOWN)
    {
        error(parser, "Unsupported operator for overloading");
    }
    advance(parser);
    if (op == OPERATOR_INDEX)
    {
        consume(parser, TOKEN_RIGHT_SQ_BRACKET, "expected ']'");
        if (match(parser, TOKEN_EQUAL))
        {
            op = OPERATOR_INDEX_ASSIGN;
        }
    }
    // For all intents and purposes, this is a method.
    function(parser, TYPE_METHOD);
    emitBytes(parser, OP_DEFINE_OPERATOR, op);
}

static void classDeclaration(Parser *parser)
{
    consume(parser, TOKEN_IDENTIFIER, "Expect class name.");
    Token className = parser->previous;
    uint8_t nameConstant = identifierConstant(parser, &parser->previous);
    declareVariable(parser);

    emitBytes(parser, OP_CLASS, nameConstant);
    defineVariable(parser, nameConstant);

    ClassCompiler classCompiler;
    classCompiler.name = parser->previous;
    classCompiler.hasSuperclass = false;
    classCompiler.enclosing = parser->currentClass;
    parser->currentClass = &classCompiler;

    if (match(parser, TOKEN_COLON))
    {
        consume(parser, TOKEN_IDENTIFIER, "Expect superclass name.");
        variable(parser, false);

        if (identifiersEqual(&className, &parser->previous))
        {
            error(parser, "A class cannot inherit from itself.");
        }
        beginScope();
        variable(parser, false);
    }
    else
    {
        beginScope();
        namedVariable(parser, syntheticToken("Object"), false);
    }

    // Store the superclass in a local variable named "super".
    addLocal(parser, syntheticToken("super"));
    defineVariable(parser, 0);

    namedVariable(parser, className, false);
    emitByte(parser, OP_INHERIT);
    classCompiler.hasSuperclass = true;

    namedVariable(parser, className, false);
    match(parser, TOKEN_EOL); // optional end of line.
    consume(parser, TOKEN_LEFT_BRACE, "Expect '{' before class body.");
    while (!check(parser, TOKEN_RIGHT_BRACE) && !check(parser, TOKEN_EOF))
    {
        if (match(parser, TOKEN_EOL))
        {
            // do nothing.
        }
        else if (match(parser, TOKEN_OPERATOR))
        {
            operator(parser);
        }
        else
        {
            method(parser);
        }
    }

    consume(parser, TOKEN_RIGHT_BRACE, "Expect '}' after class body.");
    emitByte(parser, OP_POP);

    if (classCompiler.hasSuperclass)
    {
        endScope(parser);
    }

    parser->currentClass = parser->currentClass->enclosing;
}

static void funDeclaration(Parser *parser)
{
    uint8_t global = parseVariable(parser, "Expect function name.");
    markInitialized();
    function(parser, TYPE_FUNCTION);
    defineVariable(parser, global);
}

static void enumDeclaration(Parser *parser)
{
    uint8_t enumName = parseVariable(parser, "Expect enum name");
    defineVariable(parser, enumName);
    namedVariable(parser, syntheticToken("Enum"), false);
    emitBytes(parser, OP_CALL, 0);
    emitByte(parser, OP_DUP_TOP); // Duplicate the enum instance, so the pop leaves it for the return value of assignment
    defineVariable(parser, enumName);

    consume(parser, TOKEN_LEFT_BRACE, "Expect '{' after enum declaration");
    int64_t current_value = -1;
    while (!check(parser, TOKEN_RIGHT_BRACE) && !check(parser, TOKEN_EOF))
    {
        match(parser, TOKEN_EOL); // optional end of line.
        if (match(parser, TOKEN_IDENTIFIER))
        {
            emitByte(parser, OP_DUP_TOP);
            emitBytes(parser, OP_CONSTANT, identifierConstant(parser, &parser->previous));

            if (match(parser, TOKEN_EQUAL))
            {
                current_value = strtol(parser->current.start, NULL, 10);
                if (errno == ERANGE)
                    error(parser, "Expect an integer for the enum value");
                advance(parser);
            }
            else
            {
                current_value++;
            }
            emitConstant(parser, create_number(main_thread, current_value));

            Token addToken = syntheticToken("add");
            uint8_t name = identifierConstant(parser, &addToken);
            emitBytes(parser, OP_INVOKE, name);
            emitByte(parser, 2); // argCount
            emitByte(parser, OP_POP);

            if (!match(parser, TOKEN_COMMA) && !check(parser, TOKEN_RIGHT_BRACE))
            {
                error(parser, "Expect ',' between enum values");
                break;
            }
        }
    }
    consume(parser, TOKEN_RIGHT_BRACE, "Expect '}' after enum body.");
    emitByte(parser, OP_POP);
}

static void varDeclaration(Parser *parser)
{
    uint8_t global = parseVariable(parser, "Expect variable name.");

    if (match(parser, TOKEN_EQUAL))
    {
        expression(parser);
    }
    else
    {
        emitByte(parser, OP_NIL);
    }

    defineVariable(parser, global);
}

static void expressionStatement(Parser *parser)
{
    expression(parser);
    if (!check(parser, TOKEN_EOF))
    {
        consume(parser, TOKEN_EOL, "Only one statement per line allowed");
    }
    emitByte(parser, OP_POP);
}

static void forStatement(Parser *parser)
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

    int loopStart = currentChunk(current)->count;

    int exitJump = -1;
    if (!match(parser, TOKEN_SEMI_COLON))
    {
        expression(parser);
        consume(parser, TOKEN_SEMI_COLON, "Expect ';' after loop condition.");

        // Jump out of the loop if the condition is false.
        exitJump = emitJump(parser, OP_JUMP_IF_FALSE);
        emitByte(parser, OP_POP); // Condition.
    }
    if (!match(parser, TOKEN_RIGHT_PAREN))
    {
        int bodyJump = emitJump(parser, OP_JUMP);

        int incrementStart = currentChunk(current)->count;
        expression(parser);
        emitByte(parser, OP_POP);
        consume(parser, TOKEN_RIGHT_PAREN, "Expect ')' after for clauses.");

        emitLoop(parser, loopStart);
        loopStart = incrementStart;
        patchJump(parser, bodyJump);
    }

    statement(parser);

    emitLoop(parser, loopStart);

    if (exitJump != -1)
    {
        patchJump(parser, exitJump);
        emitByte(parser, OP_POP); // Condition.
    }

    endScope(parser);
}

static void syntheticMethodCall(Parser *parser, const char *method_name)
{
    Token methodNameToken = syntheticToken(method_name);
    uint8_t constant = identifierConstant(parser, &methodNameToken);
    emitBytes(parser, OP_INVOKE, constant);
    emitByte(parser, 0);
}

static void foreachStatement(Parser *parser)
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

    int exitJump = -1;
    int loopStart = currentChunk(current)->count;

    emitBytes(parser, OP_GET_LOCAL, ex_var);
    syntheticMethodCall(parser, "has_next?");
    exitJump = emitJump(parser, OP_JUMP_IF_FALSE);
    emitByte(parser, OP_POP);

    emitBytes(parser, OP_GET_LOCAL, ex_var);
    syntheticMethodCall(parser, "get_next");
    int variable = resolveLocal(parser, current, &loop_var_name);
    emitBytes(parser, OP_SET_LOCAL, (uint8_t) variable);

    statement(parser);
    emitByte(parser, OP_POP);

    emitLoop(parser, loopStart);

    patchJump(parser, exitJump);

    endScope(parser);
}

static void ifStatement(Parser *parser)
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

static void returnStatement(Parser *parser)
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

static void whileStatement(Parser *parser)
{
    int loopStart = currentChunk(current)->count;
    consume(parser, TOKEN_LEFT_PAREN, "Expect '(' after 'while'.");
    expression(parser);
    consume(parser, TOKEN_RIGHT_PAREN, "Expect ')' after condition.");

    int exitJump = emitJump(parser, OP_JUMP_IF_FALSE);

    emitByte(parser, OP_POP);
    statement(parser);

    emitLoop(parser, loopStart);

    patchJump(parser, exitJump);
    emitByte(parser, OP_POP);
}

static void tryStatement(Parser *parser)
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

static void throwStatement(Parser *parser)
{
    expression(parser);
    if (!check(parser, TOKEN_EOF))
        consume(parser, TOKEN_EOL, "Only one statement per line allowed");
    emitByte(parser, OP_THROW);
}

static void importStatement(Parser *parser)
{
    consume(parser, TOKEN_STRING, "Import needs a module to import");
    import_path(parser->previous.start, parser->previous.length);
}

static void synchronize(Parser *parser)
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

static void declaration(Parser *parser)
{
    if (match(parser, TOKEN_CLASS))
    {
        classDeclaration(parser);
    }
    else if (match(parser, TOKEN_FUN))
    {
        funDeclaration(parser);
    }
    else if (match(parser, TOKEN_ENUM))
    {
        enumDeclaration(parser);
    }
    else if (match(parser, TOKEN_VAR))
    {
        varDeclaration(parser);
        if (!check(parser, TOKEN_EOF))
            consume(parser, TOKEN_EOL, "Only one statement per line allowed");
    }
    else if (match(parser, TOKEN_EOL))
    {
        // Do nothing, but don't error, this is a blank line
    }
    else if (match(parser, TOKEN_SEMI_COLON))
    {
        error(parser, "Unexpected ';'");
    }
    else
    {
        statement(parser);
    }

    if (parser->panicMode)
        synchronize(parser);
}

static void statement(Parser *parser)
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
    else
    {
        expressionStatement(parser);
    }
}

void initParser(Parser *parser, Scanner *scanner, const char *filename)
{
    parser->filename = filename;
    parser->hadError = false;
    parser->panicMode = false;
    parser->scanner = scanner;
    parser->currentClass = NULL;
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
