/*
 * integer_math.c
 *
 *  Created on: May 1, 2014
 *      Author: andres
 */

#include "app_global.h"
#include "integer_math.h"

// FIXME there are probably much better ways to do these!

// TODO: Unit tests for these functions

S32_T float2fixed(R32_T val)
{
    return (S32_T) (val*(1 << FRACTIONALBITS));
}

R32_T fixed2float(S32_T val)
{
    return ((R32_T) val)/(1 << FRACTIONALBITS);
}

S32_T fixp_abs(S32_T input) {
    if (input & 0x80000000) {
        if (input == 0x80000001) {
            input = 0x80000010; // Clip largest negative value
        }
        return (~input) + 1;
    } else {
        return input;
    }
}

S32_T fixp_mult(S32_T n1, S32_T n2)
{
    S64_T val = (S64_T)n1*(S64_T)n2;

    return val >> FRACTIONALBITS; // TODO check for overflow?
}
