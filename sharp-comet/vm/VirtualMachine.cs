using sharpcomet.stdlib;
using sharpcomet.vmlib;
using vmlib;

namespace sharpcomet.vm;

public class VirtualMachine
{
    private const int FRAMES_MAX = 64;


    private Stack<CallFrame> _frames;
    private VmStack<CometObject> _stack;

    public VirtualMachine(CometFunction entryPoint)
    {
        _frames = new(FRAMES_MAX);
        _stack = new();
        _frames.Push(new CallFrame(new Closure(entryPoint)));
    }

    private CallFrame? CurrentCallFrame => _frames.LastOrDefault();

    private void RuntimeError(string format, params string[] args)
    {
        Console.Error.WriteLine(format, args);
    }

    private bool Call(Closure closure, byte argCount)
    {
        if (_frames.Count == FRAMES_MAX)
        {
            RuntimeError("Stack overflow");
            return false;
        }
        _frames.Push(new CallFrame(closure));
        return true;
    }

    private bool Call(NativeFunction func, byte argCount)
    {
        var result = func.Call(_stack.GetTop(argCount));
        _stack.PopMany(argCount + 1); // Also pop the function object off
        _stack.Push(result);
        return true;
    }

    private bool CallValue(CometObject callee, byte argCount)
    {
        if (callee is NativeFunction func)
            return Call(func, argCount);
        else if (callee is Closure closure)
            return Call(closure, argCount);

        RuntimeError("Call only call functions and classes.");
        return false;
    }

    private void CloseUpValues()
    { }


    public InterpretResult Run()
    {
        var frame = CurrentCallFrame;
        if (frame == null)
            return InterpretResult.Success; // nothing to do

        while (true)
        {
            var instruction = frame!.ReadByte();
            switch (instruction)
            {
                case (byte)Op.Nil:
                {
                    _stack.Push(Nil.Instance);
                    break;
                }
                case (byte)Op.True:
                {
                    _stack.Push(CometBoolean.True);
                    break;
                }
                case (byte)Op.False:
                {
                    _stack.Push(CometBoolean.False);
                    break;
                }
                case (byte)Op.Pop:
                {
                    _stack.Pop();
                    break;
                }
                case (byte)Op.Call:
                {
                    byte argCount = frame!.ReadByte();
                    if (!CallValue(_stack.Peek(argCount), argCount))
                    {
                        return InterpretResult.RuntimeError;
                    }
                    frame = CurrentCallFrame;
                    break;
                }
                case (byte)Op.GetGlobal:
                {
                    CometObject name = frame.ReadConstant();
                    var global = Globals.FindGlobal(name);
                    if (global == null)
                    {
                        if (name is CometString str)
                        {
                            RuntimeError("Undefined variable '{0}'.", str.String);
                        }
                        else
                        {
                            RuntimeError("[Bug]: non-string name object for global. Type is: '{0}'", name.GetType().Name);
                        }
                        return InterpretResult.RuntimeError;
                    }
                    _stack.Push(global);
                    break;
                }
                case (byte)Op.DefineGlobal:
                {
                    var name = frame.ReadConstant();
                    Globals.AddGlobal(name, _stack.Peek());
                    // AddModuleVariable
                    _stack.Pop();
                    break;
                }
                case (byte)Op.Constant:
                {
                    _stack.Push(frame.ReadConstant());
                    break;
                }
                case (byte)Op.GetLocal:
                {
                    _stack.Push(frame.GetLocal(frame.ReadByte()));
                    break;
                }
                case (byte)Op.SetLocal:
                {
                    break;
                }
                case (byte)Op.Return:
                {
                    var result = _stack.Peek();
                    CloseUpValues();
                    _frames.Pop();
                    if (_frames.Count == 0)
                    {
                        return InterpretResult.Success;
                    }
                    // What to do with the return result?
                    frame = CurrentCallFrame;
                    break;
                }
                default:
                {
                    RuntimeError($"Unknown Instruction: 0x{Convert.ToHexString([instruction])}");
                    return InterpretResult.RuntimeError;
                }
            }
        }
    }
}
