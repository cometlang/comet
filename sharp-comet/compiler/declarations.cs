using sharpcomet.lexer;
using sharpcomet.vmlib;

namespace sharpcomet.compiler;

public partial class Parser
{
    private void DeclareVariable()
    {

    }

    private ushort IdentifierConstant(Token token)
    {
        return 0;
    }

    private ushort ParseVariable(string errorMessage)
    {
       Consume(TokenType.Identifier, errorMessage);

        DeclareVariable();
        if (CurrentFunction.ScopeDepth > Compiler.GLOBAL_SCOPE)
            return 0;

        return IdentifierConstant(Previous);
    }

    private void ClassDeclaration(int attributeCount)
    {

    }

    private void FunctionDeclaration(int attributeCount)
    {

    }

    private void EnumDeclaration()
    {

    }

    private void VarDeclaration()
    {
        ushort global = ParseVariable("Expected a variable name");

        if (Match(TokenType.Equal))
        {
            Expression();
        }
        else
        {
            EmitByte((byte)Op.Nil);
        }

        CurrentFunction.DefineVariable(global);
    }

    public void Declaration()
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