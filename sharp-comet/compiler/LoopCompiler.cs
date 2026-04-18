namespace sharpcomet.compiler;

public class LoopCompiler
{
    public int StartAddress { get; set; }
    public int? ExitAddress { get; set; }
    private int _loopScopeDepth;

    public int? BreakJump { get; set; }
    public LoopCompiler? Enclosing { get; private set; }

    public LoopCompiler(int startAddress, int scopeDepth, LoopCompiler? enclosing)
    {
        Enclosing = enclosing;
        StartAddress = startAddress;
        _loopScopeDepth = scopeDepth;
    }

}