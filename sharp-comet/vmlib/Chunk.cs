using System.Collections.Generic;
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
        _constants.Append(value);
        return _constants.Count - 1;
    }

    private byte MakeConstant(CometObject value)
    {
        int constant = AddConstant(value);
        if (constant > byte.MaxValue)
        {
            throw new CompilationException("Too many constants in one chunk.");
        }

        return (byte)constant;
    }

    public byte IdentifierConstant(Token token)
    {
        return MakeConstant(Strings.InternString(token.Representation));
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