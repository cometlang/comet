#ifndef _MCC_TOKEN_LIST_H_
#define _MCC_TOKEN_LIST_H_

#include "liberal.h"
#include "tokens.h"

/**
 * It hurts to have to expose this typedef in the header file
 * so please don't abuse it by calling the mcc_List* functions directly!
 */
typedef eral_ListIterator_t mcc_TokenListIterator_t;
typedef eral_List_t mcc_TokenList_t;

#if MCC_DEBUG
void mcc_DebugPrintToken(const mcc_Token_t *token);
void mcc_DebugPrintTokenList(mcc_TokenListIterator_t *iter);
#endif

/**
 * @param text      The physical text of the token as in the source file
 *
 * @param text_len  The length of the physical text
 *
 * @param type      The type of token to create
 *
 * @param lineno    The current line number of the file where the token was found
 * 
 * Allocates the memory for the token and returns it to the caller.  The
 * caller then becomes responsible for freeing the memory.  Calling
 * `mcc_TokenListDeleteStandalone` is sufficient - it is assumed the token
 * will be added to a `mcc_TokenList_t`
 */
mcc_Token_t *mcc_CreateToken(const char *text, size_t text_len,
                             TOKEN_TYPE type, int token_index,
                             const unsigned int column,
                             const int lineno, const unsigned short fileno);

/**
 * @param token     The token to copy
 * 
 * @returns a freshly allocated token that contains the content of the original
 * NOTE: does not copy the tokenIndex because I'm lazy.
 */
mcc_Token_t *mcc_CopyToken(const mcc_Token_t *token);

/**
 * @param number   The number for the token
 * @param column   The 1-based index of the line of text where this token is located
 * @param lineno   The 1-based line number in the file where this token is located
 * @param fileno   The file id for the file in which this token is located
 */
mcc_Token_t *mcc_CreateNumberToken(mcc_Number_t *number,
   const unsigned int column, const int lineno, const unsigned short fileno);

/**
 * @param token - the token to delete
 */
void mcc_DeleteToken(uintptr_t token);

/**
 * @param column The index of the line for the first character
 * @param lineno The line number in the file
 * @param fileno The file identifier
 * @param iter   The iterator defining which collection to add the token to
 *
 * Creates an end of Line token and adds it in the current position.
 */
void mcc_AddEndOfLineToken(const unsigned int column, const int lineno, const unsigned short fileno,
                           mcc_TokenListIterator_t *iter);

void mcc_InsertToken(mcc_Token_t *token, mcc_TokenListIterator_t *iter);
const mcc_Token_t *mcc_TokenListPeekCurrentToken(mcc_TokenListIterator_t *iter);
const mcc_Token_t *mcc_TokenListPeekNextToken(mcc_TokenListIterator_t *iter);

/**
 * Starting at the head of the list, work backwards until we find
 * a token that isn't a whitespace token (should be first or second).
 */
const mcc_Token_t *mcc_PeekPreviousNonWhitespaceToken(mcc_TokenListIterator_t *iter);


mcc_TokenList_t *mcc_TokenListCreate(void);
void mcc_TokenListDelete(mcc_TokenList_t *list);
mcc_TokenListIterator_t *mcc_TokenListGetIterator(mcc_TokenList_t *list);

/**
 * Add a token to the end of a list.
 * Instead of requiring an interator to insert at a particular position,
 * just append to the end of the list.  No one needs to deal with an iterator's
 * memory that way.
 * 
 * @param list The list to append to
 * @param token The token to append to the list
 * */
void mcc_TokenListAppend(mcc_TokenList_t *list, mcc_Token_t *token);


/**
 * Deep copies the list - the caller becomes responsible for freeing
 * both the list and the tokens in it.
 *
 * @param list - The list to copy
 */
eral_List_t *mcc_TokenListDeepCopy(mcc_TokenList_t *list);

mcc_TokenListIterator_t *mcc_TokenListCopyIterator(mcc_TokenListIterator_t *iter);
void mcc_TokenListDeleteIterator(mcc_TokenListIterator_t *iter);
mcc_Token_t *mcc_GetNextToken(mcc_TokenListIterator_t *iter);
mcc_Token_t *mcc_GetPreviousToken(mcc_TokenListIterator_t *iter);

void mcc_WriteTokensToOutputFile(mcc_TokenList_t *tokens);

void mcc_TokenListConcatenate(mcc_TokenList_t *dst, mcc_TokenList_t *src);

mcc_Token_t *mcc_TokenListRemoveCurrent(mcc_TokenListIterator_t *iter);

void mcc_TokenListInsertBefore(mcc_TokenListIterator_t *iter, mcc_TokenList_t *toInsert);
#endif /* _MCC_TOKEN_LIST_H_ */

