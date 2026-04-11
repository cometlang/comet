namespace sharpcomet.stdlib;

public class CometObject
{
    // Probably want to override HashCode() & Equals()

    public override string ToString()
    {
        return GetType().Name;
    }
}
