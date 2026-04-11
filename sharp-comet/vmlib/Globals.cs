using sharpcomet.stdlib;
using vmlib;

namespace sharpcomet.vmlib;


public static class Globals
{
    private static Dictionary<string, CometString> _strings = new();
    private static Dictionary<string, CometObject> _modules = new();
    private static Dictionary<CometObject, CometObject> _globals = new();

    public static CometString InternString(string str)
    {
        if (_strings.ContainsKey(str))
        {
            return _strings[str];
        }
        var result = Memory.AllocateObject<CometString>(str);
        _strings[str] = result;
        return result;
    }

    public static CometObject? GetModule(string absolutePath) => _modules.GetValueOrDefault(absolutePath);

    public static void AddModule(string absolutePath, CometObject module) => _modules[absolutePath] = module;

    public static void AddGlobal(CometObject name, CometObject global)
    {
        _globals[name] = global;
    }

    public static CometObject? FindGlobal(CometObject name)
    {
        return _globals.GetValueOrDefault(name);
    }
}