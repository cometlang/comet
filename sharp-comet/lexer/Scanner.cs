 using System;
 using System.Text;

namespace sharpcomet.lexer;

public class Scanner
{
    private readonly string _content;
    private CharIterator _contentIter;
    private int _line = 0;
    private StringBuilder _current;

    public Scanner(string content)
    {
        _content = content;
        _contentIter = new CharIterator(content);
        _current = new StringBuilder();
    }

    private char Advance()
    {
        _current.Append(_contentIter.Current);
        return _contentIter.Next();
    }

    private Token MakeToken(TokenType tokenType)
    {
        var result = new Token(tokenType, _current.ToString(), _line);
        _current = new StringBuilder();
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
        if (!_contentIter.HasNext())
            return;

        while (true)
        {
            switch (_contentIter.Current)
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

    public Token ScanToken()
    {
        SkipWhitespace();

        if (!_contentIter.HasNext())
        {
            return MakeToken(TokenType.EndOfFile);
        }

        char c = Advance();
        switch (c)
        {
            case '(':  return MakeToken(TokenType.LeftParen);
            case ')':  return MakeToken(TokenType.RightParen);
            case '{':  return MakeToken(TokenType.LeftBrace);
            case '}':  return MakeToken(TokenType.RightBrace);
            case '[':  return MakeToken(TokenType.LeftSquareBracket);
            case ']':  return MakeToken(TokenType.RightSquareBracket);
            case ',':  return MakeToken(TokenType.Comma);
            case '.':  return MakeToken(TokenType.Dot);
            case '-':  return MakeToken(TokenType.Minus);
            case '+':  return MakeToken(TokenType.Plus);
            case ';':  return MakeToken(TokenType.SemiColon);
            case '/':  return MakeToken(TokenType.Slash);
            case '*':  return MakeToken(TokenType.Star);
            case ':':  return MakeToken(TokenType.Colon);
            case '|':  return MakeToken(TokenType.VBar);
            case '%':  return MakeToken(TokenType.Percent);
            case '?':  return MakeToken(TokenType.QuestionMark);
            case '@':  return MakeToken(TokenType.AtSymbol);
            case '\n': return MakeToken(TokenType.EndOfLine);
        }

        return ErrorToken($"Unexpected character: '{_contentIter.Current}'");
    }
}
