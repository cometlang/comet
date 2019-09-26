#ifndef MCC_TOKENS_H_
#define MCC_TOKENS_H_

#include "liberal.h"

typedef enum TYPE { 
    TOK_IDENTIFIER, TOK_KEYWORD, TOK_SYMBOL, TOK_OPERATOR, TOK_NUMBER,
    TOK_STR_CONST, TOK_WHITESPACE, TOK_EOL, NUM_TOK_TYPES, TOK_NONE } TOKEN_TYPE;
extern const char *token_types[NUM_TOK_TYPES];

typedef enum mcc_NumberType { INTEGER, DOUBLE, NUMBER_OF_NUMBER_TYPES } mcc_NumberType_t;

typedef union mcc_NumberContainer
{
   int32_t integer_s;
   double float_d;
} mcc_NumberContainer_t;

typedef struct mcc_Number
{
   mcc_NumberContainer_t number;
   mcc_NumberType_t numberType;
} mcc_Number_t;

typedef struct token {
   char *text;
   TOKEN_TYPE tokenType;
   int tokenIndex;
   unsigned short fileno;
   int lineno;
   unsigned int line_index;
   mcc_Number_t number;
} mcc_Token_t;

/* The following enums need to be kept in sync with the corresponding char *[] */
typedef enum key_index {
    KEY_BREAK, KEY_CLASS, KEY_CONSTRUCTOR, KEY_CONTINUE, KEY_DEF, KEY_DO, KEY_ELSE,
    KEY_ENUM, KEY_FOREACH, KEY_IF, KEY_RETURN, KEY_STATIC,
    KEY_STRUCT, KEY_WHILE, KEY_TRY, KEY_CATCH, KEY_FINALLY, KEY_SELF, NUM_KEYWORDS, KEY_NONE } MCC_KEYWORD;

extern const char *keywords[NUM_KEYWORDS];
extern size_t keyword_strlens[NUM_KEYWORDS];

typedef enum operator_index {
    OP_TIMES_EQUALS, OP_DIVIDE_EQUALS, OP_MOD_EQUALS, OP_PLUS_EQUALS,
    OP_MINUS_EQUALS, OP_L_SHIFT_EQUALS, OP_R_SHIFT_EQUALS, OP_BITWISE_AND_EQUALS,
    OP_BITWISE_EXCL_OR_EQUALS, OP_BITWISE_INCL_OR_EQUALS, OP_COMPARE_TO,
    OP_NOT_EQUAL, OP_GREATER_EQUAL, OP_LESS_EQUAL, OP_L_SHIFT, OP_R_SHIFT,
    OP_GREATER_THAN, OP_LESS_THAN, OP_LOGICAL_AND, OP_LOGICAL_INCL_OR, OP_NOT,
    OP_BITWISE_AND, OP_BITWISE_INCL_OR, OP_BITWISE_EXCL_OR, OP_NEGATE, OP_TERNARY_IF,
    OP_TERNARY_ELSE, OP_MEMBER_OF, OP_EQUALS_ASSIGN, OP_COMMA, OP_ADD, OP_MINUS,
    OP_DIVIDE, OP_MULTIPLY, OP_MODULO, NUM_OPERATORS, OP_NONE } MCC_OPERATOR;
extern const char *operators[NUM_OPERATORS];
extern size_t operator_strlens[NUM_OPERATORS];

typedef enum symbol_index {SYM_OPEN_BRACE, SYM_CLOSE_BRACE, SYM_OPEN_BRACKET,
                           SYM_CLOSE_BRACKET, SYM_SEMI_COLON, SYM_OPEN_PAREN,
                           SYM_CLOSE_PAREN, SYM_DOUBLE_QUOTE, SYM_SINGLE_QUOTE,
                           SYM_ESCAPE, NUM_SYMBOLS, SYM_NONE} MCC_SYMBOL;
extern const char *symbols[NUM_SYMBOLS];
extern size_t symbol_strlens[NUM_SYMBOLS];

MCC_SYMBOL mcc_GetSymbol(eral_LogicalLine_t *line);
MCC_OPERATOR mcc_GetOperator(eral_LogicalLine_t *line);
MCC_KEYWORD mcc_GetKeyword(eral_LogicalLine_t * line);

void mcc_ExpectTokenType(const mcc_Token_t *token, TOKEN_TYPE tokenType, int index);

#define TOK_UNSET_INDEX -1

#if MCC_DEBUG
void mcc_DebugPrintToken_Fn(uintptr_t token_ptr);
#endif

#endif /* MCC_TOKENS_H_ */
