#include <stdlib.h>

#include "comet.h"
#include "constants.h"
#include "emitter.h"
#include "expressions.h"
#include "statements.h"
#include "variables.h"

static uint8_t argumentList(Parser *parser, TokenType_t closingToken)
{
    uint8_t argCount = 0;
    if (!check(parser, closingToken))
    {
        do
        {
            expression(parser);
            if (argCount == MAX_ARGS)
            {
                error(parser, "Cannot have more than 16 arguments.");
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
    TokenType_t operatorType = parser->previous.type;

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
    case TOKEN_BITWISE_AND:
        emitByte(parser, OP_BITWISE_AND);
        break;
    case TOKEN_VBAR:
        emitByte(parser, OP_BITWISE_OR);
        break;
    case TOKEN_BITWISE_XOR:
        emitByte(parser, OP_BITWISE_XOR);
        break;
    case TOKEN_BITSHIFT_RIGHT:
        emitByte(parser, OP_BITSHIFT_RIGHT);
        break;
    case TOKEN_BITSHIFT_LEFT:
        emitByte(parser, OP_BITSHIFT_LEFT);
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
    emitConstant(parser, parseNumber(parser));
}

static void string(Parser *parser, bool UNUSED(canAssign))
{
    emitConstant(parser, parseString(parser));
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
    TokenType_t operatorType = parser->previous.type;

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
    case TOKEN_BITWISE_NEGATE:
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
            if (check(parser, TOKEN_RIGHT_BRACE)) // hanging comma
                break;
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
    if (canAssign && match(parser, TOKEN_EQUAL)) {
        if (argCount != 1) {
            errorAtCurrent(parser, "Eactly 1 index argument is required for assignment");
        } else {
            expression(parser);
            emitBytes(parser, OP_INDEX_ASSIGN, argCount + 1);
        }
    }
    else {
        emitBytes(parser, OP_INDEX, argCount);
    }
}

static void lambda(Parser *parser, bool UNUSED(canAssign))
{
    Compiler compiler;
    initCompiler(&compiler, TYPE_LAMBDA, parser);
    beginScope(parser);

    if (!check(parser, TOKEN_VBAR))
    {
        do
        {
            parser->currentFunction->function->arity++;
            if (parser->currentFunction->function->arity > 255)
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
    // Single-character tokens.
    [TOKEN_LEFT_PAREN]       = {grouping,     call,      PREC_CALL},
    [TOKEN_RIGHT_PAREN]      = {NULL,         NULL,      PREC_NONE},
    [TOKEN_LEFT_BRACE]       = {literal_hash, NULL,      PREC_NONE},
    [TOKEN_RIGHT_BRACE]      = {NULL,         NULL,      PREC_NONE},
    [TOKEN_LEFT_SQ_BRACKET]  = {literal_list, subscript, PREC_CALL},
    [TOKEN_RIGHT_SQ_BRACKET] = {NULL,         NULL,      PREC_NONE},
    [TOKEN_COMMA]            = {NULL,         NULL,      PREC_NONE},
    [TOKEN_DOT]              = {NULL,         dot,       PREC_CALL},
    [TOKEN_MINUS]            = {unary,        binary,    PREC_TERM},
    [TOKEN_PLUS]             = {NULL,         binary,    PREC_TERM},
    [TOKEN_SEMI_COLON]       = {NULL,         NULL,      PREC_NONE},
    [TOKEN_SLASH]            = {NULL,         binary,    PREC_FACTOR},
    [TOKEN_STAR]             = {NULL,         binary,    PREC_FACTOR},
    [TOKEN_COLON]            = {NULL,         NULL,      PREC_NONE},
    [TOKEN_EOL]              = {NULL,         NULL,      PREC_NONE},
    [TOKEN_VBAR]             = {lambda,       binary,    PREC_BITWISE_OR},
    [TOKEN_PERCENT]          = {NULL,         binary,    PREC_FACTOR},
    // One or two character tokens.
    [TOKEN_BANG]             = {unary,        NULL,      PREC_UNARY},
    [TOKEN_BANG_EQUAL]       = {NULL,         binary,    PREC_EQUALITY},
    [TOKEN_EQUAL]            = {NULL,         NULL,      PREC_NONE},
    [TOKEN_EQUAL_EQUAL]      = {NULL,         binary,    PREC_EQUALITY},
    [TOKEN_GREATER]          = {NULL,         binary,    PREC_COMPARISON},
    [TOKEN_GREATER_EQUAL]    = {NULL,         binary,    PREC_COMPARISON},
    [TOKEN_LESS]             = {NULL,         binary,    PREC_COMPARISON},
    [TOKEN_LESS_EQUAL]       = {NULL,         binary,    PREC_COMPARISON},
    [TOKEN_PLUS_EQUAL]       = {NULL,         NULL,      PREC_NONE},
    [TOKEN_MINUS_EQUAL]      = {NULL,         NULL,      PREC_NONE},
    [TOKEN_STAR_EQUAL]       = {NULL,         NULL,      PREC_NONE},
    [TOKEN_SLASH_EQUAL]      = {NULL,         NULL,      PREC_NONE},
    [TOKEN_PERCENT_EQUAL]    = {NULL,         NULL,      PREC_NONE},
    [TOKEN_LOGICAL_OR]       = {NULL,         or_,       PREC_OR},
    [TOKEN_LOGICAL_AND]      = {NULL,         and_,      PREC_AND},
    [TOKEN_BITWISE_AND]      = {NULL,         binary,    PREC_BITWISE_AND},
    [TOKEN_BITWISE_XOR]      = {NULL,         binary,    PREC_XOR},
    [TOKEN_BITWISE_NEGATE]   = {unary,        NULL,      PREC_UNARY},
    [TOKEN_BITSHIFT_LEFT]    = {NULL,         binary,    PREC_BITSHIFT},
    [TOKEN_BITSHIFT_RIGHT]   = {NULL,         binary,    PREC_BITSHIFT},
    // Literals
    [TOKEN_IDENTIFIER]       = {variable,     NULL,      PREC_NONE},
    [TOKEN_STRING]           = {string,       NULL,      PREC_NONE},
    [TOKEN_NUMBER]           = {number,       NULL,      PREC_NONE},
    // Keywords
    [TOKEN_AS]               = {NULL,         NULL,      PREC_NONE},
    [TOKEN_CLASS]            = {NULL,         NULL,      PREC_NONE},
    [TOKEN_ELSE]             = {NULL,         NULL,      PREC_NONE},
    [TOKEN_ENUM]             = {NULL,         NULL,      PREC_NONE},
    [TOKEN_FALSE]            = {literal,      NULL,      PREC_NONE},
    [TOKEN_FOR]              = {NULL,         NULL,      PREC_NONE},
    [TOKEN_FOREACH]          = {NULL,         NULL,      PREC_NONE},
    [TOKEN_FUN]              = {NULL,         NULL,      PREC_NONE},
    [TOKEN_IF]               = {NULL,         NULL,      PREC_NONE},
    [TOKEN_IMPORT]           = {NULL,         NULL,      PREC_NONE},
    [TOKEN_IN]               = {NULL,         NULL,      PREC_NONE},
    [TOKEN_INSTANCEOF]       = {NULL,         binary,    PREC_INSTANCEOF},
    [TOKEN_NEXT]             = {NULL,         NULL,      PREC_NONE},
    [TOKEN_NIL]              = {literal,      NULL,      PREC_NONE},
    [TOKEN_OPERATOR]         = {NULL,         NULL,      PREC_NONE},
    [TOKEN_RETHROW]          = {NULL,         NULL,      PREC_NONE},
    [TOKEN_RETURN]           = {NULL,         NULL,      PREC_NONE},
    [TOKEN_SELF]             = {self,         NULL,      PREC_NONE},
    [TOKEN_SUPER]            = {super_,       NULL,      PREC_NONE},
    [TOKEN_TRUE]             = {literal,      NULL,      PREC_NONE},
    [TOKEN_VAR]              = {NULL,         NULL,      PREC_NONE},
    [TOKEN_WHILE]            = {NULL,         NULL,      PREC_NONE},
    [TOKEN_TRY]              = {NULL,         NULL,      PREC_NONE},
    [TOKEN_CATCH]            = {NULL,         NULL,      PREC_NONE},
    [TOKEN_THROW]            = {NULL,         NULL,      PREC_NONE},
    [TOKEN_FINAL]            = {NULL,         NULL,      PREC_NONE},
    [TOKEN_FINALLY]          = {NULL,         NULL,      PREC_NONE},
    [TOKEN_PRIVATE]          = {NULL,         NULL,      PREC_NONE},
    [TOKEN_PROTECTED]        = {NULL,         NULL,      PREC_NONE},
    [TOKEN_PUBLIC]           = {NULL,         NULL,      PREC_NONE},
    [TOKEN_STATIC]           = {NULL,         NULL,      PREC_NONE},
    [TOKEN_ERROR]            = {NULL,         NULL,      PREC_NONE},
    [TOKEN_EOF]              = {NULL,         NULL,      PREC_NONE},
};

void parsePrecedence(Parser *parser, Precedence precedence)
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

ParseRule *getRule(TokenType_t type)
{
    return &rules[type];
}

void expression(Parser *parser)
{
    parsePrecedence(parser, PREC_ASSIGNMENT);
}
