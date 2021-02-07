#ifndef _EMITTER_H_
#define _EMITTER_H_

#include "compiler_defs.h"

void emitByte(Parser *parser, uint8_t byte);
void emitBytes(Parser *parser, uint8_t byte1, uint8_t byte2);
void emitLoop(Parser *parser);
int emitJump(Parser *parser, uint8_t instruction);
void emitReturn(Parser *parser);
void emitConstant(Parser *parser, Value value);

#endif
