using sharpcomet.vmlib;

namespace sharpcomet.vm;

public class Interpreter
{
    public static InterpretResult Interpret()
    {
        try
        {
            return InterpretResult.Success;
        }
        catch (CompilationException)
        {
            return InterpretResult.CompilationError;
        }
    }
}