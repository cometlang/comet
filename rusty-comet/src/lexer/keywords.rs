use std::str::Chars;
use std::iter::Peekable;
use crate::lexer::TokenType;

pub fn check_keyword<'a>(chars: Peekable<Chars<'a>>) -> TokenType {
    return TokenType::Identifier;
}
