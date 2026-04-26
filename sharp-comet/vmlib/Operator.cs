using sharpcomet.lexer;

namespace sharpcomet.vmlib;

public enum Operator
{
    Add,
    Subtract,
    Multiply,
    Divide,
    GreaterThan,
    GreaterEqual,
    LessThan,
    LessEqual,
    Equals,
    Index,
    IndexAssign,
    Modulus,
    Unknown,
}

public static class OperatorHelper
{
    public static Operator GetOperatorFromToken(TokenType token)
    {
        switch (token)
        {
            case TokenType.Plus:
                return Operator.Add;
            case TokenType.Minus:
                return Operator.Subtract;
            case TokenType.Star:
                return Operator.Multiply;
            case TokenType.Slash:
                return Operator.Divide;
            case TokenType.GreaterThan:
                return Operator.GreaterThan;
            case TokenType.GreaterEqual:
                return Operator.GreaterEqual;
            case TokenType.EqualEqual:
                return Operator.Equals;
            case TokenType.LessThan:
                return Operator.LessThan;
            case TokenType.LessEqual:
                return Operator.LessEqual;
            case TokenType.Percent:
                return Operator.Modulus;
            case TokenType.LeftSquareBracket:
                return Operator.Index;
            default:
                return Operator.Unknown;
        }
    }
}