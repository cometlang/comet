#include <string.h>
#include "ast.h"

void delete_ast_node_tree(comet_ASTNode_t *root)
{
    if (root->left)
    {
        delete_ast_node_tree(root->left);
    }
    if (root->middle)
    {
        delete_ast_node_tree(root->middle);
    }
    if (root->right)
    {
        delete_ast_node_tree(root->right);
    }
    free(root);
}

comet_ASTNode_t *ast_node_create(const mcc_Token_t *data)
{
    comet_ASTNode_t *result = (comet_ASTNode_t *)malloc(sizeof(comet_ASTNode_t));
    memset(result, 0, sizeof(comet_ASTNode_t));
    result->data = data;
    return result;
}
