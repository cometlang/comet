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
        // need to "Allocate" the object, such that we can clean it up
        var result = new CometString(str);
        _strings[str] = result;
        return result;
    }
}