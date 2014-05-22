/*
 * integer_math.h
 *
 *  Created on: May 1, 2014
 *      Author: andres
 */

#ifndef INTEGER_MATH_H_
#define INTEGER_MATH_H_

#include "types64bit.h"

S32_T float2fixed(R32_T val);

R32_T fixed2float(S32_T val);

S32_T fixp_abs(S32_T input);

S32_T fixp_mult(S32_T n1, S32_T n2);

#endif /* INTEGER_MATH_H_ */
