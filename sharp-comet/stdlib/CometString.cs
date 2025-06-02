namespace sharpcomet.stdlib;

public sealed class CometString : CometObject
{
    public string String { get; }

    public CometString(string str)
    {
        String = str;
    }

}
