use std::str::Chars;
use std::iter::Peekable;

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
    token_type: TokenType,
    repr: String,
    line: u16,
}

struct Scanner<'a> {
    chars: Peekable<Chars<'a>>,
    current: String,
    line: u16,
}

impl<'a> Scanner<'a> {
    fn new(content: &'a String) -> Scanner<'a> {
        Scanner {
            chars: content.chars().peekable(),
            current: String::new(),
            line: 0,
        }
    }

    fn scan_token(&mut self) -> Token {
        self.skip_whitespace();

        let result = Token {
            token_type: TokenType::Error,
            repr: self.current.clone(),
            line: self.line,
        };

        self.current = String::new();

        return result;
    }

    fn skip_whitespace(&mut self) {
        loop {
            match self.chars.peek() {
                Some(x) => {
                    if *x == ' ' || *x == '\r' || *x == '\t' {
                        self.chars.next();
                    }
                    else if *x == '\n' {
                        self.line += 1;
                        return;
                    }
                    else if *x == '#' {
                        self.chars.next();
                        let mut comment_char: Option<&char>;
                        loop {
                            comment_char = self.chars.peek();
                            if comment_char == Some(&'\n') || comment_char == None {
                                return;
                            }
                            self.chars.next();
                        }
                    }
                    else {
                        return;
                    }
                },
                None => return,
            }
        }
    }

    fn finished(&mut self) -> bool {
        match self.chars.peek() {
            Some(_x) => false,
            None => true,
        }
    }
}

pub fn scan<'a>(content: &'a String) -> Vec<Token> {
    let mut scanner = Scanner::new(content);
    let mut result = Vec::<Token>::new();

    while !scanner.finished() {
        result.push(scanner.scan_token());
    }

    return result;
}