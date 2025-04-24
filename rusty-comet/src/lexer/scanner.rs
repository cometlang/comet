use std::str::Chars;
use std::iter::Peekable;
use crate::lexer::Token;
use crate::lexer::TokenType;

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

        let c = self.advance().expect("We just checked for end of string - scanning must not be complete");

        if c.is_alphabetic() || c == '_' {
            return self.identifier();
        }

        if c.is_ascii_digit() {
            return self.number();
        }

        let result = match c {
            '\n' => self.make_token(TokenType::Eol),
            '(' => self.make_token(TokenType::LeftParen),
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
            '|' => self.make_token(TokenType::VBar),
            '%' => self.check_equal(TokenType::Percent, TokenType::PercentEqual),
            '?' => self.make_token(TokenType::QuestionMark),
            '@' => self.make_token(TokenType::AtSymbol),
            '!' => self.check_equal(TokenType::Bang, TokenType::BangEqual),
            '=' => self.check_equal(TokenType::Equal, TokenType::EqualEqual),
            '>' => self.check_equal(TokenType::Greater, TokenType::GreaterEqual),
            '<' => self.check_equal(TokenType::Less, TokenType::LessEqual),
            '&' => self.make_token(TokenType::BitwiseAnd),
            '^' => self.make_token(TokenType::BitwiseXor),
            '~' => self.make_token(TokenType::BitwiseNegate),
            '"' => self.string(),
            '\'' => self.string(),
            _ => self.make_token(TokenType::Error),
        };

        self.current = String::new();
        return result;
    }

    // TODO
    fn string(&mut self) -> Token {
        return self.make_token(TokenType::String);
    }

    // TODO
    fn number(&mut self) -> Token {
        return self.make_token(TokenType::Number);
    }

    // TODO
    fn identifier(&mut self) -> Token {
        return self.make_token(TokenType::Identifier);
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