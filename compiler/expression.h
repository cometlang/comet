#ifndef _COMET_COMPILER_EXPRESSION_H_
#define _COMET_COMPILER_EXPRESSION_H_

#include "tokenList.h"
#include "ast.h"

comet_AST_t *mcc_ParseExpression(mcc_TokenListIterator_t *iter);

void mcc_DeleteAST(comet_AST_t *tree);

#endif /* _COMET_COMPILER_EXPRESSION_H_ */
