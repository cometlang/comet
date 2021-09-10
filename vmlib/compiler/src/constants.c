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
    return copyString(parser->compilation_thread,
                      parser->previous.start + 1,
                      parser->previous.length - 2);
}