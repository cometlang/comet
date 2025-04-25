use test_case::test_case;
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
fn whitespace_is_ignored() {
    // arrange
    let input = String::from("       ");

    // act
    let output = scan(&input);

    // assert
    assert_eq!(output[0].token_type, TokenType::EndOfFile);
}

#[test_case("(", TokenType::LeftParen)]
#[test_case(")", TokenType::RightParen)]
#[test_case("{", TokenType::LeftBrace)]
#[test_case("}", TokenType::RightBrace)]
#[test_case("[", TokenType::LeftSquareBracket)]
#[test_case("]", TokenType::RightSquareBracket)]
#[test_case(",", TokenType::Comma)]
#[test_case(".", TokenType::Dot)]
#[test_case("-", TokenType::Minus)]
#[test_case("+", TokenType::Plus)]
#[test_case(";", TokenType::SemiColon)]
#[test_case("/", TokenType::Slash)]
#[test_case("*", TokenType::Star)]
#[test_case(":", TokenType::Colon)]
#[test_case("\n", TokenType::Eol)]
#[test_case("|", TokenType::VBar)]
#[test_case("%", TokenType::Percent)]
#[test_case("?", TokenType::QuestionMark)]
#[test_case("@", TokenType::AtSymbol)]
#[test_case("!", TokenType::Bang)]
#[test_case("!=", TokenType::BangEqual)]
#[test_case("=", TokenType::Equal)]
#[test_case("==", TokenType::EqualEqual)]
#[test_case(">", TokenType::Greater)]
#[test_case(">=", TokenType::GreaterEqual)]
#[test_case("<", TokenType::Less)]
#[test_case("<=", TokenType::LessEqual)]
#[test_case("+=", TokenType::PlusEqual)]
#[test_case("-=", TokenType::MinusEqual)]
#[test_case("*=", TokenType::StarEqual)]
#[test_case("/=", TokenType::SlashEqual)]
#[test_case("%=", TokenType::PercentEqual)]
#[test_case("||", TokenType::LogicalOr)]
#[test_case("&&", TokenType::LogicalAnd)]
#[test_case("&", TokenType::BitwiseAnd)]
#[test_case("^", TokenType::BitwiseXor)]
#[test_case("~", TokenType::BitwiseNegate)]
#[test_case("<<", TokenType::BitShiftLeft)]
#[test_case(">>", TokenType::BitShiftRight)]
#[test_case("(|", TokenType::LambdaArgsOpen)]
#[test_case("|)", TokenType::LambdaArgsClose)]
fn string_produces_correct_token(input_str: &str, expected: TokenType) {
    // arrange
    let input = String::from(input_str);

    // act
    let output = scan(&input);

    // assert
    assert_eq!(output.len(), 2);
    assert_eq!(output[0].token_type, expected);
    assert_eq!(output[0].repr, input_str);
}


#[test_case("'this is a string'" ; "single quoted simple")]
#[test_case("\"this is a string\"" ; "double quoted simple")]
#[test_case("'this is an \\' escaped string string'" ; "single quoted escaped")]
#[test_case("\"this is an \\\" escaped string\"" ; "double quoted escaped")]
#[test_case("'this is a string\ncontaining a newline'" ; "contains newline")]
fn can_scan_string(input_str: &str) {
    // arrange
    let input = String::from(input_str);

    // act
    let output = scan(&input);

    // assert
    assert_eq!(output.len(), 2);
    assert_eq!(output[0].token_type, TokenType::String);
}

#[test_case("'this is a string" ; "single quoted simple")]
#[test_case("\"this is a string" ; "double quoted simple")]
fn spots_unterminated_strings(input_str: &str) {
    // arrange
    let input = String::from(input_str);

    // act
    let output = scan(&input);

    // assert
    assert_eq!(output.len(), 2);
    assert_eq!(output[0].token_type, TokenType::Error); // unterminated string
}