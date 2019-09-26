#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "liberal.h"
#include "tokens.h"
#include "tokenList.h"

static bool_t insideMultiLineComment = false;

static void eral_TokeniseLine(eral_LogicalLine_t *line,
                             eral_FileBuffer_t *fileBuffer,
                             mcc_TokenListIterator_t *iter);

static void handle_string_const(eral_LogicalLine_t *line,
                                eral_FileBuffer_t *fileBuffer,
                                mcc_TokenListIterator_t *iter);
static char handle_octal_integer_const(eral_LogicalLine_t *line,
                                       eral_FileBuffer_t *fileBuffer);
static char handle_hex_integer_const(eral_LogicalLine_t *line,
                                     eral_FileBuffer_t *fileBuffer);
static void handle_whitespace(eral_LogicalLine_t *line,
                              eral_FileBuffer_t *fileBuffer,
                              mcc_TokenListIterator_t *iter);

static void pretty_error(eral_FileBuffer_t *file, unsigned int index, const char *format, ...);

static void handle_whitespace(eral_LogicalLine_t *line,
                              eral_FileBuffer_t *fileBuffer,
                              mcc_TokenListIterator_t *iter)
{
   eral_StringBuffer_t *buffer = eral_CreateStringBuffer();
   int lineCol = line->index + 1;
   int numChars = 0;
   while ((line->index < line->length) &&
          (isNonBreakingWhiteSpace((line->string[line->index]))))
   {
      eral_StringBufferAppendChar(buffer, line->string[line->index]);
      line->index++;
      numChars++;
   }
   if (numChars > 0)
   {
      const mcc_Token_t *temp = mcc_TokenListPeekCurrentToken(iter);
      if (temp != NULL && temp->tokenType != TOK_WHITESPACE)
      {
         mcc_Token_t *token = mcc_CreateToken(
            eral_StringBufferGetString(buffer),
            eral_GetStringBufferLength(buffer),
            TOK_WHITESPACE,
            TOK_UNSET_INDEX,
            lineCol,
            eral_GetFileBufferCurrentLineNo(fileBuffer),
            eral_GetFileBufferFileNumber(fileBuffer));
         mcc_InsertToken(token, iter);
      }
   }
   eral_DeleteStringBuffer(buffer);
}

static void handle_string_const(eral_LogicalLine_t *line,
                                eral_FileBuffer_t *fileBuffer,
                                mcc_TokenListIterator_t *iter)
{
   int strLen = 0;
   mcc_Token_t *token = NULL;

   //move the line past the initial delimiter
   line->index++;

   while(line->string[line->index + strLen] != '"')
   {
      if ((line->index + strLen) == line->length)
      {
         //The fileBuffer should have already found all the line continuations
         //and we should be able to handle all string constants as a single logical line.
         pretty_error(fileBuffer, line->index, "Reached the end of line when parsing a string \
constant and there's no continuation character\n");
      }
      else if (line->string[line->index + strLen] == '\\')
      {
         eral_ShiftLineLeftAndShrink(line, line->index + strLen, 1);
         switch (line->string[line->index + strLen])
         {
            case 'a':
               line->string[line->index + strLen] = '\a';
               break;
            case 'b':
               line->string[line->index + strLen] = '\b';
               break;
            case 't':
               line->string[line->index + strLen] = '\t';
               break;
            case 'n':
               line->string[line->index + strLen] = '\n';
               break;
            case 'v':
               line->string[line->index + strLen] = '\v';
               break;
            case 'f':
               line->string[line->index + strLen] = '\f';
               break;
            case 'r':
               line->string[line->index + strLen] = '\r';
               break;
            case '\\':
               line->string[line->index + strLen] = '\\';
               break;
            case '\'':
               line->string[line->index + strLen] = '\'';
               break;
            case '"':
               line->string[line->index + strLen] = '"';
               break;
            case '0':
               line->string[line->index + strLen] = handle_octal_integer_const(line, fileBuffer);
               break;
            case 'x':
               line->string[line->index + strLen] = handle_hex_integer_const(line, fileBuffer);
               break;
            default:
               pretty_error(fileBuffer, line->index,
                  "Unrecognised escape sequence '\\%c' when parsing string constant\n",
                  line->string[line->index + strLen]);
               break;
         }
      }
      strLen++;
   }
   token = mcc_CreateToken(&line->string[line->index],
                           strLen, TOK_STR_CONST,
                           TOK_UNSET_INDEX,
                           line->index+1,
                           eral_GetFileBufferCurrentLineNo(fileBuffer),
                           eral_GetFileBufferFileNumber(fileBuffer));
   line->index += strLen + 1;
   mcc_InsertToken(token, iter);
}

static char handle_octal_integer_const(eral_LogicalLine_t *line,
                                       eral_FileBuffer_t *fileBuffer)
{
   int intLen = 0, j;
   long long number = 0;
   unsigned long long mask = 0xFF;

   while (isOctalChar(line->string[line->index + intLen]))
   {
      intLen++;
   }   
   for (j = 0; j < intLen; j++)
   {
      number += (line->string[line->index + j] - '0') * (long long)pow(8, intLen-j-1);
   }
   if (number != (long long)(number & mask))
   {
      pretty_error(fileBuffer, line->index,
                   "Octal integer constant '%lld' exceeds allowed precision\n",
                   number);
   }
   eral_ShiftLineLeftAndShrink(line, line->index, intLen-1);
   return (char) number;
}

static char convertHexCharToNum(char character)
{
   character = (char) toupper(character);
   if (character >= '0' && character <= '9')
   {
      return character - '0';
   }
   else if (character >= 'A' && character <= 'F')
   {
      return (character - 'A') + 10;
   }
   return 0;
}

static char handle_hex_integer_const(eral_LogicalLine_t *line,
                                     eral_FileBuffer_t *fileBuffer)
{
   int intLen = 1, j;
   long long number = 0;

   while (isHexChar(line->string[line->index + intLen]))
   {
      intLen++;
   }

   for (j = 1; j < intLen; j++)
   {
      number |= convertHexCharToNum(line->string[line->index + j]) << ((intLen - j - 1) * 4);
   }

   if ((long long)(number & 0xFF) != number)
   {
      char *tempStr = (char *) malloc((intLen + 1) * sizeof(char));
      memcpy(tempStr, &line->string[line->index + 1], intLen-1);
      tempStr[intLen-1] = 0;
      pretty_error(fileBuffer, line->index,
                   "hexadecimal integer constant '\\x%s' exceeds allowed precision\n",
                   tempStr);
   }

   eral_ShiftLineLeftAndShrink(line, line->index, intLen-2);
   return (char) number;
}
                                       
mcc_TokenList_t *lex_TokeniseFile(const char *inFilename)
{
   eral_FileBuffer_t *fileBuffer = eral_CreateFileBuffer(inFilename);
   mcc_TokenList_t *tokens = mcc_TokenListCreate();
   mcc_TokenListIterator_t *iter = mcc_TokenListGetIterator(tokens);
   eral_LogicalLine_t *logicalLine = NULL;

   while(!eral_FileBufferEOFReached(fileBuffer))
   {
      logicalLine = eral_FileBufferGetNextLogicalLine(fileBuffer);
      if ((logicalLine->length > 0) && (logicalLine->index < logicalLine->length))
      {
         eral_TokeniseLine(logicalLine, fileBuffer, iter);
      }
      if (!insideMultiLineComment)
      {
         mcc_AddEndOfLineToken(
            logicalLine->index+1,
            eral_GetFileBufferCurrentLineNo(fileBuffer),
            eral_GetFileBufferFileNumber(fileBuffer),
            iter);
      }
   }
   eral_DeleteFileBuffer(fileBuffer);
   mcc_TokenListDeleteIterator(iter);
   return tokens;
}

//by providing a logical line, we are guaranteed to only have whole tokens.
static void eral_TokeniseLine(eral_LogicalLine_t *line,
                             eral_FileBuffer_t *fileBuffer,
                             mcc_TokenListIterator_t *iter)
{
   MCC_OPERATOR current_operator = OP_NONE;
   MCC_SYMBOL current_symbol = SYM_NONE;
   mcc_Token_t *token = NULL;

   if (!insideMultiLineComment)
   {
      handle_whitespace(line, fileBuffer, iter);
   }

   while(line->index < line->length)
   {
      if ((line->string[line->index] == '/' &&
           line->string[line->index+1] == '*') || insideMultiLineComment)
      {
         //If this is the first line/opening part of the comment, handle it.
         if (!insideMultiLineComment)
         {
            insideMultiLineComment = true;
            line->index += 2;
         }
         while(line->index < line->length)
         {
            if (line->index < (line->length-1) &&
                (line->string[line->index] == '*' && line->string[line->index+1] == '/'))
            {
               line->index += 2;
               insideMultiLineComment = false;
               break;
            }
            else
            {
               line->index++;
            }
         }
      }
      else if (line->string[line->index] == '/' &&
               line->string[line->index+1] == '/')
      {
         //consider creating a token for the comment, such that we can preserve them in preprocessed output
         while(line->index < line->length)
            line->index++;
      }
      else if ((current_symbol = mcc_GetSymbol(line)) != SYM_NONE)
      {
         if (current_symbol == SYM_DOUBLE_QUOTE)
         {
            handle_string_const(line, fileBuffer, iter);
         }
         else
         {
            token = mcc_CreateToken(symbols[current_symbol],
                                    symbol_strlens[current_symbol],
                                    TOK_SYMBOL,
                                    current_symbol,
                                    line->index+1,
                                    eral_GetFileBufferCurrentLineNo(fileBuffer),
                                    eral_GetFileBufferFileNumber(fileBuffer));
            line->index += symbol_strlens[current_symbol];
         }
      }
      else if ((current_operator = mcc_GetOperator(line)) != OP_NONE)
      {
         token = mcc_CreateToken(operators[current_operator],
                                 operator_strlens[current_operator], 
                                 TOK_OPERATOR,
                                 current_operator,
                                 line->index+1,
                                 eral_GetFileBufferCurrentLineNo(fileBuffer),
                                 eral_GetFileBufferFileNumber(fileBuffer));
         line->index += operator_strlens[current_operator];
      }
      else if (isNumber(line->string[line->index]))
      {
         bool_t isDouble = false;
         int numLen = 1;
         while (isNumber(line->string[line->index + numLen]) &&
                 line->index + numLen < line->length)
         {
            numLen++;
         }
         if (toupper(line->string[line->index + numLen]) == 'E')
         {
            while (isNumber(line->string[line->index + numLen]) &&
                   line->index + numLen < line->length)
            {
               numLen++;
            }
         }
         else if (line->string[line->index + numLen] == '.')
         {
            while (isNumber(line->string[line->index + numLen]) &&
                   line->index + numLen < line->length)
            {
               numLen++;
            }
            isDouble = true;
         }
         
         token = mcc_CreateToken(&line->string[line->index], numLen,
                                 TOK_NUMBER,
                                 TOK_UNSET_INDEX,
                                 line->index+1,
                                 eral_GetFileBufferCurrentLineNo(fileBuffer),
                                 eral_GetFileBufferFileNumber(fileBuffer));
         if (isDouble)
         {
            token->number.number.float_d = strtod(token->text, NULL);
            token->number.numberType = DOUBLE;
         }
         else
         {
            token->number.number.integer_s = strtol(token->text, NULL, 10);
            token->number.numberType = INTEGER;
         }
         line->index += numLen;
      }
      else if (isWordChar(line->string[line->index]))
      {
         MCC_KEYWORD keyword = mcc_GetKeyword(line);
         if (keyword != KEY_NONE)
         {
            token = mcc_CreateToken(keywords[keyword], keyword_strlens[keyword],
                                    TOK_KEYWORD,
                                    keyword,
                                    line->index+1,
                                    eral_GetFileBufferCurrentLineNo(fileBuffer),
                                    eral_GetFileBufferFileNumber(fileBuffer));
            line->index += keyword_strlens[keyword];
         }
         else //it's an identifier
         {
            int identLen = 1;
            while ((isWordChar(line->string[line->index + identLen]) ||
                    isNumber(line->string[line->index + identLen])) &&
                   ((line->index + identLen) < line->length))
            {
               identLen++;
            }
            //This is where I add support for ! or ? trailing function (identifier?) names
            token = mcc_CreateToken(&line->string[line->index], identLen,
                                    TOK_IDENTIFIER,
                                    TOK_UNSET_INDEX,
                                    line->index+1,
                                    eral_GetFileBufferCurrentLineNo(fileBuffer),
                                    eral_GetFileBufferFileNumber(fileBuffer));
            line->index += identLen;
         }
      }
      else
      {
         pretty_error(fileBuffer, line->index,
                      "Not a recognised character: '%c'.  This is probably a bug in the tokeniser.\n",
                      line->string[line->index]);
      }
     
      if (token != NULL)
      {
         mcc_InsertToken(token, iter);
         token = NULL;
      }
      handle_whitespace(line, fileBuffer, iter);
   }
}

void pretty_error(eral_FileBuffer_t *file, unsigned int index, const char *format, ...)
{
   va_list args;
   fprintf(stderr, "%s:%d:%d ", eral_GetFileBufferFilename(file),
      eral_GetFileBufferCurrentLineNo(file), index);
   va_start(args, format);
   fprintf(stderr, format, args);
   va_end(args);
   exit(1);
}