namespace sharpcomet.lexer;

public class Token
{
    public Token(TokenType tokenType, string representation, int lineNumber)
    {
        TokenType = tokenType;
        Representation = representation;
        LineNumer = lineNumber;
    }

    public TokenType TokenType { get; }
    public string Representation { get; }
    public int LineNumer { get; }
}