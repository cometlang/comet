#include <stdio.h>
#include "tests.h"
#include "scanner.h"

char *token_strings[NUM_TOKENS] = 
{
    [TOKEN_LEFT_PAREN] = "(",
    [TOKEN_RIGHT_PAREN] = ")",
    [TOKEN_LEFT_BRACE] = "{",
    [TOKEN_RIGHT_BRACE] = "}",
    [TOKEN_LEFT_SQ_BRACKET] = "[",
    [TOKEN_RIGHT_SQ_BRACKET] = "]",
    [TOKEN_COMMA] = ",",
    [TOKEN_DOT] = ".",
    [TOKEN_MINUS] = "-",
    [TOKEN_PLUS] = "+",
    [TOKEN_SEMI_COLON] = ";",
    [TOKEN_SLASH] = "/",
    [TOKEN_STAR] = "*",
    [TOKEN_COLON] = ":",
    [TOKEN_EOL] = "\n",
    [TOKEN_VBAR] = "|",
    [TOKEN_PERCENT] = "%",
    [TOKEN_QUESTION_MARK] = "?",
    [TOKEN_AT_SYMBOL] = "@",
    [TOKEN_BANG] = "!",
    [TOKEN_BANG_EQUAL] = "!=",
    [TOKEN_EQUAL] = "=",
    [TOKEN_EQUAL_EQUAL] = "==",
    [TOKEN_GREATER] = ">",
    [TOKEN_GREATER_EQUAL] = ">=",
    [TOKEN_LESS] = "<",
    [TOKEN_LESS_EQUAL] = "<=",
    [TOKEN_PLUS_EQUAL] = "+=",
    [TOKEN_MINUS_EQUAL] = "-=",
    [TOKEN_STAR_EQUAL] = "*=",
    [TOKEN_SLASH_EQUAL] = "/=",
    [TOKEN_PERCENT_EQUAL] = "%=",
    [TOKEN_LOGICAL_OR] = "||",
    [TOKEN_LOGICAL_AND] = "&&",
    [TOKEN_BITWISE_AND] = "&",
    [TOKEN_BITWISE_XOR] = "^",
    [TOKEN_BITSHIFT_LEFT] = "<<",
    [TOKEN_BITSHIFT_RIGHT] = ">>",
    [TOKEN_BITWISE_NEGATE] = "~",
    [TOKEN_LAMBDA_ARGS_OPEN] = "(|",
    [TOKEN_LAMBDA_ARGS_CLOSE] = "|)",
    [TOKEN_IDENTIFIER] = "something",
    [TOKEN_STRING] = "'a string'",
    [TOKEN_NUMBER] = "123456",
    [TOKEN_FILE_NAME] = "__FILE__",
    [TOKEN_AS] = "as",
    [TOKEN_BREAK] = "break",
    [TOKEN_CLASS] = "class",
    [TOKEN_ELSE] = "else",
    [TOKEN_ENUM] = "enum",
    [TOKEN_FALSE] = "false",
    [TOKEN_FOR] = "for",
    [TOKEN_FOREACH] = "foreach",
    [TOKEN_FUN] = "function",
    [TOKEN_IF] = "if",
    [TOKEN_IMPORT] = "import",
    [TOKEN_IN] = "in",
    [TOKEN_IS] = "is",
    [TOKEN_NEXT] = "next",
    [TOKEN_NIL] = "nil",
    [TOKEN_OPERATOR] = "operator",
    [TOKEN_RETHROW] = "rethrow",
    [TOKEN_RETURN] = "return",
    [TOKEN_SELF] = "self",
    [TOKEN_SUPER] = "super",
    [TOKEN_TRUE] = "true",
    [TOKEN_VAR] = "var",
    [TOKEN_WHILE] = "while",
    [TOKEN_TRY] = "try",
    [TOKEN_CATCH] = "catch",
    [TOKEN_THROW] = "throw",
    [TOKEN_FINALLY] = "finally",
    [TOKEN_FINAL] = "final",
    [TOKEN_PRIVATE] = "private",
    [TOKEN_PROTECTED] = "protected",
    [TOKEN_PUBLIC] = "public",
    [TOKEN_STATIC] = "static",
};

void test_scanner_tokens(void)
{
    Scanner scanner;
    SourceFile source;
    // We aren't able to scan for TOKEN_ERROR or TOKEN_EOF
    for (int i = 0; i < NUM_TOKENS - 2; i++)
    {
        source.path = "test_scanner_tokens";
        source.source = token_strings[i];
        initScanner(&scanner, &source);

        printf("Testing: '%s', expecting %d\n", token_strings[i], i);

        Token result = scanToken(&scanner);
        DEBUG_ASSERT(result.type == (TokenType_t) i);
    }
}
