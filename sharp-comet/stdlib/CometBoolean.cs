namespace sharpcomet.stdlib;

public sealed class CometBoolean : CometObject
{
    private bool _value;

    private CometBoolean(bool value)
    {
        _value = value;
    }

    private static CometBoolean _trueInstance = new CometBoolean(true);
    public static CometBoolean True => _trueInstance;

    private static CometBoolean _falseInstance = new CometBoolean(false);
    public static CometBoolean False => _falseInstance;
}