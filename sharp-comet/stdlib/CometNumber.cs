namespace sharpcomet.stdlib;

public sealed class CometNumber : CometObject
{
    private double _number;

    public CometNumber(double number)
    {
        _number = number;
    }
}