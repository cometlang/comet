using System.Collections.Generic;
using sharpcomet.stdlib;
using sharpcomet.vmlib;

namespace sharpcomet.vm;

public class VirtualMachine
{
    private const int FRAMES_MAX = 64;


    private Stack<CallFrame> _frames;
    private Stack<CometObject> _stack;

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

    private void CloseUpValues()
    { }


    public InterpretResult Run()
    {
        var frame = CurrentCallFrame;
        if (frame == null)
            return InterpretResult.Success; // nothing to do

        while (true)
        { 
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
                    var name = frame.Closure.GetConstant(frame.ReadByte());
                    // AddModuleVariable
                    _stack.Pop();
                    break;
                }
                case (byte)Op.Constant:
                {
                    _stack.Push(frame.Closure.GetConstant(frame.ReadByte()));
                    break;
                }
                case (byte)Op.GetLocal:
                {
                    _stack.Push(frame.GetLocal(frame.ReadByte()));
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
