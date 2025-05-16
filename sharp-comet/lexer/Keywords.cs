using sharpcomet.lexer;

namespace lexer
{
    public static class Keywords
    {
        public static TokenType CheckKeyword(string identifier)
        {
            return TokenType.Identifier;
        }
    }
}
