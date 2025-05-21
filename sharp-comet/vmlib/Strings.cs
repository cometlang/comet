using sharpcomet.stdlib;

namespace sharpcomet.vmlib;


public static class Strings
{
    private static Dictionary<string, CometString> _strings = new();

    public static CometString InternString(string str)
    {
        if (_strings.ContainsKey(str))
        {
            return _strings[str];
        }
        var result = new CometString(str);
        _strings[str] = result;
        return result;
    }
}