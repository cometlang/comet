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

    private void Statement()
    {
        Match(TokenType.EndOfLine);
        if (Match(TokenType.For))
        {
            ForStatement();
        }
        else if (Match(TokenType.ForEach))
        {

        }
        else if (Match(TokenType.If))
        {
            IfStatement();
        }
        else if (Match(TokenType.Return))
        {

        }
        else if (Match(TokenType.While))
        {

        }
        else if (Match(TokenType.Try))
        {

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

        }
        else if (Match(TokenType.Next))
        {

        }
        else if (Match(TokenType.Break))
        {

        }
        else
        {
            ExpressionStatement();
        }
    }
}