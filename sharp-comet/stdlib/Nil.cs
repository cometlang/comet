namespace sharpcomet.stdlib;

public sealed class Nil : CometObject
{
    private static Nil _instance = new();
    private Nil() { }

    public static Nil Instance => _instance;

    public override string ToString()
    {
        return "nil";
    }
}