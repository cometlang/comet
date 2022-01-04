#include <stdio.h>
#include <string.h>

#include "common.h"
#include "scanner.h"

void initScanner(Scanner *scanner, const SourceFile *source)
{
    scanner->start = source->source;
    scanner->current = source->source;
    scanner->line = 1;
}

static bool isAlpha(char c)
{
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           ((unsigned char)c >= 0xC0) ||
           c == '_';
}

static bool isDigit(char c)
{
    return c >= '0' && c <= '9';
}

static bool isHex(char c)
{
    return isDigit(c) ||
        (c >= 'a' && c <= 'f') ||
        (c >= 'A' && c <= 'f');
}

static bool isAtEnd(Scanner *scanner)
{
    return *scanner->current == '\0';
}

static char advance(Scanner *scanner)
{
    scanner->current++;
    return scanner->current[-1];
}

static char peek(Scanner *scanner)
{
    return *scanner->current;
}

static char peekNext(Scanner *scanner)
{
    if (isAtEnd(scanner))
        return '\0';
    return scanner->current[1];
}

static bool match(Scanner *scanner, char expected)
{
    if (isAtEnd(scanner))
        return false;
    if (*scanner->current != expected)
        return false;

    scanner->current++;
    return true;
}

static Token makeToken(Scanner *scanner, TokenType_t type)
{
    Token token;
    token.type = type;
    token.start = scanner->start;
    token.length = (int)(scanner->current - scanner->start);
    token.line = scanner->line;

    return token;
}

static Token errorToken(Scanner *scanner, const char *message)
{
    Token token;
    token.type = TOKEN_ERROR;
    token.start = message;
    token.length = (int)strlen(message);
    token.line = scanner->line;

    return token;
}

static void skipWhitespace(Scanner *scanner)
{
    for (;;)
    {
        char c = peek(scanner);
        switch (c)
        {
        case ' ':
        case '\r':
        case '\t':
            advance(scanner);
            break;

        case '\n':
            scanner->line++;
            // Don't advance(), so we can detect line-ends for tokens
            return;

        case '#':
            // A comment goes until the end of the line.
            while (peek(scanner) != '\n' && !isAtEnd(scanner))
                advance(scanner);
            break;

        default:
            return;
        }
    }
}

static TokenType_t checkKeyword(Scanner *scanner, int start, int length,
                              const char *rest, TokenType_t type)
{
    if (scanner->current - scanner->start == start + length &&
        memcmp(scanner->start + start, rest, length) == 0)
    {
        return type;
    }

    return TOKEN_IDENTIFIER;
}

static TokenType_t identifierType(Scanner *scanner)
{
    switch (scanner->start[0])
    {
    case 'a':
        return checkKeyword(scanner, 1, 1, "s", TOKEN_AS);
    case 'b':
        return checkKeyword(scanner, 1, 4, "reak", TOKEN_BREAK);
    case 'c':
        if (scanner->current - scanner->start > 1)
        {
            switch (scanner->start[1])
            {
            case 'a':
                return checkKeyword(scanner, 2, 3, "tch", TOKEN_CATCH);
            case 'l':
                return checkKeyword(scanner, 2, 3, "ass", TOKEN_CLASS);
            }
        }
        break;
    case 'e':
        if (scanner->current - scanner->start > 1)
        {
            switch (scanner->start[1])
            {
                case 'l':
                    return checkKeyword(scanner, 2, 2, "se", TOKEN_ELSE);
                case 'n':
                    return checkKeyword(scanner, 2, 2, "um", TOKEN_ENUM);
            }
        }
        break;
    case 'f':
        if (scanner->current - scanner->start > 1)
        {
            switch (scanner->start[1])
            {
            case 'a':
                return checkKeyword(scanner, 2, 3, "lse", TOKEN_FALSE);
            case 'i':
            {
                if (checkKeyword(scanner, 2, 5, "nally", TOKEN_FINALLY) == TOKEN_FINALLY)
                    return TOKEN_FINALLY;
                return checkKeyword(scanner, 2, 3, "nal", TOKEN_FINAL);
            }
            case 'o':
            {
                if (scanner->current - scanner->start > 3)
                    return checkKeyword(scanner, 2, 5, "reach", TOKEN_FOREACH);
                return checkKeyword(scanner, 2, 1, "r", TOKEN_FOR);
            }
            case 'u':
                return checkKeyword(scanner, 2, 6, "nction", TOKEN_FUN);
            }
        }
        break;
    case 'i':
        if (scanner->current - scanner->start > 1)
        {
            switch (scanner->start[1])
            {
                case 'f':
                    return checkKeyword(scanner, 1, 1, "f", TOKEN_IF);
                case 'm':
                    return checkKeyword(scanner, 2, 4, "port", TOKEN_IMPORT);
                case 'n':
                {
                    if (scanner->current - scanner->start > 2)
                    {
                        return checkKeyword(scanner, 2, 8, "stanceof", TOKEN_INSTANCEOF);
                    }
                    return checkKeyword(scanner, 1, 1, "n", TOKEN_IN);
                }
            }
        }
        break;
    case 'n':
        if (scanner->current - scanner->start > 2)
        {
            switch (scanner->start[1])
            {
                case 'e':
                    return checkKeyword(scanner, 2, 2, "xt", TOKEN_NEXT);
                case 'i':
                    return checkKeyword(scanner, 2, 1, "l", TOKEN_NIL);
            }
        }
        break;
    case 'o':
        return checkKeyword(scanner, 1, 7, "perator", TOKEN_OPERATOR);
    case 'p':
        if (scanner->current - scanner->start > 4)
        {
            switch (scanner->start[1])
            {
                case 'r':
                    switch (scanner->start[2])
                    {
                        case 'i':
                        {
                            switch (scanner->start[3])
                            {
                                case 'v':
                                    return checkKeyword(scanner, 4, 3, "ate", TOKEN_PRIVATE);
                            }
                            break;
                        }
                        case 'o':
                            return checkKeyword(scanner, 3, 6, "tected", TOKEN_PROTECTED);
                    }
                    break;
                case 'u':
                    return checkKeyword(scanner, 2, 4, "blic", TOKEN_PUBLIC);
            }
        }
        break;
    case 'r':
        if (scanner->current - scanner->start > 3)
        {
            // We haven't checked 1 & 2, so need to include 'et' in the final checks
            switch (scanner->start[3])
            {
                case 'h':
                    return checkKeyword(scanner, 1, 6, "ethrow", TOKEN_RETHROW);
                case 'u':
                    return checkKeyword(scanner, 1, 5, "eturn", TOKEN_RETURN);
            }
        }
        break;
    case 's':
        if (scanner->current - scanner->start > 1)
        {
            switch (scanner->start[1])
            {
                case 'e':
                    return checkKeyword(scanner, 2, 2, "lf", TOKEN_SELF);
                case 'u':
                    return checkKeyword(scanner, 2, 3, "per", TOKEN_SUPER);
                case 't':
                    return checkKeyword(scanner, 2, 4, "atic", TOKEN_STATIC);
            }
        }
        break;
    case 't':
        if (scanner->current - scanner->start > 2 && scanner->start[1] == 'r')
        {
            switch (scanner->start[2])
            {
                case 'u':
                    return checkKeyword(scanner, 2, 2, "ue", TOKEN_TRUE);
                case 'y':
                    return checkKeyword(scanner, 2, 1, "y", TOKEN_TRY);
            }
        }
        else if (scanner->current - scanner->start > 1)
        {
            return checkKeyword(scanner, 1, 4, "hrow", TOKEN_THROW);
        }
        break;
    case 'v':
        return checkKeyword(scanner, 1, 2, "ar", TOKEN_VAR);
    case 'w':
        return checkKeyword(scanner, 1, 4, "hile", TOKEN_WHILE);
    }
    return TOKEN_IDENTIFIER;
}

static Token identifier(Scanner *scanner)
{
    while (isAlpha(peek(scanner)) || isDigit(peek(scanner)))
        advance(scanner);

    if (peek(scanner) == '!' || peek(scanner) == '?')
        advance(scanner);

    return makeToken(scanner, identifierType(scanner));
}

static Token number(Scanner *scanner, char previous)
{
    // hexadecimal
    if (previous == '0' &&
        (peek(scanner) == 'x' || peek(scanner) == 'X'))
    {
        advance(scanner);
        while (isHex(peek(scanner)))
            advance(scanner);
    }
    // decimal
    else
    {
        while (isDigit(peek(scanner)) || peek(scanner) == '_')
            advance(scanner);

        // Look for a fractional part.
        if (peek(scanner) == '.' && isDigit(peekNext(scanner)))
        {
            // Consume the ".".
            advance(scanner);

            while (isDigit(peek(scanner)) || peek(scanner) == '_')
                advance(scanner);
        }
    }

    return makeToken(scanner, TOKEN_NUMBER);
}

static Token string(Scanner *scanner, char terminator)
{
    while (peek(scanner) != terminator && !isAtEnd(scanner))
    {
        if (peek(scanner) == '\n')
            scanner->line++;
        if (peek(scanner) == '\\' && peekNext(scanner) == terminator)
            advance(scanner); // extra advance
        advance(scanner);
    }

    if (isAtEnd(scanner))
        return errorToken(scanner, "Unterminated string.");

    // The terminator.
    advance(scanner);
    return makeToken(scanner, TOKEN_STRING);
}

Token scanToken(Scanner *scanner)
{
    skipWhitespace(scanner);

    scanner->start = scanner->current;

    if (isAtEnd(scanner))
        return makeToken(scanner, TOKEN_EOF);

    char c = advance(scanner);
    if (isAlpha(c))
        return identifier(scanner);
    if (isDigit(c))
        return number(scanner, c);

    switch (c)
    {
    case '(':
        return makeToken(scanner, TOKEN_LEFT_PAREN);
    case ')':
        return makeToken(scanner, TOKEN_RIGHT_PAREN);
    case '{':
        return makeToken(scanner, TOKEN_LEFT_BRACE);
    case '}':
        return makeToken(scanner, TOKEN_RIGHT_BRACE);
    case '[':
        return makeToken(scanner, TOKEN_LEFT_SQ_BRACKET);
    case ']':
        return makeToken(scanner, TOKEN_RIGHT_SQ_BRACKET);
    case '\n':
        return makeToken(scanner, TOKEN_EOL);
    case ';':
        return makeToken(scanner, TOKEN_SEMI_COLON);
    case ',':
        return makeToken(scanner, TOKEN_COMMA);
    case '.':
        return makeToken(scanner, TOKEN_DOT);
    case '~':
        return makeToken(scanner, TOKEN_BITWISE_NEGATE);
    case '^':
        return makeToken(scanner, TOKEN_BITWISE_XOR);
    case '-':
        return makeToken(scanner, match(scanner, '=') ? TOKEN_MINUS_EQUAL : TOKEN_MINUS);
    case '+':
        return makeToken(scanner, match(scanner, '=') ? TOKEN_PLUS_EQUAL : TOKEN_PLUS);
    case '/':
        return makeToken(scanner, match(scanner, '=') ? TOKEN_SLASH_EQUAL : TOKEN_SLASH);
    case '*':
        return makeToken(scanner, match(scanner, '=') ? TOKEN_STAR_EQUAL : TOKEN_STAR);
    case ':':
        return makeToken(scanner, TOKEN_COLON);
    case '%':
        return makeToken(scanner, match(scanner, '=') ? TOKEN_PERCENT_EQUAL : TOKEN_PERCENT);
    case '?':
        return makeToken(scanner, TOKEN_QUESTION_MARK);
    case '|':
        return makeToken(scanner, match(scanner, '|') ? TOKEN_LOGICAL_OR : TOKEN_VBAR);
    case '&':
        return makeToken(scanner, match(scanner, '&') ? TOKEN_LOGICAL_AND : TOKEN_BITWISE_AND);
    case '!':
        return makeToken(scanner, match(scanner, '=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
    case '=':
        return makeToken(scanner, match(scanner, '=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
    case '<':
    {
        if (match(scanner, '='))
            return makeToken(scanner, TOKEN_LESS_EQUAL);
        else if (match(scanner, '<'))
            return makeToken(scanner, TOKEN_BITSHIFT_LEFT);
        return makeToken(scanner, TOKEN_LESS);
    }
    case '>':
    {
        if (match(scanner, '='))
            return makeToken(scanner, TOKEN_GREATER_EQUAL);
        else if (match(scanner, '>'))
            return makeToken(scanner, TOKEN_BITSHIFT_RIGHT);
        return makeToken(scanner, TOKEN_GREATER);
    }
    case '"':
    case '\'':
        return string(scanner, c);
    }

    return errorToken(scanner, "Unexpected character.");
}
