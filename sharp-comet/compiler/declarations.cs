using sharpcomet.lexer;
using sharpcomet.vmlib;

namespace sharpcomet.compiler;

public partial class Parser
{
    private const int MAX_VAR_COUNT = 255;

    private void DeclareVariable()
    {

    }

    private byte IdentifierConstant(Token token)
    {
        return 0;
    }

    private byte ParseVariable(string errorMessage)
    {
       Consume(TokenType.Identifier, errorMessage);

        DeclareVariable();
        if (CurrentFunction.ScopeDepth > FunctionCompiler.GLOBAL_SCOPE)
            return 0;

        return IdentifierConstant(Previous);
    }

    private void ClassDeclaration(int attributeCount)
    {

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


    private void FunctionDeclaration(int attributeCount)
    {
        var global = ParseVariable("Expected a function name");
        CurrentFunction.MarkInitialized();
        ParseFunction(FunctionType.Function, attributeCount);
        CurrentFunction.DefineVariable(global);
    }

    private void EnumDeclaration()
    {

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
            CurrentFunction.EmitBytes((byte)Op.Nil);
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
            FunctionDeclaration(0);
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