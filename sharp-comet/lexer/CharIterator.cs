public class CharIterator
{
    private string _str;
    private int _index;

    public CharIterator(string str)
    {
        _str = str;
    }

    public char Current => _str[_index];

    public char Peek() => _str[_index + 1];

    public char Next()
    {
        return _str[_index++];
    }

    public bool HasNext() => _index < _str.Length;
}