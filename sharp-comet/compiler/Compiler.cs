namespace sharpcomet.compiler;

public class Compiler
{
    public const int GLOBAL_SCOPE = 0;
    public const int UNINITIALIZED_SCOPE = -1;

    public int ScopeDepth {get; private set;}

    public Compiler? Enclosing {get;}

    public Compiler(Compiler? parent)
    {
        Enclosing = parent;
        ScopeDepth = UNINITIALIZED_SCOPE;
    }

    public void DefineVariable(ushort variable)
    {

    }

}
