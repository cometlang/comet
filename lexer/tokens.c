#include <string.h>

/**
 * This is a C File to hold the variables which contain the strings of keywords, operators and symbols
 * This is to make linkage easier than if they were in a header file (which needs to be #include'd
 * in several places)
 */
#include "tokens.h"
#include "liberal.h"
#include "tokenList.h"

const char *token_types[NUM_TOK_TYPES] = {
    "Identifier", "Keyword", "Symbol", "Operator", "Number", "String Constant",
    "Whitespace", "End Of Line"};

/* The following char *[]'s need to be kept in sync with the corresponding enums in tokens.h */

const char *keywords[NUM_KEYWORDS] = {
    "break", "class", "constructor", "continue", "def", "do",
    "else", "enum", "foreach", "if", "return", "static",
    "struct", "while", "try", "catch", "finally", "self"
};

size_t keyword_strlens[NUM_KEYWORDS];

const char *operators[NUM_OPERATORS] = { "*=", "/=", "%=", "+=",
                                         "-=", "<<=", ">>=", "&=", "^=", "|=",
                                         "==", "!=", ">=", "<=", "<<", ">>",
                                         ">", "<", "&&", "||", "!", "&", "|", "^",
                                         "~", "?", ":", ".", "=", ",", "+", "-",
                                         "/", "*", "%" };
size_t operator_strlens[NUM_OPERATORS];

const char *symbols[NUM_SYMBOLS] = { "{", "}", "[", "]", "(", ")", "\"", "'", "\\"};
size_t symbol_strlens[NUM_SYMBOLS];

static bool_t initialised;

static inline void init_tokens(void)
{
   int i;
   for (i = 0; i < NUM_KEYWORDS; i++)
   {
      keyword_strlens[i] = strlen(keywords[i]);
   }

   for (i = 0; i < NUM_SYMBOLS; i++)
   {
      symbol_strlens[i] = strlen(symbols[i]);
   }

   for (i = 0; i < NUM_OPERATORS; i++)
   {
      operator_strlens[i] = strlen(operators[i]);
   }

   initialised = true;
}

static int get_token_impl(eral_LogicalLine_t *line, int num_tokens,
                          const char **token_list, size_t *strlens_list,
                          bool_t requires_non_word_after)
{
   int i;
   if (!initialised)
   {
      init_tokens();
   }

   for (i = 0; i < num_tokens; i++)
   {
      if (strncmp(&line->string[line->index], token_list[i], strlens_list[i]) == 0)
      {
         if (requires_non_word_after) {
            if (strlens_list[i] + line->index < line->length &&
                !isWordChar(line->string[strlens_list[i] + line->index]))
            {
               return i;
            }
         } else {
            return i;
         }
      }
   }
   
   return -1;
}

MCC_SYMBOL mcc_GetSymbol(eral_LogicalLine_t *line)
{
   int sym;

   sym = get_token_impl(line, NUM_SYMBOLS, symbols, symbol_strlens, false);

   if (sym != -1)
   {
      return (MCC_SYMBOL) sym;
   }
   else
   {
      return SYM_NONE;
   }
}


MCC_OPERATOR mcc_GetOperator(eral_LogicalLine_t *line)
{
   int op;

   op = get_token_impl(line, NUM_OPERATORS, operators, operator_strlens, false);

   if (op != -1)
   {
      return (MCC_OPERATOR) op;
   }
   else
   {
      return OP_NONE;
   }
}

MCC_KEYWORD mcc_GetKeyword(eral_LogicalLine_t *line)
{
   int key;

   key = get_token_impl(line, NUM_KEYWORDS, keywords, keyword_strlens, true);

   if (key != -1)
   {
      return (MCC_KEYWORD) key;
   }
   else
   {
      return KEY_NONE;
   }
}

void mcc_ExpectTokenType(const mcc_Token_t *token, TOKEN_TYPE tokenType, int index)
{
   const char *expected;
   switch(tokenType)
   {
      case TOK_OPERATOR:
      {
         expected = operators[index];
      }
      break;
      case TOK_KEYWORD:
      {
         expected = keywords[index];
      }
      break;
      default:
      {
         expected = token_types[tokenType];
      }
      break;
   }
   if (token == NULL)
   {
      fprintf(stderr, "Encountered unexpected end of file while searching for %s\n", expected);
      exit(1);
   }
   if (token->tokenType != tokenType)
   {
      fprintf(stderr,
         "%s:%d:%d Expected '%s', but got '%s' (%s)\n",
         eral_ResolveFileNameFromNumber(token->fileno),
         token->lineno,
         token->line_index,
         expected,
         token_types[token->tokenType],
         token->text);
      exit(1);
   }
}

#if MCC_DEBUG
void mcc_DebugPrintToken_Fn(uintptr_t token_ptr)
{
   mcc_DebugPrintToken((mcc_Token_t *) token_ptr);
}
#endif
