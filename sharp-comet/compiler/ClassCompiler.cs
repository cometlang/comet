using sharpcomet.lexer;

namespace sharpcomet.compiler;

public class ClassCompiler
{
    private Token _classNameToken;

    public ClassCompiler(Token classNameToken, ClassCompiler? enclosing)
    {
        _classNameToken = classNameToken;
        Enclosing = enclosing;
    }

    public ClassCompiler? Enclosing { get; }
}