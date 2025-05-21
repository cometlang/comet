namespace sharpcomet.stdlib;

public class CometString : CometObject
{
    public string String { get; }

    public CometString(string str)
    {
        String = str;
    }

}
