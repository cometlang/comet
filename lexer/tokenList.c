#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "tokens.h"
#include "tokenList.h"
#include "liberal.h"

static const char whitespaceText = ' ';

mcc_Token_t *mcc_CreateToken(const char *text, size_t text_len,
                             TOKEN_TYPE type, int token_index,
                             const unsigned int column,
                             const int lineno, const unsigned short fileno)
{
   mcc_Token_t *token = (mcc_Token_t *) malloc(sizeof(mcc_Token_t));
   token->text = (char *) malloc(sizeof(char) * (text_len + 1));
   memcpy(token->text, text, text_len);
   token->text[text_len] = '\0';
   token->tokenType = type;
   token->tokenIndex = token_index;
   token->line_index = column;
   token->lineno = lineno;
   token->fileno = fileno;

   return token;
}

mcc_Token_t *mcc_CopyToken(const mcc_Token_t *token)
{
   mcc_Token_t *result = mcc_CreateToken(
      token->text, strlen(token->text), token->tokenType,
      token->tokenIndex, token->line_index, token->lineno, token->fileno);

   if (result->tokenType == TOK_NUMBER)
   {
      result->number = token->number;
   }
   return result;
}

mcc_Token_t *mcc_CreateNumberToken(mcc_Number_t *number,
   const unsigned int column, const int lineno, const unsigned short fileno)
{
   char numberText[20] = {0};
   snprintf(numberText, 20, "%d", number->number.integer_s);
   mcc_Token_t *result = mcc_CreateToken(
      numberText, strlen(numberText), TOK_NUMBER, TOK_UNSET_INDEX,
      column, lineno, fileno);
   memcpy(&result->number, number, sizeof(*number));
   return result;
}

mcc_TokenList_t *mcc_TokenListDeepCopy(mcc_TokenList_t *list)
{
   mcc_TokenList_t *result = mcc_TokenListCreate();
   mcc_TokenListIterator_t *iter = mcc_TokenListGetIterator(list);
   mcc_Token_t *token = mcc_GetNextToken(iter);
   while (token != NULL)
   {
      mcc_TokenListAppend(result, mcc_CopyToken(token));
      token = mcc_GetNextToken(iter);
   }
   mcc_TokenListDeleteIterator(iter);
   return result;
}

void mcc_AddEndOfLineToken(const unsigned int column, const int lineno, const unsigned short fileno,
                           mcc_TokenListIterator_t *iter)
{
   mcc_Token_t *token = mcc_CreateToken(&whitespaceText, sizeof(whitespaceText),
                                        TOK_EOL, TOK_UNSET_INDEX, column, lineno, fileno);
   mcc_InsertToken(token, iter);   
}

void mcc_DeleteToken(uintptr_t token)
{
   mcc_Token_t *temp = (mcc_Token_t *) token;
   free(temp->text);
   free(temp);
}

void mcc_InsertToken(mcc_Token_t *token, mcc_TokenListIterator_t *iter)
{
   eral_ListInsertDataAtCurrentPosition((eral_ListIterator_t *) iter, (uintptr_t) token);
}

const mcc_Token_t *mcc_TokenListPeekCurrentToken(mcc_TokenListIterator_t *iter)
{
   return (const mcc_Token_t *) eral_ListPeekCurrentData((eral_ListIterator_t *) iter);
}

const mcc_Token_t *mcc_TokenListPeekNextToken(mcc_TokenListIterator_t *iter)
{
   return (const mcc_Token_t *) eral_ListPeekNextData((eral_ListIterator_t *)iter);
}

void mcc_TokenListDeleteIterator(mcc_TokenListIterator_t *iter)
{
   eral_ListDeleteIterator((eral_ListIterator_t *) iter);
}

mcc_TokenListIterator_t *mcc_TokenListGetIterator(mcc_TokenList_t *list)
{
   return (mcc_TokenListIterator_t *) eral_ListGetIterator((eral_List_t *)list);
}

void mcc_TokenListAppend(mcc_TokenList_t *list, mcc_Token_t *token)
{
   eral_ListAppendData((eral_List_t *)list, (uintptr_t)token);
}

mcc_TokenListIterator_t *mcc_TokenListCopyIterator(mcc_TokenListIterator_t *iter)
{
   return (mcc_TokenListIterator_t *) eral_ListCopyIterator((eral_ListIterator_t *) iter);
}

mcc_Token_t *mcc_GetNextToken(mcc_TokenListIterator_t *iter)
{
   return (mcc_Token_t *) eral_ListGetNextData((eral_ListIterator_t *) iter);
}

mcc_Token_t *mcc_GetPreviousToken(mcc_TokenListIterator_t *iter)
{
   return (mcc_Token_t *) eral_ListGetPrevData((eral_ListIterator_t *) iter);
}

const mcc_Token_t *mcc_PeekPreviousNonWhitespaceToken(mcc_TokenListIterator_t *iter)
{
   const mcc_Token_t *tok = (mcc_Token_t *) eral_ListPeekCurrentData(iter);
   if (tok && tok->tokenType != TOK_WHITESPACE)
      return tok;
   mcc_TokenListIterator_t *temp = eral_ListCopyIterator(iter);
   while (tok && tok->tokenType == TOK_WHITESPACE)
      tok = (mcc_Token_t *) eral_ListGetPrevData(temp);
   eral_ListDeleteIterator(temp);
   return tok;
}

mcc_TokenList_t *mcc_TokenListCreate(void)
{
   return (mcc_TokenList_t*) eral_ListCreate();
}

void mcc_TokenListDelete(mcc_TokenList_t *list)
{
   eral_ListDelete(list, &mcc_DeleteToken);
}

void mcc_TokenListConcatenate(mcc_TokenList_t *dst, mcc_TokenList_t *src)
{
   eral_ListConcatenate((eral_List_t *)dst, (eral_List_t *)src);
}

mcc_Token_t *mcc_TokenListRemoveCurrent(mcc_TokenListIterator_t *iter)
{
   return (mcc_Token_t *) eral_ListRemoveCurrentData((eral_ListIterator_t *)iter);
}

char *escape_string(const char *input)
{
   eral_StringBuffer_t *buffer = eral_CreateStringBuffer();
   int i;
   int len = strlen(input);
   for (i = 0; i < len; i++)
   {
      switch (input[i])
      {
         case '\a':
            eral_StringBufferAppendString(buffer, "\\a");
            break;
         case '\b':
            eral_StringBufferAppendString(buffer, "\\b");
            break;
         case '\t':
            eral_StringBufferAppendString(buffer, "\\t");
            break;
         case '\n':
            eral_StringBufferAppendString(buffer, "\\n");
            break;
         case '\v':
            eral_StringBufferAppendString(buffer, "\\v");
            break;
         case '\f':
            eral_StringBufferAppendString(buffer, "\\f");
            break;
         case '\r':
            eral_StringBufferAppendString(buffer, "\\r");
            break;
         case '\\':
            eral_StringBufferAppendString(buffer, "\\\\");
            break;
         case '"':
            eral_StringBufferAppendString(buffer, "\\\"");
            break;
         default:
            eral_StringBufferAppendChar(buffer, input[i]);
            break;
      }
   }
   return eral_DestroyBufferNotString(buffer);
}

void mcc_WriteTokensToOutputFile(mcc_TokenList_t *tokens)
{
   FILE *outf = fopen("output.cmt", "w+");
   mcc_TokenListIterator_t *iter = NULL;
   mcc_Token_t *tok = NULL;
   char *escaped = NULL;
   if (outf == NULL)
   {
      printf("Couldn't open output file 'output.cmt' for writing\n");
      exit(1);
   }
   iter = mcc_TokenListGetIterator(tokens);
   tok = mcc_GetNextToken(iter);
   while (tok != NULL)
   {
      switch (tok->tokenType)
      {
         case TOK_IDENTIFIER:
         case TOK_KEYWORD:
         case TOK_SYMBOL:
         case TOK_OPERATOR:
         case TOK_NUMBER:
         case TOK_WHITESPACE:
            fprintf(outf, "%s", tok->text);
            break;
         case TOK_STR_CONST:
            escaped = escape_string(tok->text);
            fprintf(outf, "\"%s\"", escaped);
            free(escaped);
            break;
         case TOK_EOL:
            fprintf(outf, "\n");
         default:
            break;
      }
      tok = mcc_GetNextToken(iter);
   }
   fclose(outf);
}

#if MCC_DEBUG
void mcc_DebugPrintToken(const mcc_Token_t *token)
{
   if (token != NULL)
   {
      printf("\t----- Token -----\n\tText: %s\n\tType: %s\n\tIndex: %d\n\tPosition: %s:%d:%u\n",
             token->text, token_types[token->tokenType], token->tokenIndex,
             mcc_ResolveFileNameFromNumber(token->fileno), token->lineno, token->line_index);
   }
   else
   {
      printf("Token was NULL\n");
   }
}

void mcc_DebugPrintTokenList(mcc_TokenListIterator_t *iter)
{
   mcc_TokenListIterator_t *copy = mcc_TokenListCopyIterator(iter);
   mcc_Token_t *token = (mcc_Token_t *)mcc_TokenListPeekCurrentToken(copy);
   if (token == NULL)
      token = mcc_GetNextToken(copy);
   printf("----- Token List -----\n");
   printf("Num Tokens: %d\n", eral_ListIteratorGetLength(iter));
   while (token != NULL)
   {
      mcc_DebugPrintToken(token);
      token = mcc_GetNextToken(copy);
   }
   printf("----- End Token List -----\n");
   mcc_TokenListDeleteIterator(copy);
}
#endif
