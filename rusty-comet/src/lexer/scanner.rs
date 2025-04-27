use std::str::Chars;
use std::iter::Peekable;
use crate::lexer::Token;
use crate::lexer::TokenType;
use crate::lexer::keywords::check_keyword;

struct Scanner<'a> {
    chars: Peekable<Chars<'a>>,
    current: String,
    line: i32,
}

impl<'a> Scanner<'a> {
    fn new(content: &'a String) -> Scanner<'a> {
        return Scanner {
            chars: content.chars().peekable(),
            current: String::new(),
            line: 0,
        }
    }

    fn make_token(&self, token_type: TokenType) -> Token {
        return Token::new(token_type, &self.current, self.line);
    }

    fn error_token(&self, message: &str) -> Token {
        return Token::new(TokenType::Error, &String::from(message), self.line);
    }

    fn advance(&mut self) -> Option<char> {
        let c = self.chars.next();
        match c {
            Some(x) => self.current.push(x),
            None => (),
        }
        return c;
    }

    fn check_equal(&mut self, without: TokenType, with: TokenType) -> Token {
        if self.chars.peek() == Some(&'=') {
            self.advance();
            return self.make_token(with);
        } else {
            return self.make_token(without);
        }
    }

    fn scan_token(&mut self) -> Token {
        self.skip_whitespace();

        if self.finished() {
            return self.make_token(TokenType::EndOfFile);
        }

        let c = self.advance().unwrap();

        if c.is_alphabetic() || c == '_' {
            return self.identifier();
        }

        if c.is_ascii_digit() {
            return self.number(c);
        }

        let result = match c {
            '\n' => self.make_token(TokenType::Eol),
            '(' => {
                return match self.chars.peek() {
                    Some(x) => {
                        if *x == '|' {
                            self.advance();
                            return self.make_token(TokenType::LambdaArgsOpen);
                        }
                        else {
                            return self.make_token(TokenType::LeftParen);
                        }
                    },
                    _ => self.make_token(TokenType::LeftParen),
                }
            },
            ')' => self.make_token(TokenType::RightParen),
            '[' => self.make_token(TokenType::LeftSquareBracket),
            ']' => self.make_token(TokenType::RightSquareBracket),
            '{' => self.make_token(TokenType::LeftBrace),
            '}' => self.make_token(TokenType::RightBrace),
            ',' => self.make_token(TokenType::Comma),
            '.' => self.make_token(TokenType::Dot),
            '-' => self.check_equal(TokenType::Minus, TokenType::MinusEqual),
            '+' => self.check_equal(TokenType::Plus, TokenType::PlusEqual),
            ';' => self.make_token(TokenType::SemiColon),
            '/' => self.check_equal(TokenType::Slash, TokenType::SlashEqual),
            '*' => self.check_equal(TokenType::Star, TokenType::StarEqual),
            ':' => self.make_token(TokenType::Colon),
            '|' => {
                return match self.chars.peek() {
                    Some(x) => {
                        if *x == '|' {
                            self.advance();
                            return self.make_token(TokenType::LogicalOr);
                        }
                        else if *x == ')' {
                            self.advance();
                            return self.make_token(TokenType::LambdaArgsClose);
                        }
                        else {
                            return self.make_token(TokenType::VBar);
                        }
                    },
                    _ => self.make_token(TokenType::VBar),
                }
            },
            '%' => self.check_equal(TokenType::Percent, TokenType::PercentEqual),
            '?' => self.make_token(TokenType::QuestionMark),
            '@' => self.make_token(TokenType::AtSymbol),
            '!' => self.check_equal(TokenType::Bang, TokenType::BangEqual),
            '=' => self.check_equal(TokenType::Equal, TokenType::EqualEqual),
            '>' => {
                return match self.chars.peek() {
                    Some(x) => {
                        if *x == '>' {
                            self.advance();
                            return self.make_token(TokenType::BitShiftRight);
                        }
                        else if *x == '=' {
                            self.advance();
                            return self.make_token(TokenType::GreaterEqual);
                        }
                        else {
                            return self.make_token(TokenType::Greater);
                        }
                    },
                    _ => self.make_token(TokenType::Greater),
                }
            },
            '<' => {
                return match self.chars.peek() {
                    Some(x) => {
                        if *x == '<' {
                            self.advance();
                            return self.make_token(TokenType::BitShiftLeft);
                        }
                        else if *x == '=' {
                            self.advance();
                            return self.make_token(TokenType::LessEqual);
                        }
                        else {
                            return self.make_token(TokenType::Less);
                        }
                    },
                    _ => self.make_token(TokenType::Less),
                }
            },
            '&' => {
                return match self.chars.peek() {
                    Some(x) => {
                        if *x == '&' {
                            self.advance();
                            return self.make_token(TokenType::LogicalAnd);
                        }
                        else {
                            return self.make_token(TokenType::BitwiseAnd);
                        }
                    },
                    _ => self.make_token(TokenType::BitwiseAnd),
                }
            },
            '^' => self.make_token(TokenType::BitwiseXor),
            '~' => self.make_token(TokenType::BitwiseNegate),
            '"' => self.string('"'),
            '\'' => self.string('\''),
            _ => self.make_token(TokenType::Error),
        };

        self.current = String::new();
        return result;
    }

    fn string(&mut self, terminator: char) -> Token {
        self.current.clear(); // drop the opening quote from the repr
        let mut c = self.chars.peek();
        while c != None && c != Some(&terminator) {
            match c {
                Some(&'\n') => {
                    self.line += 1;
                    self.advance();
                },
                Some(&'\\') => {
                    self.advance();
                    if self.chars.peek() == Some(&terminator) {
                        self.advance();
                    }
                },
                _ => { self.advance(); },
            }
            c = self.chars.peek();
        }

        if c == None {
            return self.error_token("Unterminated String");
        }

        self.chars.next(); // avoid adding the closing quote to the repr
        return self.make_token(TokenType::String);
    }

    fn number(&mut self, first_digit: char) -> Token {
        let mut c = self.advance();
        if first_digit == '0' && c == Some('x') {
            c = self.advance();
            while c.unwrap_or('x').is_ascii_hexdigit() {
                c = self.advance();
            }
        }
        else {
            while c.unwrap_or('x').is_ascii_digit() || c == Some('_') {
                c = self.advance();
            }
            if c == Some('.') {
                c = self.advance();
                while c.unwrap_or('x').is_ascii_digit() || c == Some('_') {
                    c = self.advance();
                }
            }
        }
        return self.make_token(TokenType::Number);
    }

    fn identifier(&mut self) -> Token {
        self.advance();
        let mut c = self.chars.peek();
        while c.is_some() {
            let x = c.unwrap();
            if x.is_alphanumeric() || *x == '_' {
                self.advance();
                c = self.chars.peek();
            }
            else {
                if *x == '?' || *x == '!' {
                    self.advance();
                }
                break;
            }
        }
        return self.make_token(check_keyword(&self.current));
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

    let mut token = scanner.scan_token();
    while token.token_type != TokenType::EndOfFile {
        result.push(token);
        token = scanner.scan_token();
    }
    result.push(token);

    return result;
}