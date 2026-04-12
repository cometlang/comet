using sharpcomet.lexer;
using sharpcomet.vmlib;

namespace sharpcomet.compiler;

public partial class Parser
{

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

    private void Statement()
    {
        if (Match(TokenType.LeftBrace))
        {
            CurrentFunction.BeginScope();
            Block();
            CurrentFunction.EndScope();
        }
        else
        {
            ExpressionStatement();
        }
    }
}