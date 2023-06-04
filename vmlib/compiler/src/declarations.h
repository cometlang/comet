#ifndef _DECLARATIONS_H_
#define _DECLARATIONS_H_
#include "compiler_defs.h"

void method(Parser *parser, uint8_t attributeCount);
void operator_(Parser *parser);
void classDeclaration(Parser *parser, uint8_t attributeCount);
void functionDeclaration(Parser *parser, uint8_t attributeCount);
void enumDeclaration(Parser *parser);
void varDeclaration(Parser *parser);
void function(Parser *parser, FunctionType type, uint8_t attributeCount);

void declaration(Parser *parser);

#endif
