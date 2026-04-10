using System.Collections.Generic;
using System.Linq;
using System.Security.Principal;
using sharpcomet.vmlib;
using sharpcomet.stdlib;

namespace sharpcomet.compiler;

public enum FunctionType
{
    Function,
    Initializer,
    Method,
    Script,
    Lambda,
}

public class FunctionCompiler
{
    public const int GLOBAL_SCOPE = 0;
    public const int UNINITIALIZED_SCOPE = -1;
    public const int UNRESOLVED_VARIABLE_INDEX = -1;

    private Stack<LocalVariable> _locals;
    private FunctionType _functionType;
    public CometFunction Function { get; private set; }

    public int ScopeDepth { get; private set; }
    public FunctionCompiler? Enclosing { get; }

    public FunctionCompiler(FunctionCompiler? parent, FunctionType functionType)
    {
        Enclosing = parent;
        ScopeDepth = parent?.ScopeDepth ?? UNINITIALIZED_SCOPE;
        _locals = new();
        Function = new();
        _functionType = functionType;
        if (_functionType == FunctionType.Method || _functionType == FunctionType.Initializer)
        {
            _locals.Push(new LocalVariable("self"));
        }
    }

    public byte MakeConstant(CometObject value)
    {
        return Function.MakeConstant(value);
    }

    public void EmitBytes(params byte[] instructions)
    {
        Function.EmitBytes(instructions);
    }

    public void EmitConstant(CometObject constant)
    {
        EmitBytes((byte)Op.Constant, Function.MakeConstant(constant));
    }

    public void MarkInitialized()
    {
        if (ScopeDepth > GLOBAL_SCOPE)
        {
            _locals.Last().Depth = ScopeDepth;
        }
    }

    public void BeginScope()
    {
        ScopeDepth++;
    }

    public void EndScope()
    {
        ScopeDepth--;
        while (_locals.Any() && _locals.Peek().Depth > ScopeDepth)
        {
            if (_locals.Peek().IsCaptured)
            {
                EmitBytes((byte)Op.CloseUpValue);
            }
            else
            {
                EmitBytes((byte)Op.Pop);
            }
            _locals.Pop();
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

    public int ResolveLocal(string variableName)
    {
        int result = 0;
        foreach (var local in _locals)
        {
            if (local.Name == variableName)
                return result;
            result++;
        }
        return UNRESOLVED_VARIABLE_INDEX;
    }

    public CometFunction EndCompiler(bool emitParams)
    {
        if (emitParams)
        {
            EmitBytes((byte)Op.Closure, Function.MakeConstant(Function));
            Function.EmitUpValues();
        }
        return Function;
    }

}
