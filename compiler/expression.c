#include <stdlib.h>
#include <string.h>
#include "expression.h"
#include "liberal.h"
#include "ast.h"
#include "lexer_error.h"

static comet_ASTNode_t *parseCommaExpression(comet_AST_t *tree);
static comet_ASTNode_t *parseConditionalExpression(comet_AST_t *tree);
static comet_ASTNode_t *parseLogicalAndExpression(comet_AST_t *tree);
static comet_ASTNode_t *parseLogicalOrExpression(comet_AST_t *tree);
static comet_ASTNode_t *parseBitwiseExpression(comet_AST_t *tree);
static comet_ASTNode_t *parseNotEqualExpression(comet_AST_t *tree);
static comet_ASTNode_t *parseComparisonExpression(comet_AST_t *tree);
static comet_ASTNode_t *parseStrictComparisonExpression(comet_AST_t *tree);
static comet_ASTNode_t *parseExpression(comet_AST_t *tree);
static comet_ASTNode_t *parseFactor(comet_AST_t *tree);
static comet_ASTNode_t *parseTerm(comet_AST_t *tree);

#define get_number(x) x->number.number.integer_s

#define ICE_Error(tree, format, ...) \
    lexer_TokenPrettyError(tree->currentToken, format, __VA_ARGS__)

static void GetNonWhitespaceToken(comet_AST_t *tree)
{
    mcc_Token_t *token = mcc_GetNextToken(tree->iterator);
    if (token == NULL) {
        tree->currentToken = NULL;
        return;
    }
    if (token->tokenType == TOK_WHITESPACE)
    {
        token = mcc_GetNextToken(tree->iterator);
    }
    tree->currentToken = token;
}

static comet_ASTNode_t *parseFactor(comet_AST_t *tree)
{
    comet_ASTNode_t *result;
    if (tree->currentToken == NULL)
    {
        fprintf(stderr, "Unexpected end of expression\n");
        exit(EXIT_FAILURE);
    }
    if (tree->currentToken->tokenType == TOK_OPERATOR &&
        tree->currentToken->tokenIndex == OP_NOT)
    {
        result = ast_node_create(tree->currentToken);
        GetNonWhitespaceToken(tree);
        result->middle = parseFactor(tree);
        return result;
    }
    else if (tree->currentToken->tokenType == TOK_NUMBER ||
             tree->currentToken->tokenType == TOK_IDENTIFIER)
    {
        result = ast_node_create(tree->currentToken);
        GetNonWhitespaceToken(tree);
        return result;
    }
    else if (tree->currentToken->tokenType == TOK_SYMBOL &&
             tree->currentToken->tokenIndex == SYM_OPEN_PAREN)
    {
        GetNonWhitespaceToken(tree);
        result = parseCommaExpression(tree);
        if (tree->currentToken->tokenType != TOK_SYMBOL &&
            tree->currentToken->tokenIndex != SYM_CLOSE_PAREN)
        {
            ICE_Error(tree,
                "Unmatched parentheses! Expected ')' but got '%s'\n",
                tree->currentToken->text);
        }
        GetNonWhitespaceToken(tree);
        return result;
    }
#if MCC_DEBUG
    mcc_DebugPrintToken(tree->currentToken);
#endif
    ICE_Error(tree,
        "Unknown token in arithmetic expression '%s'\n",
        tree->currentToken->text);
    return NULL;
}

static comet_ASTNode_t *parseTerm(comet_AST_t *tree)
{
    comet_ASTNode_t *node = parseFactor(tree);
    while (tree->currentToken &&
        tree->currentToken->tokenType == TOK_OPERATOR &&
        (tree->currentToken->tokenIndex == OP_MULTIPLY ||
         tree->currentToken->tokenIndex == OP_DIVIDE))
    {
        comet_ASTNode_t *op_node = ast_node_create(tree->currentToken);
        GetNonWhitespaceToken(tree);
        op_node->left = node;
        op_node->right = parseFactor(tree);
        node = op_node;
    }

    return node;
}

static comet_ASTNode_t *parseExpression(comet_AST_t *tree)
{
    comet_ASTNode_t *node = parseTerm(tree);
    while (tree->currentToken &&
        tree->currentToken->tokenType == TOK_OPERATOR &&
        (tree->currentToken->tokenIndex == OP_ADD ||
         tree->currentToken->tokenIndex == OP_MINUS))
    {
        comet_ASTNode_t *op_node = ast_node_create(tree->currentToken);
        GetNonWhitespaceToken(tree);
        op_node->left = node;
        op_node->right = parseTerm(tree);
        node = op_node;
    }

    return node;
}

static comet_ASTNode_t *parseStrictComparisonExpression(comet_AST_t *tree)
{
    comet_ASTNode_t *node = parseExpression(tree);
    while (tree->currentToken &&
        tree->currentToken->tokenType == TOK_OPERATOR &&
        (tree->currentToken->tokenIndex == OP_LESS_THAN ||
         tree->currentToken->tokenIndex == OP_GREATER_THAN))
    {
        comet_ASTNode_t *op_node = ast_node_create(tree->currentToken);
        GetNonWhitespaceToken(tree);
        op_node->left = node;
        op_node->right = parseExpression(tree);
        node = op_node;
    }

    return node;
}

static comet_ASTNode_t *parseComparisonExpression(comet_AST_t *tree)
{
    comet_ASTNode_t *node = parseStrictComparisonExpression(tree);
    while (tree->currentToken &&
        tree->currentToken->tokenType == TOK_OPERATOR &&
        (tree->currentToken->tokenIndex == OP_LESS_EQUAL ||
         tree->currentToken->tokenIndex == OP_GREATER_EQUAL ||
         tree->currentToken->tokenIndex == OP_COMPARE_TO))
    {
        comet_ASTNode_t *op_node = ast_node_create(tree->currentToken);
        GetNonWhitespaceToken(tree);
        op_node->left = node;
        op_node->right = parseStrictComparisonExpression(tree);
        node = op_node;
    }

    return node;
}

static comet_ASTNode_t *parseNotEqualExpression(comet_AST_t *tree)
{
    comet_ASTNode_t *node = parseComparisonExpression(tree);
    while (tree->currentToken &&
           tree->currentToken->tokenType == TOK_OPERATOR &&
           tree->currentToken->tokenIndex == OP_NOT_EQUAL)
    {
        comet_ASTNode_t *op_node = ast_node_create(tree->currentToken);
        GetNonWhitespaceToken(tree);
        op_node->left = node;
        op_node->right = parseComparisonExpression(tree);
        node = op_node;
    }

    return node;
}

static comet_ASTNode_t *parseBitwiseExpression(comet_AST_t *tree)
{
    comet_ASTNode_t *node = parseNotEqualExpression(tree);
    while (tree->currentToken &&
        tree->currentToken->tokenType == TOK_OPERATOR &&
        (tree->currentToken->tokenIndex == OP_BITWISE_AND ||
         tree->currentToken->tokenIndex == OP_BITWISE_EXCL_OR ||
         tree->currentToken->tokenIndex == OP_BITWISE_INCL_OR))
    {
        comet_ASTNode_t *op_node = ast_node_create(tree->currentToken);
        GetNonWhitespaceToken(tree);
        op_node->left = node;
        op_node->right = parseNotEqualExpression(tree);
        node = op_node;
    }

    return node;
}

static comet_ASTNode_t *parseLogicalAndExpression(comet_AST_t *tree)
{
    comet_ASTNode_t *node = parseBitwiseExpression(tree);
    while (tree->currentToken &&
        tree->currentToken->tokenType == TOK_OPERATOR &&
        tree->currentToken->tokenIndex == OP_LOGICAL_AND)
    {
        comet_ASTNode_t *op_node = ast_node_create(tree->currentToken);
        GetNonWhitespaceToken(tree);
        op_node->left = node;
        op_node->right = parseBitwiseExpression(tree);
        node = op_node;
    }

    return node;
}

static comet_ASTNode_t *parseLogicalOrExpression(comet_AST_t *tree)
{
    comet_ASTNode_t *node = parseLogicalAndExpression(tree);
    while (tree->currentToken &&
        tree->currentToken->tokenType == TOK_OPERATOR &&
        tree->currentToken->tokenIndex == OP_LOGICAL_INCL_OR)
    {
        comet_ASTNode_t *op_node = ast_node_create(tree->currentToken);
        GetNonWhitespaceToken(tree);
        op_node->left = node;
        op_node->right = parseLogicalAndExpression(tree);
        node = op_node;
    }

    return node;
}

static comet_ASTNode_t *parseConditionalExpression(comet_AST_t *tree)
{
    comet_ASTNode_t *node = parseLogicalOrExpression(tree);
    while (tree->currentToken &&
           tree->currentToken->tokenType == TOK_OPERATOR &&
           tree->currentToken->tokenIndex == OP_TERNARY_IF)
    {
        comet_ASTNode_t *op_node = ast_node_create(tree->currentToken);
        GetNonWhitespaceToken(tree);
        op_node->left = node;
        op_node->middle = parseCommaExpression(tree);
        if (tree->currentToken->tokenType != TOK_OPERATOR &&
            tree->currentToken->tokenIndex != OP_TERNARY_ELSE)
        {
            ICE_Error(tree, "Expected ':' after '?' instead of '%s'\n", tree->currentToken->text);
        }
        GetNonWhitespaceToken(tree);
        op_node->right = parseConditionalExpression(tree);
        node = op_node;
    }

    return node;
}

static comet_ASTNode_t *parseCommaExpression(comet_AST_t *tree)
{
    comet_ASTNode_t *node = parseConditionalExpression(tree);
    if (tree->currentToken &&
        tree->currentToken->tokenType == TOK_OPERATOR &&
        tree->currentToken->tokenIndex == OP_COMMA)
    {
        GetNonWhitespaceToken(tree);
    }
    return node;
}

static comet_AST_t *create_syntax_tree(mcc_TokenListIterator_t *iter)
{
    comet_AST_t *result = (comet_AST_t *) malloc(sizeof(comet_AST_t));
    result->numbers_to_delete = eral_ListCreate();
    result->root = NULL;
    result->currentToken = NULL;
    result->iterator = iter;
    return result;
}

void mcc_DeleteAST(comet_AST_t *tree)
{
    eral_ListIterator_t *number_iter = eral_ListGetIterator(tree->numbers_to_delete);
    uintptr_t death_row = eral_ListGetNextData(number_iter);
    while(death_row != NULL_DATA)
    {
        mcc_DeleteToken(death_row);
        death_row = eral_ListGetNextData(number_iter);
    }
    eral_ListDeleteIterator(number_iter);
    eral_ListDelete(tree->numbers_to_delete, NULL);
    delete_ast_node_tree(tree->root);
    free(tree);
}

comet_AST_t *mcc_ParseExpression(mcc_TokenListIterator_t *iter)
{
    comet_AST_t *tree = create_syntax_tree(iter);
    tree->currentToken = mcc_TokenListPeekCurrentToken(iter);
    tree->root = parseCommaExpression(tree);
    return tree;
}
