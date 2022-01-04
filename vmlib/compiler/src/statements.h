#ifndef _STATEMENTS_H_
#define _STATEMENTS_H_

#include "compiler_defs.h"

#define UNINITALISED_ADDRESS -1

void block(Parser *parser);

void expressionStatement(Parser *parser);
void forStatement(Parser *parser);
void foreachStatement(Parser *parser);
void ifStatement(Parser *parser);
void returnStatement(Parser *parser);
void whileStatement(Parser *parser);
void tryStatement(Parser *parser);
void throwStatement(Parser *parser);
void importStatement(Parser *parser);
void nextStatement(Parser *parser);

void statement(Parser *parser);

#endif
