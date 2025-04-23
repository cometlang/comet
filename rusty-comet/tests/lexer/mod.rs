use rusty_comet::lexer::{scan,TokenType};

#[test]
fn empty_string_produces_eof_token() {
    // arrange
    let input = String::new();

    // act
    let output = scan(&input);

    // assert
    assert_eq!(output[0].token_type, TokenType::EndOfFile);
}