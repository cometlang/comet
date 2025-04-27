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
    assert_eq!(output[0].token_type, TokenType::Error);
    assert_eq!(output[0].repr, String::from("Unterminated String"));
}


#[test_case("_leading_underscore")]
#[test_case("_has2numbers2")]
#[test_case("end_with_bang!")]
#[test_case("end_with_question?")]
fn can_scan_identifier(input_str: &str) {
    // arrange
    let input = String::from(input_str);

    // act
    let output = scan(&input);

    // assert
    assert_eq!(output.len(), 2);
    assert_eq!(output[0].token_type, TokenType::Identifier);
    assert_eq!(output[0].repr, input);
}

#[test_case("1234")]
#[test_case("1234.4321")]
#[test_case("0x1234")]
#[test_case("1_234")]
fn can_scan_numbers(input_str: &str) {
    // arrange
    let input = String::from(input_str);

    // act
    let output = scan(&input);

    // assert
    assert_eq!(output.len(), 2);
    assert_eq!(output[0].token_type, TokenType::Number);
    assert_eq!(output[0].repr, input);
}

#[test_case("as", TokenType::As)]
#[test_case("break", TokenType::Break)]
#[test_case("class", TokenType::Class)]
#[test_case("else", TokenType::Else)]
#[test_case("enum", TokenType::Enum)]
#[test_case("false", TokenType::False)]
#[test_case("for", TokenType::For)]
#[test_case("foreach", TokenType::Foreach)]
#[test_case("from", TokenType::From)]
#[test_case("function", TokenType::Function)]
#[test_case("if", TokenType::If)]
#[test_case("import", TokenType::Import)]
#[test_case("in", TokenType::In)]
#[test_case("is", TokenType::Is)]
#[test_case("next", TokenType::Next)]
#[test_case("nil", TokenType::Nil)]
#[test_case("operator", TokenType::Operator)]
#[test_case("rethrow", TokenType::Rethrow)]
#[test_case("return", TokenType::Return)]
#[test_case("self", TokenType::TokenSelf)]
#[test_case("super", TokenType::Super)]
#[test_case("true", TokenType::True)]
#[test_case("var", TokenType::Var)]
#[test_case("while", TokenType::While)]
#[test_case("try", TokenType::Try)]
#[test_case("catch", TokenType::Catch)]
#[test_case("throw", TokenType::Throw)]
#[test_case("final", TokenType::Final)]
#[test_case("finally", TokenType::Finally)]
#[test_case("__FILE__", TokenType::FileName)]
fn can_scan_keywords(input_str: &str, expected: TokenType) {
    // arrange
    let input = String::from(input_str);

    // act
    let output = scan(&input);

    // assert
    assert_eq!(output.len(), 2);
    assert_eq!(output[0].token_type, expected);
    assert_eq!(output[0].repr, input);
}