#include <stdio.h>
#include "Victim.h"

// training function for attacker, secret leaking function for victim
int func(int length)
{
    int i, sum = 0;
    for (i = 0; i < length; i++)
        sum++;
    return sum;
}
