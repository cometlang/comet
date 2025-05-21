using System.Collections.Generic;
using System.Linq;
using sharpcomet.vmlib;

namespace sharpcomet.compiler;

public class FunctionCompiler
{
    public const int GLOBAL_SCOPE = 0;
    public const int UNINITIALIZED_SCOPE = -1;

    private List<LocalVariable> _locals;
    private Chunk _chunk;

    public int ScopeDepth { get; private set; }

    public FunctionCompiler? Enclosing { get; }

    public FunctionCompiler(FunctionCompiler? parent)
    {
        Enclosing = parent;
        ScopeDepth = UNINITIALIZED_SCOPE;
        _locals = new();
        _chunk = new();
    }

    public void EmitBytes(params byte[] instructions)
    {

    }

    private void MarkInitialized()
    {
        if (ScopeDepth > GLOBAL_SCOPE)
        {
            _locals.Last().Depth = ScopeDepth;
        }
    }

    public void DefineVariable(byte variable)
    {
        if (ScopeDepth > GLOBAL_SCOPE)
        {
            MarkInitialized();
        }
        else
        {
            EmitBytes((byte)Op.DefineGlobal, variable);
        }
    }

}
