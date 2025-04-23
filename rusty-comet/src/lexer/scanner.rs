use std::str::Chars;
use std::iter::Peekable;
use crate::lexer::Token;
use crate::lexer::TokenType;

struct Scanner<'a> {
    chars: Peekable<Chars<'a>>,
    current: String,
    line: u16,
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

    fn scan_token(&mut self) -> Token {
        self.skip_whitespace();

        if self.finished() {
            return self.make_token(TokenType::EndOfFile);
        }

        let result = self.make_token(TokenType::Error);
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

    let mut token = scanner.scan_token();
    while token.token_type != TokenType::EndOfFile {
        result.push(token);
        token = scanner.scan_token();
    }
    result.push(token);

    return result;
}