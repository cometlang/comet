using sharpcomet.lexer;

namespace lexer
{
    public static class Keywords
    {
        private static TokenType MatchKeyword(string identifier, string keyword, TokenType keywordType)
        {
            if (identifier == keyword)
                return keywordType;
            return TokenType.Identifier;
        }

        public static TokenType CheckKeyword(string identifier)
        {
            switch (identifier[0])
            {
                case 'a':
                    return MatchKeyword(identifier, "as", TokenType.As);
                case 'b':
                    return MatchKeyword(identifier, "break", TokenType.Break);
                case 'c':
                    if (identifier.Length > 1)
                    {
                        switch (identifier[1])
                        {
                            case 'a':
                                return MatchKeyword(identifier, "catch", TokenType.Catch);
                            case 'l':
                                return MatchKeyword(identifier, "class", TokenType.Class);
                        }
                    }
                    break;
                case 'e':
                    if (identifier.Length > 1)
                    {
                        switch (identifier[1])
                        {
                            case 'l':
                                return MatchKeyword(identifier, "else", TokenType.Else);
                            case 'n':
                                return MatchKeyword(identifier, "enum", TokenType.Enum);
                        }
                    }
                    break;
                case 'f':
                    if (identifier.Length > 1)
                    {
                        switch (identifier[1])
                        {
                            case 'a':
                                return MatchKeyword(identifier, "false", TokenType.False);
                            case 'i':
                                {
                                    if (MatchKeyword(identifier, "finally", TokenType.Finally) == TokenType.Finally)
                                        return TokenType.Finally;
                                    return MatchKeyword(identifier, "final", TokenType.Final);
                                }
                            case 'o':
                                {
                                    if (identifier.Length > 3)
                                        return MatchKeyword(identifier, "foreach", TokenType.ForEach);
                                    return MatchKeyword(identifier, "for", TokenType.For);
                                }
                            case 'r':
                                {
                                    return MatchKeyword(identifier, "from", TokenType.From);
                                }
                            case 'u':
                                return MatchKeyword(identifier, "function", TokenType.Function);
                        }
                    }
                    break;
                case 'i':
                    if (identifier.Length > 1)
                    {
                        switch (identifier[1])
                        {
                            case 'f':
                                return MatchKeyword(identifier, "if", TokenType.If);
                            case 'm':
                                return MatchKeyword(identifier, "import", TokenType.Import);
                            case 'n':
                                return MatchKeyword(identifier, "in", TokenType.In);
                            case 's':
                                return MatchKeyword(identifier, "is", TokenType.Is);
                        }
                    }
                    break;
                case 'n':
                    if (identifier.Length > 2)
                    {
                        switch (identifier[1])
                        {
                            case 'e':
                                return MatchKeyword(identifier, "next", TokenType.Next);
                            case 'i':
                                return MatchKeyword(identifier, "nil", TokenType.Nil);
                        }
                    }
                    break;
                case 'o':
                    return MatchKeyword(identifier, "operator", TokenType.Operator);
                case 'p':
                    if (identifier.Length > 3)
                    {
                        switch (identifier[1])
                        {
                            case 'r':
                                switch (identifier[2])
                                {
                                    case 'i':
                                        {
                                            return MatchKeyword(identifier, "private", TokenType.Private);
                                        }
                                    case 'o':
                                        return MatchKeyword(identifier, "protected", TokenType.Protected);
                                }
                                break;
                            case 'u':
                                return MatchKeyword(identifier, "public", TokenType.Public);
                        }
                    }
                    break;
                case 'r':
                    if (identifier.Length > 3)
                    {
                        switch (identifier[3])
                        {
                            case 'h':
                                return MatchKeyword(identifier, "rethrow", TokenType.Rethrow);
                            case 'u':
                                return MatchKeyword(identifier, "return", TokenType.Return);
                        }
                    }
                    break;
                case 's':
                    if (identifier.Length > 1)
                    {
                        switch (identifier[1])
                        {
                            case 'e':
                                return MatchKeyword(identifier, "self", TokenType.Self);
                            case 'u':
                                return MatchKeyword(identifier, "super", TokenType.Super);
                            case 't':
                                return MatchKeyword(identifier, "static", TokenType.Static);
                        }
                    }
                    break;
                case 't':
                    if (identifier.Length > 2 && identifier[1] == 'r')
                    {
                        switch (identifier[2])
                        {
                            case 'u':
                                return MatchKeyword(identifier, "true", TokenType.True);
                            case 'y':
                                return MatchKeyword(identifier, "try", TokenType.Try);
                        }
                    }
                    else if (identifier.Length > 1)
                    {
                        return MatchKeyword(identifier, "throw", TokenType.Throw);
                    }
                    break;
                case 'v':
                    return MatchKeyword(identifier, "var", TokenType.Var);
                case 'w':
                    return MatchKeyword(identifier, "while", TokenType.While);
                case '_':
                    return MatchKeyword(identifier, "__FILE__", TokenType.Filename);
            }
            return TokenType.Identifier;
        }
    }
}
