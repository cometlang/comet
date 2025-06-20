namespace sharpcomet.compiler;

public struct Precedence
{
    public static Precedence None = new Precedence(0);
    public static Precedence Assignment = new Precedence(1);
    public static Precedence Ternary = new Precedence(2);
    public static Precedence Or = new Precedence(3);
    public static Precedence And = new Precedence(4);
    public static Precedence Equality = new Precedence(5);
    public static Precedence BitShift = new Precedence(6);
    public static Precedence Xor = new Precedence(7);
    public static Precedence BitwiseOr = new Precedence(8);
    public static Precedence BitwiseAnd = new Precedence(9);
    public static Precedence Comparison = new Precedence(10);
    public static Precedence Term = new Precedence(11);
    public static Precedence Factor = new Precedence(12);
    public static Precedence Unary = new Precedence(13);
    public static Precedence Is = new Precedence(14);
    public static Precedence Call = new Precedence(15);
    public static Precedence Primary = new Precedence(16);

    private int _precedenceValue;
    private Precedence(int precedenceValue)
    {
        _precedenceValue = precedenceValue;
    }

    public override bool Equals(object? obj)
    {
        if (obj is Precedence otherObj)
            return otherObj._precedenceValue == this._precedenceValue;
        return false;
    }

    public override int GetHashCode()
    {
        return _precedenceValue.GetHashCode();
    }

    public static bool operator >(Precedence left, Precedence right)
    {
        return left._precedenceValue > right._precedenceValue;
    }

    public static bool operator >=(Precedence left, Precedence right)
    {
        return left._precedenceValue >= right._precedenceValue;
    }

    public static bool operator <(Precedence left, Precedence right)
    {
        return left._precedenceValue < right._precedenceValue;
    }

    public static bool operator <=(Precedence left, Precedence right)
    {
        return left._precedenceValue <= right._precedenceValue;
    }

}