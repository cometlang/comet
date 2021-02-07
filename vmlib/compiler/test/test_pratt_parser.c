#include "expressions.h"

extern ParseRule rules[NUM_TOKENS];

void test_rulesAreComplete(void)
{
    for (int i = 0; i < NUM_TOKENS; i++)
    {
        DEBUG_ASSERT(rules[i].precedence >= PREC_NONE &&
                     rules[i].precedence < PREC_PRIMARY);
    }
}
