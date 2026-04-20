using sharpcomet.lexer;
using sharpcomet.vmlib;
using System;

namespace sharpcomet.compiler;

public partial class Parser
{
    private const int MAX_VAR_COUNT = 255;

    //void declareVariable(Parser* parser)
    //{
    //    // Global variables are implicitly declared.
    //    if (parser->currentFunction->scopeDepth == GLOBAL_SCOPE)
    //        return;

    //    Token* name = &parser->previous;
    //    for (int i = parser->currentFunction->localCount - 1; i >= 0; i--)
    //    {
    //        Local* local = &parser->currentFunction->locals[i];
    //        if (local->depth != UNINITIALIZED_SCOPE && local->depth < parser->currentFunction->scopeDepth)
    //        {
    //            addLocal(parser, *name);
    //            return;
    //        }

    //        if (identifiersEqual(name, &local->name))
    //        {
    //            error(parser, "Variable with this name already declared in this scope.");
    //            return;
    //        }
    //    }
    //}
    private void DeclareVariable()
    {
        // Global variables are implicitly declared
        if (CurrentFunction.ScopeDepth == FunctionCompiler.GLOBAL_SCOPE)
            return;

        throw new NotImplementedException();
    }

    private byte IdentifierConstant(Token token)
    {
        return CurrentFunction.MakeConstant(Globals.InternString(token.Representation));
    }

    private byte ParseVariable(string errorMessage)
    {
       Consume(TokenType.Identifier, errorMessage);

        DeclareVariable();
        if (CurrentFunction.ScopeDepth > FunctionCompiler.GLOBAL_SCOPE)
            return 0;

        return IdentifierConstant(Previous);
    }

    private void Operator()
    {
        throw new NotImplementedException();
    }

    private void Method(byte attributeCount)
    {
        bool isStatic = Match(TokenType.Static);
        Consume(TokenType.Identifier, "Expected a method name");
        byte constant = IdentifierConstant(Previous);

        // If the method is named "init", then it's an initializer.
        FunctionType type = FunctionType.Method;
        if (Previous.Representation == "init")
        {
            if (isStatic)
            {
                Error("Initializers can't be declared static");
            }
            type = FunctionType.Initializer;
        }

        FunctionDeclaration(type, attributeCount);

        if (isStatic)
            CurrentFunction.EmitBytes((byte)Op.StaticMethod, constant);
        else
            CurrentFunction.EmitBytes((byte)Op.Method, constant);
    }

    private void ClassDeclaration(byte attributeCount)
    {
        bool isFinal = Match(TokenType.Final);

        Consume(TokenType.Identifier, "Expected a class name.");
        Token className = Previous;
        byte nameConstant = IdentifierConstant(Previous);
        DeclareVariable();

        CurrentFunction.EmitBytes((byte)Op.Class, nameConstant, (byte)(isFinal ? 1 : 0), attributeCount);
        CurrentClass = new ClassCompiler(Previous, CurrentClass);
        if (Match(TokenType.Colon))
        {
            Consume(TokenType.Identifier, "Expected a class name to inherit.");
            if (className.Representation == Previous.Representation)
            {
                Error("A class cannot inherit from itself.");
            }

            Variable(false);
            if (Match(TokenType.Dot) && Match(TokenType.Identifier))
            {
                byte name = IdentifierConstant(Previous);
                CurrentFunction.EmitBytes((byte)Op.GetProperty, name);
            }
        }
        else
        {
            NamedVariable(SyntheticToken("Object"), false);
        }
        NamedVariable(className, false);
        CurrentFunction.EmitBytes(Op.Inherit);

        CurrentFunction.BeginScope();
        byte local = CurrentFunction.AddLocal("super");
        CurrentFunction.DefineVariable(local);

        NamedVariable(className, false);
        Match(TokenType.EndOfLine); // optional newline before class body
        Consume(TokenType.LeftBrace, "Expected a '{' for the class body.");
        while(!Check(TokenType.RightBrace) && !Check(TokenType.EndOfFile))
        {
            if (Match(TokenType.EndOfLine))
            {
                // Do Nothing, it's fine to have whitespace inside a class
            }
            else if (Match(TokenType.Operator))
            {
                Operator();
            }
            else if (Match(TokenType.AtSymbol))
            {
                Expression();
            }
            else
            {
                Method(0);
            }
        }

        Consume(TokenType.RightBrace, "Expected a '}' after the class body.");
        CurrentFunction.EmitBytes(Op.Pop);

        // Not sure why, but I need attributeCount-1 to pop off the stack
        for (int i = 1; i < attributeCount; i++)
        {
            CurrentFunction.EmitBytes(Op.Pop);
        }

        CurrentFunction.EndScope();
        CurrentClass = CurrentClass.Enclosing;
    }

    //     void function(Parser *parser, FunctionType type, uint8_t attributeCount)
    // {
    //     Compiler compiler;
    //     initCompiler(&compiler, type, parser);
    //     beginScope(parser);

    //     // Compile the parameter list.
    //     consume(parser, TOKEN_LEFT_PAREN, "Expect '(' after function name.");
    //     if (!check(parser, TOKEN_RIGHT_PAREN))
    //     {
    //         bool startedOptionals = false;
    //         do
    //         {
    //             match(parser, TOKEN_EOL);
    //             parser->currentFunction->function->arity++;
    //             if (parser->currentFunction->function->arity > MAX_VAR_COUNT)
    //             {
    //                 errorAtCurrent(parser, "Cannot have more than 255 parameters.");
    //             }

    //             if (parser->currentFunction->function->restParam)
    //             {
    //                 errorAtCurrent(parser, "Cannot have further parameters after a *parameter declaration.");
    //             }

    //             if (match(parser, TOKEN_STAR))
    //             {
    //                 parser->currentFunction->function->restParam = true;
    //             }

    //             uint8_t paramConstant = parseVariable(parser, "Expected a parameter name.");
    //             defineVariable(parser, paramConstant);
    //             if (match(parser, TOKEN_EQUAL))
    //             {
    //                 startedOptionals = true;
    //                 defaultParameter(parser);
    //             }
    //             else if (startedOptionals)
    //             {
    //                 errorAtCurrent(parser, "Non-optional parameter encountered after an optional one");
    //             }
    //         } while (match(parser, TOKEN_COMMA));
    //     }
    //     consume(parser, TOKEN_RIGHT_PAREN, "Expect ')' after parameters.");

    //     // The body.
    //     match(parser, TOKEN_EOL);
    //     consume(parser, TOKEN_LEFT_BRACE, "Expect '{' before function body.");
    //     block(parser);
    // }

    private void ParseFunction(FunctionType functionType, int attributeCount)
    {
        CurrentFunction = new FunctionCompiler(CurrentFunction, functionType);
        CurrentFunction.BeginScope();

        // argument list
        Consume(TokenType.LeftParen, "Expected '(' after the function name");
        if (!Check(TokenType.RightParen))
        {
            do
            {
                Match(TokenType.EndOfLine);
                CurrentFunction.Function.Arity++;
                if (CurrentFunction.Function.Arity > MAX_VAR_COUNT)
                {
                    ErrorAtCurrent($"Cannot have more than {MAX_VAR_COUNT} function parameters");
                }

                var param = ParseVariable("Expected a parameter name");
                CurrentFunction.DefineVariable(param);

            } while (Match(TokenType.Comma));
        }

        // body
        Match(TokenType.EndOfLine);
        Consume(TokenType.LeftBrace, "Expected '{' before the function body");
        Block();

        // emit
        CurrentFunction.EndCompiler(true);
        CurrentFunction = CurrentFunction.Enclosing;
    }


    private void FunctionDeclaration(FunctionType type, int attributeCount)
    {
        var global = ParseVariable("Expected a function name");
        CurrentFunction.MarkInitialized();
        ParseFunction(FunctionType.Function, attributeCount);
        CurrentFunction.DefineVariable(global);
    }

    private void EnumDeclaration()
    {
        throw new NotImplementedException();
    }

    private void VarDeclaration()
    {
        byte global = ParseVariable("Expected a variable name");

        if (Match(TokenType.Equal))
        {
            Expression();
        }
        else
        {
            CurrentFunction.EmitBytes(Op.Nil);
        }

        CurrentFunction.DefineVariable(global);
    }

    private void Declaration()
    {
        if (Match(TokenType.Class))
        {
            ClassDeclaration(0);
        }
        else if (Match(TokenType.Function))
        {
            FunctionDeclaration(FunctionType.Function, 0);
        }
        else if (Match(TokenType.Enum))
        {
            EnumDeclaration();
        }
        else if (Match(TokenType.Var))
        {
            VarDeclaration();
            if (!Check(TokenType.EndOfFile))
                Consume(TokenType.EndOfLine, "Only one statement per line allowed");
        }
        else if (Match(TokenType.EndOfLine))
        {
            // Do nothing, but don't error, this is a blank line
        }
        else if (Match(TokenType.SemiColon))
        {
            Error("Unexpected ';'");
        }
        else
        {
            Statement();
        }

        if (_panicMode)
        {
            Synchronize();
        }
    }

}