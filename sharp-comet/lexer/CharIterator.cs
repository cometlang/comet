using System;

public class CharIterator
{
    private string _str;
    private int _index;

    public CharIterator(string str)
    {
        _str = str;
    }

    public char Peek()
    {
        if (_index < _str.Length)
            return _str[_index];

        return '\0';
    }

    public char Next()
    {
        return _str[_index++];
    }

    public bool HasNext() => _index < _str.Length;

    internal char PeekNext()
    {
        if (_index + 1 < _str.Length)
        {
            return _str[_index + 1];
        }
        return '\0';
    }
}