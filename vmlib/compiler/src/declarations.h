#ifndef _DECLARATIONS_H_
#define _DECLARATIONS_H_
#include "compiler_defs.h"

void method(Parser *parser);
void operator_(Parser *parser);
void classDeclaration(Parser *parser);
void functionDeclaration(Parser *parser);
void enumDeclaration(Parser *parser);
void varDeclaration(Parser *parser);
void function(Parser *parser, FunctionType type);

void declaration(Parser *parser);

#endif
