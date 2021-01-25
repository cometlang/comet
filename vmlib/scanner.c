#include <stdio.h>
#include <string.h>

#include "common.h"
#include "scanner.h"

typedef struct
{
    const char *start;
    const char *current;
    int line;
} Scanner;

static Scanner scanner;

void initScanner(const SourceFile *source)
{
    scanner.start = source->source;
    scanner.current = source->source;
    scanner.line = 1;
}

static bool isAlpha(char c)
{
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           c == '_';
}

static bool isDigit(char c)
{
    return c >= '0' && c <= '9';
}

static bool isAtEnd()
{
    return *scanner.current == '\0';
}

static char advance(void)
{
    scanner.current++;
    return scanner.current[-1];
}

static char peek(void)
{
    return *scanner.current;
}

static char peekNext()
{
    if (isAtEnd())
        return '\0';
    return scanner.current[1];
}

static bool match(char expected)
{
    if (isAtEnd())
        return false;
    if (*scanner.current != expected)
        return false;

    scanner.current++;
    return true;
}

static Token makeToken(TokenType type)
{
    Token token;
    token.type = type;
    token.start = scanner.start;
    token.length = (int)(scanner.current - scanner.start);
    token.line = scanner.line;

    return token;
}

static Token errorToken(const char *message)
{
    Token token;
    token.type = TOKEN_ERROR;
    token.start = message;
    token.length = (int)strlen(message);
    token.line = scanner.line;

    return token;
}

static void skipWhitespace()
{
    for (;;)
    {
        char c = peek();
        switch (c)
        {
        case ' ':
        case '\r':
        case '\t':
            advance();
            break;

        case '\n':
            scanner.line++;
            // Don't advance(), so we can detect line-ends for tokens
            return;

        case '#':
            // A comment goes until the end of the line.
            while (peek() != '\n' && !isAtEnd())
                advance();
            break;

        default:
            return;
        }
    }
}

static TokenType checkKeyword(int start, int length,
                              const char *rest, TokenType type)
{
    if (scanner.current - scanner.start == start + length &&
        memcmp(scanner.start + start, rest, length) == 0)
    {
        return type;
    }

    return TOKEN_IDENTIFIER;
}

static TokenType identifierType()
{
    switch (scanner.start[0])
    {
    case 'c':
        if (scanner.current - scanner.start > 1)
        {
            switch (scanner.start[1])
            {
            case 'a':
                return checkKeyword(2, 3, "tch", TOKEN_CATCH);
            case 'l':
                return checkKeyword(2, 3, "ass", TOKEN_CLASS);
            }
        }
        break;
    case 'e':
        if (scanner.current - scanner.start > 1)
        {
            switch (scanner.start[1])
            {
                case 'l':
                    return checkKeyword(2, 2, "se", TOKEN_ELSE);
                case 'n':
                    return checkKeyword(2, 2, "um", TOKEN_ENUM);
            }
        }
        break;
    case 'f':
        if (scanner.current - scanner.start > 1)
        {
            switch (scanner.start[1])
            {
            case 'a':
                return checkKeyword(2, 3, "lse", TOKEN_FALSE);
            case 'i':
                return checkKeyword(2, 5, "nally", TOKEN_FINALLY);
            case 'o':
                return checkKeyword(2, 1, "r", TOKEN_FOR);
            case 'u':
                return checkKeyword(2, 6, "nction", TOKEN_FUN);
            }
        }
        break;
    case 'i':
        if (scanner.current - scanner.start > 1)
        {
            switch (scanner.start[1])
            {
                case 'f':
                    return checkKeyword(1, 1, "f", TOKEN_IF);
                case 'n':
                {
                    if (scanner.current - scanner.start > 2)
                    {
                        return checkKeyword(2, 8, "stanceof", TOKEN_INSTANCEOF);
                    }
                    return checkKeyword(1, 1, "n", TOKEN_IN);
                }
            }
        }
        break;
    case 'n':
        return checkKeyword(1, 2, "il", TOKEN_NIL);
    case 'o':
        return checkKeyword(1, 7, "perator", TOKEN_OPERATOR);
    case 'p':
        if (scanner.current - scanner.start > 4)
        {
            if (scanner.start[1] == 'r')
            {
                switch (scanner.start[2])
                {
                    case 'i':
                    {
                        switch (scanner.start[3])
                        {
                            case 'v':
                                return checkKeyword(4, 3, "ate", TOKEN_PRIVATE);
                        }
                        break;
                    }
                    case 'o':
                        return checkKeyword(3, 6, "tected", TOKEN_PROTECTED);
                    case 'u':
                       return checkKeyword(3, 4, "blic", TOKEN_PUBLIC);
                }
            }
        }
        break;
    case 'r':
        return checkKeyword(1, 5, "eturn", TOKEN_RETURN);
    case 's':
        if (scanner.current - scanner.start > 1)
        {
            switch (scanner.start[1])
            {
                case 'e':
                    return checkKeyword(2, 2, "lf", TOKEN_SELF);
                case 'u':
                    return checkKeyword(2, 3, "per", TOKEN_SUPER);
                case 't':
                    return checkKeyword(2, 4, "atic", TOKEN_STATIC);
            }
        }
        break;
    case 't':
        if (scanner.current - scanner.start > 2 && scanner.start[1] == 'r')
        {
            switch (scanner.start[2])
            {
                case 'u':
                    return checkKeyword(2, 2, "ue", TOKEN_TRUE);
                case 'y':
                    return checkKeyword(2, 1, "y", TOKEN_TRY);
            }
        }
        else if (scanner.current - scanner.start > 1)
        {
            return checkKeyword(1, 4, "hrow", TOKEN_THROW);
        }
        break;
    case 'v':
        return checkKeyword(1, 2, "ar", TOKEN_VAR);
    case 'w':
        return checkKeyword(1, 4, "hile", TOKEN_WHILE);
    }
    return TOKEN_IDENTIFIER;
}

static Token identifier()
{
    while (isAlpha(peek()) || isDigit(peek()))
        advance();

    if (peek() == '!' || peek() == '?')
        advance();

    return makeToken(identifierType());
}

static Token number()
{
    while (isDigit(peek()))
        advance();

    // Look for a fractional part.
    if (peek() == '.' && isDigit(peekNext()))
    {
        // Consume the ".".
        advance();

        while (isDigit(peek()))
            advance();
    }

    return makeToken(TOKEN_NUMBER);
}

static Token string(char terminator)
{
    while (peek() != terminator && !isAtEnd())
    {
        if (peek() == '\n')
            scanner.line++;
        if (peek() == '\\' && peekNext() == terminator)
            advance(); // extra advance
        advance();
    }

    if (isAtEnd())
        return errorToken("Unterminated string.");

    // The terminator.
    advance();
    return makeToken(TOKEN_STRING);
}

Token scanToken()
{
    skipWhitespace();

    scanner.start = scanner.current;

    if (isAtEnd())
        return makeToken(TOKEN_EOF);

    char c = advance();
    if (isAlpha(c))
        return identifier();
    if (isDigit(c))
        return number();

    switch (c)
    {
    case '(':
        return makeToken(TOKEN_LEFT_PAREN);
    case ')':
        return makeToken(TOKEN_RIGHT_PAREN);
    case '{':
        return makeToken(TOKEN_LEFT_BRACE);
    case '}':
        return makeToken(TOKEN_RIGHT_BRACE);
    case '[':
        return makeToken(TOKEN_LEFT_SQ_BRACKET);
    case ']':
        return makeToken(TOKEN_RIGHT_SQ_BRACKET);
    case '\n':
        return makeToken(TOKEN_EOL);
    case ';':
        return makeToken(TOKEN_SEMI_COLON);
    case ',':
        return makeToken(TOKEN_COMMA);
    case '.':
        return makeToken(TOKEN_DOT);
    case '-':
        return makeToken(TOKEN_MINUS);
    case '+':
        return makeToken(TOKEN_PLUS);
    case '/':
        return makeToken(TOKEN_SLASH);
    case '*':
        return makeToken(TOKEN_STAR);
    case ':':
        return makeToken(TOKEN_COLON);
    case '|':
        return makeToken(match('|') ? TOKEN_LOGICAL_OR : TOKEN_VBAR);
    case '&':
        if (match('&'))
            return makeToken(TOKEN_LOGICAL_AND);
        break;
    case '!':
        return makeToken(match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
    case '=':
        return makeToken(match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
    case '<':
        return makeToken(match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
    case '>':
        return makeToken(match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
    case '"':
    case '\'':
        return string(c);
    }

    return errorToken("Unexpected character.");
}
