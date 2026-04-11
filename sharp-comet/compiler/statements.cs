using sharpcomet.lexer;
using sharpcomet.vmlib;

namespace sharpcomet.compiler;

public partial class Parser
{

    private void Block()
    {

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
        ExpressionStatement();
    }
}