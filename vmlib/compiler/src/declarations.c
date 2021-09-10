#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "comet.h"
#include "constants.h"
#include "declarations.h"
#include "expressions.h"
#include "statements.h"
#include "emitter.h"
#include "variables.h"

static void defineParameterValue(Parser *parser, uint16_t value)
{
    ObjFunction *func = parser->currentFunction->function;
    if (func->optionalArgCount == MAX_OPTIONAL_ARGS)
    {
        errorAtCurrent(parser, "Too many optional arguments in one function");
    }
    func->optionalArguments[func->optionalArgCount++] = value;
}

static void defaultParameter(Parser *parser)
{
    uint16_t constantVal;
    if (match(parser, TOKEN_LEFT_SQ_BRACKET))
    {
        if (!match(parser, TOKEN_RIGHT_SQ_BRACKET))
        {
            error(parser, "A default parameter list must be empty");
            return;
        }
        constantVal = NEW_LIST_PARAM_VALUE;
    }
    else if (match(parser, TOKEN_LEFT_BRACE))
    {
        constantVal = NEW_HASH_PARAM_VALUE;
        if (!match(parser, TOKEN_RIGHT_BRACE))
        {
            error(parser, "A default parameter hash must be empty");
            return;
        }
    }
    else if (match(parser, TOKEN_STRING))
    {
        constantVal = makeConstant(parser, parseString(parser));
    }
    else if (match(parser, TOKEN_NUMBER))
    {
        constantVal = makeConstant(parser, parseNumber(parser));
    }
    else if (match(parser, TOKEN_TRUE))
    {
        constantVal = makeConstant(parser, TRUE_VAL);
    }
    else if (match(parser, TOKEN_FALSE))
    {
        constantVal = makeConstant(parser, FALSE_VAL);
    }
    else if (match(parser, TOKEN_NIL))
    {
        constantVal = makeConstant(parser, NIL_VAL);
    }
    else
    {
        errorAtCurrent(parser, "A default parameter can only be a string, number, nil, true/false, [] or {}");
        return;
    }
    defineParameterValue(parser, constantVal);
}

void method(Parser *parser)
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

void operator_(Parser *parser)
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

void classDeclaration(Parser *parser)
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
        beginScope(parser);
        variable(parser, false);
    }
    else
    {
        beginScope(parser);
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
            operator_(parser);
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

void function(Parser *parser, FunctionType type)
{
    Compiler compiler;
    initCompiler(&compiler, type, parser);
    beginScope(parser);

    // Compile the parameter list.
    consume(parser, TOKEN_LEFT_PAREN, "Expect '(' after function name.");
    if (!check(parser, TOKEN_RIGHT_PAREN))
    {
        bool startedOptionals = false;
        do
        {
            parser->currentFunction->function->arity++;
            if (parser->currentFunction->function->arity > MAX_VAR_COUNT)
            {
                errorAtCurrent(parser, "Cannot have more than 255 parameters.");
            }

            uint8_t paramConstant = parseVariable(parser, "Expect parameter name.");
            defineVariable(parser, paramConstant);
            if (match(parser, TOKEN_EQUAL))
            {
                startedOptionals = true;
                defaultParameter(parser);
            }
            else if (startedOptionals)
            {
                errorAtCurrent(parser, "Non-optional parameter encountered after an optional one");
            }
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

void functionDeclaration(Parser *parser)
{
    uint8_t global = parseVariable(parser, "Expect function name.");
    markInitialized(parser);
    function(parser, TYPE_FUNCTION);
    defineVariable(parser, global);
}

void enumDeclaration(Parser *parser)
{
    uint8_t enumName = parseVariable(parser, "Expect enum name");
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
            emitConstant(parser, create_number(parser->compilation_thread, current_value));

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

void varDeclaration(Parser *parser)
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

void declaration(Parser *parser)
{
    if (match(parser, TOKEN_CLASS))
    {
        classDeclaration(parser);
    }
    else if (match(parser, TOKEN_FUN))
    {
        functionDeclaration(parser);
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
