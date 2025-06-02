using System.Collections.Generic;
using sharpcomet.stdlib;
using sharpcomet.vmlib;

namespace sharpcomet.vm;

public class VirtualMachine
{
    private const int FRAMES_MAX = 64;


    private Stack<CallFrame> _frames;
    private Stack<CometObject> _stack;

    public VirtualMachine()
    {
        _frames = new(FRAMES_MAX);
        _stack = new();
    }

    private CallFrame? CurrentCallFrame => _frames.LastOrDefault();

    private void RuntimeError(string format, params string[] args)
    {
        Console.Error.WriteLine(format, args);
    }

    private bool Call(Closure closure, int argCount)
    {
        if (_stack.Count == FRAMES_MAX)
        {
            RuntimeError("Stack overflow");
            return false;
        }

        _stack.Push(closure);
        return true;
    }

    private void PopCallFrame()
    {
        _frames.Pop();
    }

    private InterpretResult Run()
    {
        var frame = CurrentCallFrame;
        if (frame == null)
            return InterpretResult.Success; // nothing to do

        var instruction = frame.ReadByte();
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
            case (byte)Op.DefineGlobal:
                {
                    break;
                }
            default:
                {
                    RuntimeError($"Unknown Instruction: 0x{Convert.ToHexString([instruction])}");
                    return InterpretResult.RuntimeError;
                }
        }

        return InterpretResult.Success;
    }

}
