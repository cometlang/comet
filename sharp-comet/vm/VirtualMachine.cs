using System.Collections.Generic;

namespace sharpcomet.vm;

public class VirtualMachine
{

    private Stack<CallFrame> _frames;

    public VirtualMachine()
    {
        _frames = new();
    }

    public CallFrame? CurrentCallFrame => _frames.LastOrDefault();

    public void Call(Closure closure, int argCount)
    {

    }

    public void PopCallFrame()
    {
        _frames.Pop();
    }

}
