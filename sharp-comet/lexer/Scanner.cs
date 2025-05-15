 using System;
 using System.Text;

namespace sharpcomet.lexer;

public class Scanner
{
    private readonly string _content;
    private CharEnumerator _contentIter;
    private bool _finished = false;
    private int _line = 0;
    private StringBuilder _current;

    public Scanner(string content)
    {
        _content = content;
        _contentIter = content.GetEnumerator();
        _current = new StringBuilder();
        Advance();
    }

    private char Peek() => _contentIter.Current;

    private void Advance()
    {
        _finished = !_contentIter.MoveNext();
        _current.Append(_contentIter.Current);
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

    }

    public Token ScanToken()
    {
        SkipWhitespace();

        if (_finished)
        {
            return MakeToken(TokenType.EndOfFile);
        }

        return ErrorToken($"Unexpected character: '{_contentIter.Current}'");
    }
}
