#ifndef _LEXER_ERROR_H_
#define _LEXER_ERROR_H_

#include "tokens.h"

/**
 * Print a pretty error to stderr (filename:line:col ${error}) using a Token
 * as the source for the location information.
 * 
 * Format and va_args are passed to fprintf(stderr, format, ...);
 * 
 * exit(EXIT_FAILURE); is then called - noreturn
 */
void lexer_TokenPrettyError(const mcc_Token_t *token, const char *format, ...);

#endif
