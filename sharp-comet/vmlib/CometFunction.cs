using sharpcomet.stdlib;

namespace sharpcomet.vmlib;

public class CometFunction : CometObject
{
    private Chunk _chunk;

    public CometFunction()
    {
        _chunk = new();
    }

}