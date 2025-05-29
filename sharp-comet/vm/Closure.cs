using System.Collections.Generic;
using sharpcomet.stdlib;
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
}