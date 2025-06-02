using System.Collections.Generic;
using sharpcomet.stdlib;
using sharpcomet.vmlib;

namespace sharpcomet.vm;

public class Closure : CometObject
{
    private CometFunction _function;
    private List<UpValue> _upValues;

    public Closure(CometFunction function)
    {
        _function = function;
        _upValues = new();
    }

    public byte[] GetCode()
    {
        return _function.GetCode();
    }
}