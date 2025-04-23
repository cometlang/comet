#[derive(PartialEq, Debug)]
pub enum TokenType {
    // Single-character tokens.
    LeftParen, RightParen, LeftBrace, RightBrace,
    LeftSquareBracket, RightSquareBracket,
    Comma, Dot, Minus, Plus, SemiColon, Slash, Star,
    Colon, Eol, VBar, Percent, QuestionMark, AtSymbol,

    // One or two character tokens.
    Bang, BangEqual,
    Equal, EqualEqual,
    Greater, GreaterEqual,
    Less, LessEqual,
    PlusEqual, MinusEqual, StarEqual, SlashEqual, PercentEqual,
    LogicalOr, LogicalAnd,
    BitwiseAnd, BitwiseXor, BitwiseNegate,
    BitShiftLeft, BitShiftRight,
    LambdaArgsOpen, LambdaArgsClose,

    // Literals.
    Identifier, String, Number, FileName,

    // Keywords.
    As, Break, Class, Else, Enum, False, For, Foreach, From, Fun, If, Import, In,
    Is, Next, Nil, Operator, Rethrow, Return, TokenSelf, Super, True, Var, While,
    Try, Catch, Throw, Final, Finally,

    // Access Modifiers
    Private, Protected, Public, Static,

    Error, EndOfFile,
}

pub struct Token {
    pub token_type: TokenType,
    pub repr: String,
    pub line: u16,
}

impl Token {
    pub fn new(token_type: TokenType, repr: &String, line: u16) -> Token {
        return Token {
            token_type,
            repr: repr.clone(),
            line,
        }
    }
}
