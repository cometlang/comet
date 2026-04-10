using sharpcomet.stdlib;

namespace sharpcomet.vmlib;


public static class Globals
{
    private static Dictionary<string, CometString> _strings = new();
    private static Dictionary<string, CometObject> _modules = new();

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

    public static CometObject GetModule(string absolutePath) => _modules.GetValueOrDefault(absolutePath);

    public static void AddModule(string absolutePath, CometObject module) => _modules[absolutePath] = module;
}