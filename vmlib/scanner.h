#ifndef clox_scanner_h
#define clox_scanner_h

#include "common.h"

typedef enum {
  // Single-character tokens.
  TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN,
  TOKEN_LEFT_BRACE, TOKEN_RIGHT_BRACE,
  TOKEN_LEFT_SQ_BRACKET, TOKEN_RIGHT_SQ_BRACKET,
  TOKEN_COMMA, TOKEN_DOT, TOKEN_MINUS, TOKEN_PLUS,
  TOKEN_SEMI_COLON, TOKEN_SLASH, TOKEN_STAR,
  TOKEN_COLON, TOKEN_EOL, TOKEN_VBAR, TOKEN_PERCENT,

  // One or two character tokens.
  TOKEN_BANG, TOKEN_BANG_EQUAL,
  TOKEN_EQUAL, TOKEN_EQUAL_EQUAL,
  TOKEN_GREATER, TOKEN_GREATER_EQUAL,
  TOKEN_LESS, TOKEN_LESS_EQUAL,
  TOKEN_LOGICAL_OR, TOKEN_LOGICAL_AND,

  // Literals.
  TOKEN_IDENTIFIER, TOKEN_STRING, TOKEN_NUMBER,

  // Keywords.
  TOKEN_AS, TOKEN_CLASS, TOKEN_ELSE, TOKEN_ENUM, TOKEN_FALSE,
  TOKEN_FOR, TOKEN_FOREACH, TOKEN_FUN, TOKEN_IF, TOKEN_IMPORT, TOKEN_IN,
  TOKEN_INSTANCEOF, TOKEN_NIL, TOKEN_OPERATOR, TOKEN_RETURN,
  TOKEN_SELF, TOKEN_SUPER, TOKEN_TRUE, TOKEN_VAR, TOKEN_WHILE,
  TOKEN_TRY, TOKEN_CATCH, TOKEN_THROW, TOKEN_FINALLY,

  // Access Modifiers
  TOKEN_PRIVATE, TOKEN_PROTECTED, TOKEN_PUBLIC, TOKEN_STATIC,

  TOKEN_ERROR,
  TOKEN_EOF,

  NUM_TOKENS,
} TokenType;

typedef struct {
    TokenType type;
    const char *start;
    int length;
    int line;
} Token;

typedef struct
{
    const char *start;
    const char *current;
    int line;
} Scanner;

void initScanner(Scanner *scanner, const SourceFile *source);
Token scanToken(Scanner *scanner);

#endif
