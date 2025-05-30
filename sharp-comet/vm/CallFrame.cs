namespace sharpcomet.vm;

public class CallFrame
{
    private Closure _closure;

    public CallFrame(Closure closure)
    {
        _closure = closure;
    }

    public byte ReadByte()
    {
        return 0;
    }
}