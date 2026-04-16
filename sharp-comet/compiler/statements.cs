using sharpcomet.lexer;
using sharpcomet.vmlib;

namespace sharpcomet.compiler;

public partial class Parser
{
    private void ForStatement()
    {
        CurrentFunction.BeginScope();
        CurrentFunction.EndScope();
    }

    private void IfStatement()
    {
        Consume(TokenType.LeftParen, "Expected '(' after 'if'.");
        Expression();
        Consume(TokenType.RightParen, "Expected ')' after condition.");

        int thenJump = CurrentFunction.EmitJump(Op.JumpIfFalse);
        CurrentFunction.EmitBytes((byte)Op.Pop);
        Statement();

        int elseJump = CurrentFunction.EmitJump(Op.Jump);
        if (!CurrentFunction.PatchJump(thenJump))
        {
            Error("Too much code to jump over!");
        }
        CurrentFunction.EmitBytes((byte)Op.Pop);

        Match(TokenType.EndOfLine);
        if (Match(TokenType.Else))
            Statement();

        if (!CurrentFunction.PatchJump(elseJump))
        {
            Error("Too much code to jump over!");
        }
    }

    private void Block()
    {
        Match(TokenType.EndOfLine);
        while (!Check(TokenType.RightBrace) && !Check(TokenType.EndOfFile))
        {
            Declaration();
        }
        Match(TokenType.EndOfLine);
        Consume(TokenType.RightBrace, "Expected a '}' after a block.");
    }

    private void ExpressionStatement()
    {
        Expression();
        if (!Check(TokenType.EndOfFile) && !Check(TokenType.RightBrace))
        {
            Consume(TokenType.EndOfLine, "Only one statement per line allowed.");
        }
        CurrentFunction.EmitBytes((byte)Op.Pop);
    }

    private void ThrowStatement()
    {
        Expression();
        if (!Check(TokenType.EndOfFile))
            Consume(TokenType.EndOfLine, "Only one statement per line allowed.");
        CurrentFunction.EmitBytes((byte)Op.Throw);
    }

    private void RethrowStatement()
    {
        Expression();
        if (!Check(TokenType.EndOfFile))
            Consume(TokenType.EndOfLine, "Only one statement per line allowed.");
        CurrentFunction.EmitBytes((byte)Op.Rethrow);
    }

    private void ForEachStatement()
    { }

    private void ReturnStatement()
    {
        if (CurrentFunction.IsScript())
        {
            Error("Cannot return from top-level code.");
        }
        if (Match(TokenType.EndOfLine))
        {
            CurrentFunction.EmitReturn();
        }
        else
        {
            if (CurrentFunction.IsInitializer())
            {
                Error("Cannot return from an initializer.");
            }
            Expression();
            if (!Check(TokenType.RightBrace))
            {
                Consume(TokenType.EndOfLine, "Only one statement per line allowed.");
            }
            CurrentFunction.EmitBytes((byte)Op.Return);
        }
    }

    private void WhileStatement()
    { }

    private void TryStatement()
    { }

    private void ImportStatement()
    {
        byte? importParamCount = 0;
        if (Match(TokenType.LeftBrace))
        {
            byte[] moduleIdentifierConstants = new byte[255];
            if (Match(TokenType.Star))
            {
                importParamCount = null;
            }
            else
            {
                do
                {
                    if (importParamCount == 255)
                    {
                        ErrorAtCurrent("Cannot have more than 255 import parameters.");
                    }
                    Match(TokenType.EndOfLine);
                    Consume(TokenType.Identifier, "Expected a module import identifier.");
                    byte paramConstant = IdentifierConstant(Previous);
                    moduleIdentifierConstants[importParamCount.Value] = paramConstant;
                    importParamCount++;

                } while (Match(TokenType.Comma));
            }
            Consume(TokenType.RightBrace, "Expected a '}' after import parameters.");
            Match(TokenType.EndOfLine);
            Consume(TokenType.From, "Expected 'from' after import parameters.");
            Expression();
            // Imports are a function that returns nil, so pop that off the stack, too
            CurrentFunction.EmitBytes((byte)Op.Import, (byte)Op.Pop);
            CurrentFunction.EmitBytes((byte)Op.ImportParams, importParamCount ?? 0);
            if (importParamCount != null)
            {
                CurrentFunction.EmitBytes(moduleIdentifierConstants);
            }
        }
        else
        {
            Expression();
            // Imports are a function that returns nil, so pop that off the stack, too
            CurrentFunction.EmitBytes((byte)Op.Import, (byte)Op.Pop);
            Consume(TokenType.As, "Expected 'as' after the module to import.");
            byte global = ParseVariable("Expected a variable name for the imported module.");
            CurrentFunction.DefineVariable(global);
        }
    }

    private void NextStatement()
    { }

    private void BreakStatement()
    { }

    private void Statement()
    {
        Match(TokenType.EndOfLine);
        if (Match(TokenType.For))
        {
            ForStatement();
        }
        else if (Match(TokenType.ForEach))
        {
            ForEachStatement();
        }
        else if (Match(TokenType.If))
        {
            IfStatement();
        }
        else if (Match(TokenType.Return))
        {
            ReturnStatement();
        }
        else if (Match(TokenType.While))
        {
            WhileStatement();
        }
        else if (Match(TokenType.Try))
        {
            TryStatement();
        }
        else if (Match(TokenType.Rethrow))
        {
            RethrowStatement();
        }
        else if (Match(TokenType.Throw))
        {
            ThrowStatement();
        }
        else if (Match(TokenType.LeftBrace))
        {
            CurrentFunction.BeginScope();
            Block();
            CurrentFunction.EndScope();
        }
        else if (Match(TokenType.Import))
        {
            ImportStatement();
        }
        else if (Match(TokenType.Next))
        {
            NextStatement();
        }
        else if (Match(TokenType.Break))
        {
            BreakStatement();
        }
        else
        {
            ExpressionStatement();
        }
    }
}