namespace sharpcomet.vm;

public class CallFrame
{
    private int _instructionPointer = 0;
    private readonly byte[] _code;

    public CallFrame(Closure closure)
    {
        Closure = closure;
        _code = Closure.GetCode();
    }

    public Closure Closure { get; private set; }

    public byte ReadByte()
    {
        return _code[_instructionPointer++];
    }
}