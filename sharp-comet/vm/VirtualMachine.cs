using System.Collections.Generic;

namespace sharpcomet.vm;

public class VirtualMachine
{

    private List<CallFrame> _frames;

    public VirtualMachine()
    {
        _frames = new();
    }

    public CallFrame? CurrentCallFrame => _frames.LastOrDefault();

}
