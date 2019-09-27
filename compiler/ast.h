#ifndef _COMET_COMPILER_AST_H
#define _COMET_COMPILER_AST_H
#include "tokenList.h"

typedef struct ASTNode
{
    struct ASTNode *left;
    struct ASTNode *middle;
    struct ASTNode *right;
    const mcc_Token_t *data;
} comet_ASTNode_t;

typedef struct syntax_tree {
    comet_ASTNode_t *root;
    mcc_TokenListIterator_t *iterator;
    const mcc_Token_t *currentToken;
    eral_List_t *numbers_to_delete;
} comet_AST_t;

comet_ASTNode_t *ast_node_create(const mcc_Token_t *data);

void delete_ast_node_tree(comet_ASTNode_t *root);

#endif
