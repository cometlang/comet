using sharpcomet.stdlib;

namespace sharpcomet.vmlib;

public class CometFunction : CometObject
{
    private Chunk _chunk;

    public int Arity;

    public CometFunction()
    {
        _chunk = new();
    }

    public byte MakeConstant(CometObject obj)
    {
        return _chunk.MakeConstant(obj);
    }

    public void EmitBytes(params byte[] bytes)
    {
        _chunk.EmitBytes(bytes);
    }

    public byte[] GetCode()
    {
        return _chunk.GetCode();
    }

    //     for (int i = 0; i < function->upvalueCount; i++)
    //     {
    //         emitByte(parser, compiler.upvalues[i].isLocal ? 1 : 0);
    //         emitByte(parser, compiler.upvalues[i].index);
    //     }
    public void EmitUpValues()
    { }
}