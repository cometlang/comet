using sharpcomet.lexer;
using sharpcomet.stdlib;

namespace sharpcomet.vmlib;

public class Chunk
{
    private List<CometObject> _constants;
    private List<int> _lines;
    private List<ushort> _executionCounts;
    private List<byte> _code;

    public Chunk()
    {
        _constants = new();
        _lines = new();
        _executionCounts = new();
        _code = new();
    }

    private int AddConstant(CometObject value)
    {
        _constants.Add(value);
        return _constants.Count - 1;
    }

    public byte MakeConstant(CometObject value)
    {
        int constant = AddConstant(value);
        if (constant > byte.MaxValue)
        {
            throw new CompilationException("Too many constants in one chunk.");
        }

        return (byte)constant;
    }

    public CometObject GetConstant(byte index)
    {
        return _constants[index];
    }

    public void EmitBytes(params byte[] bytes)
    {
        _code.AddRange(bytes);
    }

    public byte[] GetCode()
    {
        return _code.ToArray();
    }
}