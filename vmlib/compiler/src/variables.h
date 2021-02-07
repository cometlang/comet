#ifndef _VARIABLES_H_
#define _VARIABLES_H_

#include "compiler_defs.h"

uint8_t makeConstant(Parser *parser, Value value);
uint8_t identifierConstant(Parser *parser, Token *name);
bool identifiersEqual(Token *a, Token *b);
void markInitialized();
void defineVariable(Parser *parser, uint8_t global);
uint8_t parseVariable(Parser *parser, const char *errorMessage);
void declareVariable(Parser *parser);
void namedVariable(Parser *parser, Token name, bool canAssign);
void variable(Parser *parser, bool canAssign);
int resolveUpvalue(Compiler *compiler, Parser *parser, Token *name);
int resolveLocal(Parser *parser, Compiler *compiler, Token *name);

#endif
