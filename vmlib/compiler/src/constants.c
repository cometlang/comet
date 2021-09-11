#include <stdlib.h>

#include "mem.h"
#include "constants.h"
#include "comet.h"


Value parseNumber(Parser *parser)
{
    char *number_chars = ALLOCATE(char, parser->previous.length + 1);
    int offset = 0;
    for (int i = 0; i < parser->previous.length; i++)
    {
        if (parser->previous.start[i] != '_')
            number_chars[offset++] = parser->previous.start[i];
    }
    number_chars[offset] = '\0';

    double value = strtod(number_chars, NULL);
    FREE_ARRAY(char, number_chars, parser->previous.length + 1);
    return create_number(parser->compilation_thread, value);
}

Value parseString(Parser *parser)
{
    int index = 0;
    char *string_chars = ALLOCATE(char, parser->previous.length - 1);
    for (int i = 1; i < parser->previous.length - 1; i++)
    {
        if (parser->previous.start[i] == '\\')
        {
            if (parser->previous.start[i+1] == 'n')
            {
                string_chars[index++] = '\n';
                i++;
            }
            else if (parser->previous.start[i+1] == '\\')
            {
                string_chars[index++] = '\\';
                i++;
            }
            else if (parser->previous.start[i+1] == 't')
            {
                string_chars[index++] = '\t';
                i++;
            }
            else if (parser->previous.start[i+1] == 'r')
            {
                string_chars[index++] = '\r';
                i++;
            }
            else if (parser->previous.start[i+1] == '\'')
            {
                string_chars[index++] = '\'';
                i++;
            }
            else if (parser->previous.start[i+1] == '"')
            {
                string_chars[index++] = '"';
                i++;
            }
            else
            {
                string_chars[index++] = parser->previous.start[i];
            }
        }
        else
        {
            string_chars[index++] = parser->previous.start[i];
        }
    }
    string_chars[index] = '\0';
    return takeString(parser->compilation_thread, string_chars, index);
}