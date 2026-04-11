namespace sharpcomet.stdlib;

public sealed class Number : CometObject
{
    private double _number;

    public Number(double number)
    {
        _number = number;
    }

    public override string ToString()
    {
        return _number.ToString();
    }
}