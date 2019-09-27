#include <stdarg.h>
#include "error.h"
#include "tokens.h"
#include "lexer_error.h"

void lexer_TokenPrettyError(const mcc_Token_t *token, const char *format, ...)
{
    va_list arguments;
    va_start(arguments, format);
    PrettyError(
        eral_ResolveFileNameFromNumber(token->fileno),
        token->lineno,
        token->line_index + 1,
        format,
        arguments);
    va_end(arguments);
}
