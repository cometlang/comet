use std::collections::HashMap;
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

#[test]
fn comments_are_ignored() {
    // arrange
    let input = String::from("# this is a comment");

    // act
    let output = scan(&input);

    // assert
    assert_eq!(output[0].token_type, TokenType::EndOfFile);
}


#[test]
fn string_produces_correct_token() {
    // arrange
    let token_definitions : HashMap<&str, TokenType> = HashMap::from([
        ("\n", TokenType::Eol),
    ]);

    for def in token_definitions {
        let input = String::from(def.0);

        // act
        let output = scan(&input);

        // assert
        assert_eq!(output[0].token_type, def.1);
    }
}