use crate::lexer::TokenType;

fn match_keyword(token_content: &String, keyword: &str, keyword_type: TokenType) -> TokenType {
    if token_content == keyword {
        return keyword_type;
    }
    return TokenType::Identifier;
}


pub fn check_keyword<'a>(token_content: &String) -> TokenType {
    let mut chars = token_content.chars().peekable();
    match chars.next() {
        Some(x) => {
            if x == 'a' { return match_keyword(token_content, "as", TokenType::As); }
            else if x == 'b' { return match_keyword(token_content, "break", TokenType::Break); }
            else if x == 'c' {
                match chars.next() {
                    Some(x2) => {
                        if x2 == 'a' { return match_keyword(token_content, "catch", TokenType::Catch); }
                        else if x2 == 'l' { return match_keyword(token_content, "class", TokenType::Class); }
                    },
                    None => (),
                }
            }
            else if x == 'e' {
                match chars.next() {
                    Some(x2) => {
                        if x2 == 'l' { return match_keyword(token_content, "else", TokenType::Else); }
                        else if x2 == 'n' { return match_keyword(token_content, "enum", TokenType::Enum); }
                    },
                    None => (),
                }
            }
            else if x == 'f' {
                match chars.next() {
                    Some(x2) => {
                        if x2 == 'a' { return match_keyword(token_content, "false", TokenType::False); }
                        else if x2 == 'i' {
                            if match_keyword(token_content, "final", TokenType::Final) == TokenType::Final {
                                return TokenType::Final;
                            }
                            return match_keyword(token_content, "finally", TokenType::Finally);
                        }
                        else if x2 == 'o' {
                            if match_keyword(token_content, "for", TokenType::For) == TokenType::For {
                                return TokenType::For;
                            }
                            return match_keyword(token_content, "foreach", TokenType::Foreach);
                        }
                        else if x2 == 'r' { return match_keyword(token_content, "from", TokenType::From); }
                        else if x2 == 'u' { return match_keyword(token_content, "function", TokenType::Function); }
                    },
                    None => (),
                }
            }
            else if x == 'i' {
                match chars.next() {
                    Some(x2) => {
                        if x2 == 'f' { return match_keyword(token_content, "if", TokenType::If); }
                        else if x2 == 'm' { return match_keyword(token_content, "import", TokenType::Import); }
                        else if x2 == 'n' { return match_keyword(token_content, "in", TokenType::In); }
                        else if x2 == 's' { return match_keyword(token_content, "is", TokenType::Is); }
                    },
                    None => (),
                }
            }
            else if x == 'n' {
                match chars.next() {
                    Some(x2) => {
                        if x2 == 'e' { return match_keyword(token_content, "next", TokenType::Next); }
                        else if x2 == 'i' { return match_keyword(token_content, "nil", TokenType::Nil); }
                    },
                    None => (),
                }
            }
            else if x == 'o' { return match_keyword(token_content, "operator", TokenType::Operator); }
            else if x == 'r' {
                if match_keyword(token_content, "return", TokenType::Return) == TokenType::Return {
                    return TokenType::Return;
                }
                else {
                    return match_keyword(token_content, "rethrow", TokenType::Rethrow);
                }
            }
            else if x == 's' {
                match chars.next() {
                    Some(x2) => {
                        if x2 == 'e' { return match_keyword(token_content, "self", TokenType::TokenSelf); }
                        else if x2 == 'u' { return match_keyword(token_content, "super", TokenType::Super); }
                        else if x2 == 't' { return match_keyword(token_content, "static", TokenType::Static); }
                    },
                    None => (),
                }
            }
            else if x == 't' {
                match chars.next() {
                    Some(x2) => {
                        if x2 == 'h' {
                            return match_keyword(token_content, "throw", TokenType::Throw);
                        }
                        else if match_keyword(token_content, "true", TokenType::True) == TokenType::True {
                            return TokenType::True;
                        }
                        else {
                            return match_keyword(token_content, "try", TokenType::Try);
                        }
                    },
                    None => (),
                }
            }
            else if x == 'v' { return match_keyword(token_content, "var", TokenType::Var); }
            else if x == 'w' { return match_keyword(token_content, "while", TokenType::While); }
            else if x == '_' { return match_keyword(token_content, "__FILE__", TokenType::FileName); }
        },
        None => (),
    }
    return TokenType::Identifier;
}
