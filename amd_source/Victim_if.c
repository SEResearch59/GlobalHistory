#include <stdio.h>
#include "Victim_if.h"

// training function for attacker, secret leaking function for victim
int func(int secret)
{
    int sum = 0, temp = 3;
    if (secret) {
        sum += 1;
        temp += (temp+1)/(temp+1);
    }
    return sum*temp;
}
