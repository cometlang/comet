namespace sharpcomet.lexer;

public enum TokenType
{
    // Single-character tokens.
    LeftParen, RightParen,
    LeftBrace, RightBrace,
    LeftSquareBracket, RightSquareBracket,
    Comma, Dot, Minus, Plus,
    SemiColon, Slash, Star,
    Colon, EndOfLine, VBar, Percent,
    QuestionMark, AtSymbol,

    // One or two character tokens.
    Bang, BangEqual,
    Equal, EqualEqual,
    GreaterThan, GreaterEqual,
    LessThan, LessEqual,
    PlusEqual, MinusEqual, SlashEqual, StarEqual, PercentEqual,
    LogicalOr, LogicalAnd,
    BitwiseAnd, BitwiseXor, BitwiseNegate,
    BitShiftLeft, BitShiftRight,
    LambdaArgsOpen, LambdaArgsClose,

    // Literals.
    Identifier, String, Number, Filename,

    // Keywords.
    As, Break, Class, Else, Enum, False, For, ForEach, From, Function, If, Import,
    In, Is, Next, Nil, Operator, Rethrow, Return, Self, Super, True, Var, While, Try, Catch,
    Throw, Final, Finally,

    // Access Modifiers
    Private, Protected, Public, Static,

    Error, EndOfFile,
}