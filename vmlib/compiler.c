#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "compiler.h"
#include "mem.h"
#include "scanner.h"

#if DEBUG_PRINT_CODE
#include "debug.h"
#endif

#define GLOBAL_SCOPE 0
#define UNINITIALIZED_LOCAL_SCOPE -1
#define UNRESOLVED_VARIABLE_INDEX -1

typedef struct
{
    Token current;
    Token previous;
    bool hadError;
    bool panicMode;
    const char *filename;
} Parser;

typedef enum
{
    PREC_NONE,
    PREC_ASSIGNMENT, // =
    PREC_OR,         // or
    PREC_AND,        // and
    PREC_EQUALITY,   // == !=
    PREC_COMPARISON, // < > <= >=
    PREC_TERM,       // + -
    PREC_FACTOR,     // * /
    PREC_UNARY,      // ! -
    PREC_CALL,       // . () []
    PREC_PRIMARY
} Precedence;

typedef void (*ParseFn)(bool canAssign);

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
    TYPE_SCRIPT
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

typedef struct ClassCompiler
{
    struct ClassCompiler *enclosing;

    Token name;
    bool hasSuperclass;
} ClassCompiler;

static Parser parser;

static Compiler *current = NULL;

ClassCompiler *currentClass = NULL;

static VM *main_thread = NULL;

static Chunk *currentChunk()
{
    return &current->function->chunk;
}

static void errorAt(Token *token, const char *message)
{
    if (parser.panicMode)
        return;
    parser.panicMode = true;
    fprintf(stderr, "[%s:%d] Error", parser.filename, token->line);

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
    parser.hadError = true;
}

static void errorAtCurrent(const char *message)
{
    errorAt(&parser.current, message);
}

static void error(const char *message)
{
    errorAt(&parser.previous, message);
}

static void advance()
{
    parser.previous = parser.current;

    for (;;)
    {
        parser.current = scanToken();
        if (parser.current.type != TOKEN_ERROR)
            break;

        errorAtCurrent(parser.current.start);
    }
}

static void consume(TokenType type, const char *message)
{
    if (parser.current.type == type)
    {
        advance();
        return;
    }

    errorAtCurrent(message);
}

static bool check(TokenType type)
{
    return parser.current.type == type;
}

static bool match(TokenType type)
{
    if (!check(type))
        return false;
    advance();
    return true;
}

static void emitByte(uint8_t byte)
{
    writeChunk(currentChunk(), byte, parser.previous.line);
}

static void emitBytes(uint8_t byte1, uint8_t byte2)
{
    emitByte(byte1);
    emitByte(byte2);
}

static void emitLoop(int loopStart)
{
    emitByte(OP_LOOP);

    int offset = currentChunk()->count - loopStart + 2;
    if (offset > UINT16_MAX)
        error("Loop body too large.");

    emitByte((offset >> 8) & 0xff);
    emitByte(offset & 0xff);
}

static int emitJump(uint8_t instruction)
{
    emitByte(instruction);
    emitByte(0xff);
    emitByte(0xff);
    return currentChunk()->count - 2;
}

static void emitReturn()
{
    // An initializer automatically returns "self".
    if (current->type == TYPE_INITIALIZER)
    {
        emitBytes(OP_GET_LOCAL, 0);
    }
    else
    {
        emitByte(OP_NIL);
    }
    emitByte(OP_RETURN);
}

static int addConstant(Chunk *chunk, Value value)
{
    push(main_thread, value);
    writeValueArray(&chunk->constants, value);
    pop(main_thread);
    return chunk->constants.count - 1;
}

static uint8_t makeConstant(Value value)
{
    int constant = addConstant(currentChunk(), value);
    if (constant > UINT8_MAX)
    {
        error("Too many constants in one chunk.");
        return 0;
    }

    return (uint8_t)constant;
}

static void emitConstant(Value value)
{
    emitBytes(OP_CONSTANT, makeConstant(value));
}

static void patchJump(int offset)
{
    // -2 to adjust for the bytecode for the jump offset itself.
    int jump = currentChunk()->count - offset - 2;

    if (jump > UINT16_MAX)
    {
        error("Too much code to jump over.");
    }

    currentChunk()->code[offset] = (jump >> 8) & 0xff;
    currentChunk()->code[offset + 1] = jump & 0xff;
}

static void initCompiler(Compiler *compiler, FunctionType type)
{
    compiler->enclosing = current;
    compiler->function = NULL;
    compiler->type = type;
    compiler->localCount = 0;
    compiler->scopeDepth = 0;
    compiler->function = newFunction(main_thread);
    compiler->function->chunk.filename = parser.filename;
    current = compiler;

    if (type != TYPE_SCRIPT)
    {
        current->function->name = copyString(main_thread, parser.previous.start,
                                             parser.previous.length);
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

static ObjFunction *endCompiler()
{
    emitReturn();
    ObjFunction *function = current->function;
#if DEBUG_PRINT_CODE
    if (!parser.hadError)
    {
        disassembleChunk(currentChunk(),
                         function->name != NULL ? function->name->chars : "<script>");
    }
#endif
    current = current->enclosing;
    return function;
}

static void beginScope()
{
    current->scopeDepth++;
}

static void endScope()
{
    current->scopeDepth--;

    while (current->localCount > 0 &&
           current->locals[current->localCount - 1].depth > current->scopeDepth)
    {
        if (current->locals[current->localCount - 1].isCaptured)
        {
            emitByte(OP_CLOSE_UPVALUE);
        }
        else
        {
            emitByte(OP_POP);
        }
        current->localCount--;
    }
}

static void expression();
static void statement();
static void declaration();
static ParseRule *getRule(TokenType type);
static void parsePrecedence(Precedence precedence);

static uint8_t identifierConstant(Token *name)
{
    return makeConstant(copyString(main_thread, name->start, name->length));
}

static bool identifiersEqual(Token *a, Token *b)
{
    if (a->length != b->length)
        return false;
    return memcmp(a->start, b->start, a->length) == 0;
}

static int resolveLocal(Compiler *compiler, Token *name)
{
    for (int i = compiler->localCount - 1; i >= 0; i--)
    {
        Local *local = &compiler->locals[i];
        if (identifiersEqual(name, &local->name))
        {
            if (local->depth == UNINITIALIZED_LOCAL_SCOPE)
            {
                error("Cannot read local variable in its own initializer.");
            }
            return i;
        }
    }

    return UNRESOLVED_VARIABLE_INDEX;
}

static int addUpvalue(Compiler *compiler, uint8_t index, bool isLocal)
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
        error("Too many closure variables in function.");
        return 0;
    }

    compiler->upvalues[upvalueCount].isLocal = isLocal;
    compiler->upvalues[upvalueCount].index = index;
    return compiler->function->upvalueCount++;
}

static int resolveUpvalue(Compiler *compiler, Token *name)
{
    if (compiler->enclosing == NULL)
        return UNRESOLVED_VARIABLE_INDEX;

    int local = resolveLocal(compiler->enclosing, name);
    if (local != UNRESOLVED_VARIABLE_INDEX)
    {
        compiler->enclosing->locals[local].isCaptured = true;
        return addUpvalue(compiler, (uint8_t)local, true);
    }

    int upvalue = resolveUpvalue(compiler->enclosing, name);
    if (upvalue != UNRESOLVED_VARIABLE_INDEX)
    {
        return addUpvalue(compiler, (uint8_t)upvalue, false);
    }

    return UNRESOLVED_VARIABLE_INDEX;
}

static void addLocal(Token name)
{
    if (current->localCount == MAX_VAR_COUNT)
    {
        error("Too many local variables in function.");
        return;
    }
    Local *local = &current->locals[current->localCount++];
    local->name = name;
    local->depth = UNINITIALIZED_LOCAL_SCOPE;
    local->isCaptured = false;
}

static void declareVariable()
{
    // Global variables are implicitly declared.
    if (current->scopeDepth == GLOBAL_SCOPE)
        return;

    Token *name = &parser.previous;
    for (int i = current->localCount - 1; i >= 0; i--)
    {
        Local *local = &current->locals[i];
        if (local->depth != UNINITIALIZED_LOCAL_SCOPE && local->depth < current->scopeDepth)
        {
            break;
        }

        if (identifiersEqual(name, &local->name))
        {
            error("Variable with this name already declared in this scope.");
        }
    }
    addLocal(*name);
}

static uint8_t parseVariable(const char *errorMessage)
{
    consume(TOKEN_IDENTIFIER, errorMessage);

    declareVariable();
    if (current->scopeDepth > GLOBAL_SCOPE)
        return 0;

    return identifierConstant(&parser.previous);
}

static void markInitialized()
{
    if (current->scopeDepth == GLOBAL_SCOPE)
        return;
    current->locals[current->localCount - 1].depth = current->scopeDepth;
}

static void defineVariable(uint8_t global)
{
    if (current->scopeDepth > GLOBAL_SCOPE)
    {
        markInitialized();
        return;
    }
    emitBytes(OP_DEFINE_GLOBAL, global);
}

static uint8_t argumentList(TokenType closingToken)
{
    uint8_t argCount = 0;
    if (!check(closingToken))
    {
        do
        {
            expression();
            if (argCount == 255)
            {
                error("Cannot have more than 255 arguments.");
            }
            argCount++;
        } while (match(TOKEN_COMMA));
    }

    // Need to sort out a string for the closing token
    consume(closingToken, "Expect ')' after arguments.");
    return argCount;
}

static void and_(bool UNUSED(canAssign))
{
    int endJump = emitJump(OP_JUMP_IF_FALSE);

    emitByte(OP_POP);
    parsePrecedence(PREC_AND);

    patchJump(endJump);
}

static void binary(bool UNUSED(canAssign))
{
    // Remember the operator.
    TokenType operatorType = parser.previous.type;

    // Compile the right operand.
    ParseRule *rule = getRule(operatorType);
    parsePrecedence((Precedence)(rule->precedence + 1));

    // Emit the operator instruction.
    switch (operatorType)
    {
    case TOKEN_BANG_EQUAL:
        emitBytes(OP_EQUAL, OP_NOT);
        break;
    case TOKEN_EQUAL_EQUAL:
        emitByte(OP_EQUAL);
        break;
    case TOKEN_GREATER:
        emitByte(OP_GREATER);
        break;
    case TOKEN_GREATER_EQUAL:
        emitBytes(OP_LESS, OP_NOT);
        break;
    case TOKEN_LESS:
        emitByte(OP_LESS);
        break;
    case TOKEN_LESS_EQUAL:
        emitBytes(OP_GREATER, OP_NOT);
        break;
    case TOKEN_PLUS:
        emitByte(OP_ADD);
        break;
    case TOKEN_MINUS:
        emitByte(OP_SUBTRACT);
        break;
    case TOKEN_STAR:
        emitByte(OP_MULTIPLY);
        break;
    case TOKEN_SLASH:
        emitByte(OP_DIVIDE);
        break;
    default:
        return; // Unreachable.
    }
}

static void call(bool UNUSED(canAssign))
{
    uint8_t argCount = argumentList(TOKEN_RIGHT_PAREN);
    emitBytes(OP_CALL, argCount);
}

static void dot(bool canAssign)
{
    consume(TOKEN_IDENTIFIER, "Expect property name after '.'.");
    uint8_t name = identifierConstant(&parser.previous);

    if (canAssign && match(TOKEN_EQUAL))
    {
        expression();
        emitBytes(OP_SET_PROPERTY, name);
    }
    else if (match(TOKEN_LEFT_PAREN))
    {
        uint8_t argCount = argumentList(TOKEN_RIGHT_PAREN);
        emitBytes(OP_INVOKE, name);
        emitByte(argCount);
    }
    else
    {
        emitBytes(OP_GET_PROPERTY, name);
    }
}

static void literal(bool UNUSED(canAssign))
{
    switch (parser.previous.type)
    {
    case TOKEN_FALSE:
        emitByte(OP_FALSE);
        break;
    case TOKEN_NIL:
        emitByte(OP_NIL);
        break;
    case TOKEN_TRUE:
        emitByte(OP_TRUE);
        break;
    default:
        return; // Unreachable.
    }
}

static void grouping(bool UNUSED(canAssign))
{
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

static void number(bool UNUSED(canAssign))
{
    double value = strtod(parser.previous.start, NULL);
    emitConstant(NUMBER_VAL(value));
}

static void or_(bool UNUSED(canAssign))
{
    int elseJump = emitJump(OP_JUMP_IF_FALSE);
    int endJump = emitJump(OP_JUMP);

    patchJump(elseJump);
    emitByte(OP_POP);

    parsePrecedence(PREC_OR);
    patchJump(endJump);
}

static void string(bool UNUSED(canAssign))
{
    emitConstant(copyString(main_thread, parser.previous.start + 1,
                            parser.previous.length - 2));
}

static void namedVariable(Token name, bool canAssign)
{
    uint8_t getOp, setOp;
    int arg = resolveLocal(current, &name);
    if (arg != UNRESOLVED_VARIABLE_INDEX)
    {
        getOp = OP_GET_LOCAL;
        setOp = OP_SET_LOCAL;
    }
    else if ((arg = resolveUpvalue(current, &name)) != UNRESOLVED_VARIABLE_INDEX)
    {
        getOp = OP_GET_UPVALUE;
        setOp = OP_SET_UPVALUE;
    }
    else
    {
        arg = identifierConstant(&name);
        getOp = OP_GET_GLOBAL;
        setOp = OP_SET_GLOBAL;
    }

    if (canAssign && match(TOKEN_EQUAL))
    {
        expression();
        emitBytes(setOp, (uint8_t)arg);
    }
    else
    {
        emitBytes(getOp, (uint8_t)arg);
    }
}

static void variable(bool canAssign)
{
    namedVariable(parser.previous, canAssign);
}

static Token syntheticToken(const char *text)
{
    Token token;
    token.start = text;
    token.length = (int)strlen(text);
    return token;
}

static void pushSuperclass()
{
    if (currentClass == NULL)
        return;
    namedVariable(syntheticToken("super"), false);
}

static void super_(bool UNUSED(canAssign))
{
    if (currentClass == NULL)
    {
        error("Cannot use 'super' outside of a class.");
    }
    else if (!currentClass->hasSuperclass)
    {
        error("Cannot use 'super' in a class with no superclass.");
    }

    consume(TOKEN_DOT, "Expect '.' after 'super'.");
    consume(TOKEN_IDENTIFIER, "Expect superclass method name.");
    uint8_t name = identifierConstant(&parser.previous);

    // Push the receiver.
    namedVariable(syntheticToken("self"), false);

    if (match(TOKEN_LEFT_PAREN))
    {
        uint8_t argCount = argumentList(TOKEN_RIGHT_PAREN);

        pushSuperclass();
        emitBytes(OP_SUPER, argCount);
        emitByte(name);
    }
    else
    {
        pushSuperclass();
        emitBytes(OP_GET_SUPER, name);
    }
}

static void self(bool UNUSED(canAssign))
{
    if (currentClass == NULL)
    {
        error("Cannot use 'self' outside of a class.");
    }
    else
    {
        variable(false);
    }
}

static void unary(bool UNUSED(canAssign))
{
    TokenType operatorType = parser.previous.type;

    // Compile the operand.
    parsePrecedence(PREC_UNARY);

    // Emit the operator instruction.
    switch (operatorType)
    {
    case TOKEN_BANG:
        emitByte(OP_NOT);
        break;
    case TOKEN_MINUS:
        emitByte(OP_NEGATE);
        break;
    default:
        return; // Unreachable.
    }
}

static void literal_hash(bool canAssign)
{
    consume(TOKEN_RIGHT_BRACE, "Expected '}' for a literal hash declaration");
    namedVariable(syntheticToken("Hash"), canAssign);
    emitBytes(OP_CALL, 0);
}

static void literal_list(bool canAssign)
{
    namedVariable(syntheticToken("List"), canAssign);
    emitBytes(OP_CALL, 0); // Create a list using the default constructor
    emitByte(OP_DUP_TOP); // Duplicate the list instance, so the pop leaves it for the return value of assignment
    uint8_t argCount = argumentList(TOKEN_RIGHT_SQ_BRACKET);
    Token addToken = syntheticToken("add");
    uint8_t name = identifierConstant(&addToken);
    emitBytes(OP_INVOKE, name);
    emitByte(argCount);
    emitByte(OP_POP);
}

static void subscript(bool UNUSED(canAssign))
{
    uint8_t argCount = argumentList(TOKEN_RIGHT_SQ_BRACKET);
    emitBytes(OP_INDEX, argCount);
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
    {unary, NULL, PREC_NONE},        // TOKEN_BANG
    {NULL, binary, PREC_EQUALITY},   // TOKEN_BANG_EQUAL
    {NULL, NULL, PREC_NONE},         // TOKEN_EQUAL
    {NULL, binary, PREC_EQUALITY},   // TOKEN_EQUAL_EQUAL
    {NULL, binary, PREC_COMPARISON}, // TOKEN_GREATER
    {NULL, binary, PREC_COMPARISON}, // TOKEN_GREATER_EQUAL
    {NULL, binary, PREC_COMPARISON}, // TOKEN_LESS
    {NULL, binary, PREC_COMPARISON}, // TOKEN_LESS_EQUAL
    {variable, NULL, PREC_NONE},     // TOKEN_IDENTIFIER
    {string, NULL, PREC_NONE},       // TOKEN_STRING
    {number, NULL, PREC_NONE},       // TOKEN_NUMBER
    {NULL, and_, PREC_AND},          // TOKEN_AND
    {NULL, NULL, PREC_NONE},         // TOKEN_CLASS
    {NULL, NULL, PREC_NONE},         // TOKEN_ELSE
    {NULL, NULL, PREC_NONE},         // TOKEN_ENUM
    {literal, NULL, PREC_NONE},      // TOKEN_FALSE
    {NULL, NULL, PREC_NONE},         // TOKEN_FOR
    {NULL, NULL, PREC_NONE},         // TOKEN_FUN
    {NULL, NULL, PREC_NONE},         // TOKEN_IF
    {NULL, NULL, PREC_NONE},         // TOKEN_IN
    {NULL, NULL, PREC_NONE},         // TOKEN_INSTANCEOF
    {literal, NULL, PREC_NONE},      // TOKEN_NIL
    {NULL, NULL, PREC_NONE},         // TOKEN_OPERATOR
    {NULL, or_, PREC_OR},            // TOKEN_OR
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

static void parsePrecedence(Precedence precedence)
{
    advance();
    ParseFn prefixRule = getRule(parser.previous.type)->prefix;
    if (prefixRule == NULL)
    {
        error("Expect expression.");
        return;
    }

    bool canAssign = precedence <= PREC_ASSIGNMENT;
    prefixRule(canAssign);

    while (precedence <= getRule(parser.current.type)->precedence)
    {
        advance();
        ParseFn infixRule = getRule(parser.previous.type)->infix;
        infixRule(canAssign);
    }

    if (canAssign && match(TOKEN_EQUAL))
    {
        error("Invalid assignment target.");
    }
}

static ParseRule *getRule(TokenType type)
{
    return &rules[type];
}

void expression()
{
    parsePrecedence(PREC_ASSIGNMENT);
}

static void block()
{
    while (!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF))
    {
        declaration();
    }

    match(TOKEN_EOL); // optional end of line.
    consume(TOKEN_RIGHT_BRACE, "Expect '}' after block.");
}

static void function(FunctionType type)
{
    Compiler compiler;
    initCompiler(&compiler, type);
    beginScope();

    // Compile the parameter list.
    consume(TOKEN_LEFT_PAREN, "Expect '(' after function name.");
    if (!check(TOKEN_RIGHT_PAREN))
    {
        do
        {
            current->function->arity++;
            if (current->function->arity > 255)
            {
                errorAtCurrent("Cannot have more than 255 parameters.");
            }

            uint8_t paramConstant = parseVariable("Expect parameter name.");
            defineVariable(paramConstant);
        } while (match(TOKEN_COMMA));
    }
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after parameters.");

    // The body.
    match(TOKEN_EOL); // optional end of line.
    consume(TOKEN_LEFT_BRACE, "Expect '{' before function body.");
    block();

    // Create the function object.
    ObjFunction *function = endCompiler();
    emitBytes(OP_CLOSURE, makeConstant(OBJ_VAL(function)));

    for (int i = 0; i < function->upvalueCount; i++)
    {
        emitByte(compiler.upvalues[i].isLocal ? 1 : 0);
        emitByte(compiler.upvalues[i].index);
    }
}

static void method()
{
    bool isStatic = match(TOKEN_STATIC);

    consume(TOKEN_IDENTIFIER, "Expect method name.");
    uint8_t constant = identifierConstant(&parser.previous);

    // If the method is named "init", it's an initializer.
    FunctionType type = TYPE_METHOD;
    if (parser.previous.length == 4 &&
        memcmp(parser.previous.start, "init", 4) == 0)
    {
        if (isStatic)
            error("Initializer can't be declared static");
        type = TYPE_INITIALIZER;
    }

    function(type);

    if (isStatic)
    {
        emitBytes(OP_STATIC_METHOD, constant);
    }
    else
    {
        emitBytes(OP_METHOD, constant);
    }
}

static void operator()
{
    OPERATOR op = getOperatorFromToken(parser.current.type);
    if (op == OPERATOR_UNKNOWN)
    {
        error("Unsupported operator for overloading");
    }
    advance();
    if (op == OPERATOR_INDEX)
    {
        consume(TOKEN_RIGHT_SQ_BRACKET, "expected ']'");
        if (match(TOKEN_EQUAL))
        {
            op = OPERATOR_INDEX_ASSIGN;
        }
    }
    // For all intents and purposes, this is a method.
    function(TYPE_METHOD);
    emitBytes(OP_DEFINE_OPERATOR, op);
}

static void classDeclaration()
{
    consume(TOKEN_IDENTIFIER, "Expect class name.");
    Token className = parser.previous;
    uint8_t nameConstant = identifierConstant(&parser.previous);
    declareVariable();

    emitBytes(OP_CLASS, nameConstant);
    defineVariable(nameConstant);

    ClassCompiler classCompiler;
    classCompiler.name = parser.previous;
    classCompiler.hasSuperclass = false;
    classCompiler.enclosing = currentClass;
    currentClass = &classCompiler;

    if (match(TOKEN_COLON))
    {
        consume(TOKEN_IDENTIFIER, "Expect superclass name.");
        variable(false);

        if (identifiersEqual(&className, &parser.previous))
        {
            error("A class cannot inherit from itself.");
        }
        beginScope();
        variable(false);
    }
    else
    {
        beginScope();
        namedVariable(syntheticToken("Object"), false);
    }

    // Store the superclass in a local variable named "super".
    addLocal(syntheticToken("super"));
    defineVariable(0);

    namedVariable(className, false);
    emitByte(OP_INHERIT);
    classCompiler.hasSuperclass = true;

    namedVariable(className, false);
    match(TOKEN_EOL); // optional end of line.
    consume(TOKEN_LEFT_BRACE, "Expect '{' before class body.");
    while (!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF))
    {
        if (match(TOKEN_EOL))
        {
            // do nothing.
        }
        else if (match(TOKEN_OPERATOR))
        {
            operator();
        }
        else
        {
            method();
        }
    }

    consume(TOKEN_RIGHT_BRACE, "Expect '}' after class body.");
    emitByte(OP_POP);

    if (classCompiler.hasSuperclass)
    {
        endScope();
    }

    currentClass = currentClass->enclosing;
}

static void funDeclaration()
{
    uint8_t global = parseVariable("Expect function name.");
    markInitialized();
    function(TYPE_FUNCTION);
    defineVariable(global);
}

static void enumDeclaration()
{
    consume(TOKEN_IDENTIFIER, "Expect enum name.");
    uint8_t enumName = identifierConstant(&parser.previous);
    declareVariable();

    emitBytes(OP_ENUM, enumName);
    defineVariable(enumName);

    match(TOKEN_EOL); // optional end of line.
    consume(TOKEN_LEFT_BRACE, "Expect '{' before enum body.");
    while (!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF))
    {
        advance();
    }
    consume(TOKEN_RIGHT_BRACE, "Expect '}' after enum body.");
}

static void varDeclaration()
{
    uint8_t global = parseVariable("Expect variable name.");

    if (match(TOKEN_EQUAL))
    {
        expression();
    }
    else
    {
        emitByte(OP_NIL);
    }
    if (!check(TOKEN_EOF))
        consume(TOKEN_EOL, "Only one statement per line allowed");

    defineVariable(global);
}

static void expressionStatement()
{
    expression();
    if (!check(TOKEN_EOF))
        consume(TOKEN_EOL, "Only one statement per line allowed");
    emitByte(OP_POP);
}

static void forStatement()
{
    beginScope();
    consume(TOKEN_LEFT_PAREN, "Expect '(' after 'for'.");
    if (match(TOKEN_SEMI_COLON))
    {
        // No initializer.
    }
    else if (match(TOKEN_VAR))
    {
        varDeclaration();
    }
    else
    {
        expressionStatement();
    }

    int loopStart = currentChunk()->count;

    int exitJump = -1;
    if (!match(TOKEN_SEMI_COLON))
    {
        expression();
        consume(TOKEN_SEMI_COLON, "Expect ';' after loop condition.");

        // Jump out of the loop if the condition is false.
        exitJump = emitJump(OP_JUMP_IF_FALSE);
        emitByte(OP_POP); // Condition.
    }
    if (!match(TOKEN_RIGHT_PAREN))
    {
        int bodyJump = emitJump(OP_JUMP);

        int incrementStart = currentChunk()->count;
        expression();
        emitByte(OP_POP);
        consume(TOKEN_RIGHT_PAREN, "Expect ')' after for clauses.");

        emitLoop(loopStart);
        loopStart = incrementStart;
        patchJump(bodyJump);
    }

    statement();

    emitLoop(loopStart);

    if (exitJump != -1)
    {
        patchJump(exitJump);
        emitByte(OP_POP); // Condition.
    }

    endScope();
}

static void ifStatement()
{
    consume(TOKEN_LEFT_PAREN, "Expect '(' after 'if'.");
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after condition.");

    int thenJump = emitJump(OP_JUMP_IF_FALSE);
    emitByte(OP_POP);
    statement();

    int elseJump = emitJump(OP_JUMP);
    patchJump(thenJump);
    emitByte(OP_POP);

    if (match(TOKEN_ELSE))
        statement();
    patchJump(elseJump);
}

static void returnStatement()
{
    if (current->type == TYPE_SCRIPT)
    {
        error("Cannot return from top-level code.");
    }
    if (match(TOKEN_EOL))
    {
        emitReturn();
    }
    else
    {
        if (current->type == TYPE_INITIALIZER)
        {
            error("Cannot return a value from an initializer.");
        }
        expression();
        consume(TOKEN_EOL, "Only one statement per line allowed.");
        emitByte(OP_RETURN);
    }
}

static void whileStatement()
{
    int loopStart = currentChunk()->count;
    consume(TOKEN_LEFT_PAREN, "Expect '(' after 'while'.");
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after condition.");

    int exitJump = emitJump(OP_JUMP_IF_FALSE);

    emitByte(OP_POP);
    statement();

    emitLoop(loopStart);

    patchJump(exitJump);
    emitByte(OP_POP);
}

static void throwStatement()
{
    expression();
    if (!check(TOKEN_EOF))
        consume(TOKEN_EOL, "Only one statement per line allowed.");
    emitByte(OP_THROW);
}

static void synchronize()
{
    parser.panicMode = false;

    while (parser.current.type != TOKEN_EOF)
    {
        if (parser.previous.type == TOKEN_EOL)
            return;

        switch (parser.current.type)
        {
        case TOKEN_CLASS:
        case TOKEN_FUN:
        case TOKEN_VAR:
        case TOKEN_OPERATOR:
        case TOKEN_FOR:
        case TOKEN_IF:
        case TOKEN_WHILE:
        case TOKEN_THROW:
        case TOKEN_RETURN:
        case TOKEN_STATIC:
            return;

        default:
            // Do nothing.
            ;
        }

        advance();
    }
}

static void declaration()
{
    if (match(TOKEN_CLASS))
    {
        classDeclaration();
    }
    else if (match(TOKEN_FUN))
    {
        funDeclaration();
    }
    else if (match(TOKEN_ENUM))
    {
        enumDeclaration();
    }
    else if (match(TOKEN_VAR))
    {
        varDeclaration();
    }
    else if (match(TOKEN_EOL))
    {
        // Do nothing, but don't error, this is a blank line
    }
    else if (match(TOKEN_SEMI_COLON))
    {
        error("Unexpected ';'");
    }
    else
    {
        statement();
    }

    if (parser.panicMode)
        synchronize();
}

static void statement()
{
    if (match(TOKEN_FOR))
    {
        forStatement();
    }
    else if (match(TOKEN_IF))
    {
        ifStatement();
    }
    else if (match(TOKEN_RETURN))
    {
        returnStatement();
    }
    else if (match(TOKEN_WHILE))
    {
        whileStatement();
    }
    else if (match(TOKEN_THROW))
    {
        throwStatement();
    }
    else if (match(TOKEN_LEFT_BRACE))
    {
        beginScope();
        block();
        endScope();
    }
    else
    {
        expressionStatement();
    }
}

ObjFunction *compile(const SourceFile *source, VM *thread)
{
    initScanner(source);
    main_thread = thread;

    parser.filename = source->path;
    parser.hadError = false;
    parser.panicMode = false;

    Compiler compiler;
    initCompiler(&compiler, TYPE_SCRIPT);

    // chicken-egg situation.
    advance();
    while (!match(TOKEN_EOF))
    {
        declaration();
    }
    ObjFunction *function = endCompiler();
    return parser.hadError ? NULL : function;
}

void markCompilerRoots(VM *vm)
{
    Compiler *compiler = current;
    while (compiler != NULL)
    {
        markObject(vm, (Obj *)compiler->function);
        compiler = compiler->enclosing;
    }
}
