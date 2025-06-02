namespace sharpcomet.vm;

public class CallFrame
{
    private Closure _closure;
    private int _instructionPointer = 0;
    private readonly byte[] _code;

    public CallFrame(Closure closure)
    {
        _closure = closure;
        _code = _closure.GetCode();
    }

    public byte ReadByte()
    {
        return _code[_instructionPointer++];
    }
}