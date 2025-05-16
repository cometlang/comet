using lexer;
using System.Text;

namespace sharpcomet.lexer;

public class Scanner
{
    private CharIterator _contentIter;
    private int _line = 0;
    private StringBuilder _current;

    public Scanner(string content)
    {
        _contentIter = new CharIterator(content);
        _current = new StringBuilder();
    }

    private char Advance()
    {
        _current.Append(_contentIter.Peek());
        return _contentIter.Next();
    }

    private Token MakeToken(TokenType tokenType)
    {
        var result = new Token(tokenType, _current.ToString(), _line);
        _current.Clear();
        return result;
    }

    private Token ErrorToken(string message)
    {
        var result = new Token(TokenType.Error, message, _line);
        _current = new StringBuilder();
        return result;
    }

    private void SkipWhitespace()
    {
        while (true)
        {
            switch (_contentIter.Peek())
            {
            case ' ':
            case '\r':
            case '\t':
                Advance();
                break;

            case '\n':
                _line++;
                // Don't advance(), so we can detect line-ends for tokens
                return;

            case '#':
                // A comment goes until the end of the line.
                while (_contentIter.HasNext() && _contentIter.Peek() != '\n')
                    Advance();
                break;

            default:
                return;
            }
        }
    }

    public Token ScanIdentifier()
    {
        while (char.IsLetterOrDigit(_contentIter.Peek()) || _contentIter.Peek() == '_')
            Advance();

        if (_contentIter.Peek() == '?' || _contentIter.Peek() == '!')
            Advance();

        return MakeToken(Keywords.CheckKeyword(_current.ToString()));
    }

    public Token ScanNumber(char previous)
    {
        if (previous == '0' && (_contentIter.Peek() == 'x' || _contentIter.Peek() == 'X'))
        {
            Advance();
            while (char.IsAsciiHexDigit(_contentIter.Peek()))
                Advance();
        }
        else
        {
            while (char.IsDigit(_contentIter.Peek()) || _contentIter.Peek() == '_')
                Advance();

            if (_contentIter.Peek() == '.' && char.IsDigit(_contentIter.PeekNext()))
            {
                Advance();

                while (char.IsDigit(_contentIter.Peek()) || _contentIter.Peek() == '_')
                    Advance();
            }
        }

        return MakeToken(TokenType.Number);
    }

    public Token ScanString(char terminator)
    {
        // remove the opening quote
        _current.Clear();
        while (_contentIter.Peek() != terminator && _contentIter.HasNext())
        {
            if (_contentIter.Peek() == '\n')
                _line++;
            if (_contentIter.Peek() == '\\' && _contentIter.PeekNext() == terminator)
                Advance(); // extra advance
            Advance();
        }

        if (!_contentIter.HasNext())
            return ErrorToken("Unterminated string.");

        // The terminator.
        _contentIter.Next();

        return MakeToken(TokenType.String);
    }

    public bool Match(char toMatch)
    {
        if (_contentIter.Peek() == toMatch)
        {
            Advance();
            return true;
        }
        return false;
    }

    public Token ScanToken()
    {
        SkipWhitespace();

        if (!_contentIter.HasNext())
        {
            return MakeToken(TokenType.EndOfFile);
        }

        char c = Advance();

        if (char.IsLetter(c) || c == '_')
            return ScanIdentifier();

        if (char.IsDigit(c))
            return ScanNumber(c);

        switch (c)
        {
            case '(':  return Match('|') ? MakeToken(TokenType.LambdaArgsOpen) : MakeToken(TokenType.LeftParen);
            case ')':  return MakeToken(TokenType.RightParen);
            case '{':  return MakeToken(TokenType.LeftBrace);
            case '}':  return MakeToken(TokenType.RightBrace);
            case '[':  return MakeToken(TokenType.LeftSquareBracket);
            case ']':  return MakeToken(TokenType.RightSquareBracket);
            case ',':  return MakeToken(TokenType.Comma);
            case '.':  return MakeToken(TokenType.Dot);
            case '-':  return Match('=') ? MakeToken(TokenType.MinusEqual) : MakeToken(TokenType.Minus);
            case '+':  return Match('=') ? MakeToken(TokenType.PlusEqual) : MakeToken(TokenType.Plus);
            case ';':  return MakeToken(TokenType.SemiColon);
            case '/':  return Match('=') ? MakeToken(TokenType.SlashEqual) : MakeToken(TokenType.Slash);
            case '*':  return Match('=') ? MakeToken(TokenType.StarEqual) : MakeToken(TokenType.Star);
            case ':':  return MakeToken(TokenType.Colon);
            case '|':
            {
                if (Match(')'))
                    return MakeToken(TokenType.LambdaArgsClose);
                else if (Match('|'))
                    return MakeToken(TokenType.LogicalOr);
                else
                    return MakeToken(TokenType.VBar);
            }
            case '%':  return Match('=') ? MakeToken(TokenType.PercentEqual) : MakeToken(TokenType.Percent);
            case '?':  return MakeToken(TokenType.QuestionMark);
            case '@':  return MakeToken(TokenType.AtSymbol);
            case '\n': return MakeToken(TokenType.EndOfLine);
            case '~': return MakeToken(TokenType.BitwiseNegate);
            case '^': return MakeToken(TokenType.BitwiseXor);
            case '&': return Match('&') ? MakeToken(TokenType.LogicalAnd) : MakeToken(TokenType.BitwiseAnd);
            case '!': return Match('=') ? MakeToken(TokenType.BangEqual) : MakeToken(TokenType.Bang);
            case '=': return Match('=') ? MakeToken(TokenType.EqualEqual) : MakeToken(TokenType.Equal);
            case '<':
            {
                if (Match('<'))
                    return MakeToken(TokenType.BitShiftLeft);
                else if (Match('='))
                    return MakeToken(TokenType.LessEqual);
                else
                    return MakeToken(TokenType.LessThan);
            }
            case '>':
            {
                if (Match('>'))
                    return MakeToken(TokenType.BitShiftRight);
                else if (Match('='))
                    return MakeToken(TokenType.GreaterEqual);
                else
                    return MakeToken(TokenType.GreaterThan);
            }
            case '"':
            case '\'':
                return ScanString(c);
        }

        return ErrorToken($"Unexpected character: '{c}'");
    }
}
