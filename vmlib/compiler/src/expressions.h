#ifndef _EXPRESSIONS_H_
#define _EXPRESSIONS_H_
#include "compiler_defs.h"

void parsePrecedence(Parser *parser, Precedence precedence);
void expression(Parser *parser);
ParseRule *getRule(TokenType_t type);

#endif