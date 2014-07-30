/*
 * ugen_modsine.xc
 *
 *  Created on: Jul 21, 2014
 *      Author: andres
 */

#include "app_global.h"
#include "ugen_modsine.h"

// Adapted the audio programming book DVD chapter 3
/* Copyright (c) 2009 Richard Dobson

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/


#include "math.h"
#include <print.h>
#include "integer_math.h"

#ifndef M_PI
#    define M_PI 3.14159265358979323846
#endif

#define UGEN_MODSINE_CST_MAXLEN 0x10000000
#define UGEN_MODSINE_PHMASK     0x0FFFFFFF

//#define UGEN_MODSINE_INTERP  // TODO Maybe a better idea to set by instace of ugens rather than by ugen

void ugen_modsine_init_data(ugen_modsine_data_t * unsafe data)
{
    ugen_modsine_controls_t tempcontrols;
    ugen_modsine_init_controls(tempcontrols);
    unsafe {
        data->sicvt = (R32_T) UGEN_MODSINE_CST_MAXLEN / SAMP_FREQ;
    }
    unsafe {
        ugen_modsine_ctl(tempcontrols, data);
    }
}

void ugen_modsine_init_controls(ugen_modsine_controls_t &controls)
{
    controls.freq = UGEN_MODSINE_DEFAULT_FREQ;
}

void ugen_modsine_init(ugen_modsine_state_t &state,  ugen_modsine_data_t * unsafe data)
{
    state.data = data;
    ugen_modsine_init_data(state.data);
    long ltest,lobits,lomod,length = UGEN_MODSINE_TAB_LEN;
    for(ltest=length,lobits=0; (ltest & UGEN_MODSINE_CST_MAXLEN) == 0; lobits++, ltest<<=1);

    state.maxlen = UGEN_MODSINE_CST_MAXLEN;
    state.lobits = lobits;
    /* set interpolation precision per sample */
    lomod = UGEN_MODSINE_CST_MAXLEN / UGEN_MODSINE_TAB_LEN;
    state.lomask = lomod - 1;
    state.lodiv = 1.0/(double)lomod;
    state.phase = 0;

    for (int i = 0; i < UGEN_MODSINE_TAB_LEN; i++) {
        state.table[i] = (S32_T) (sin(0.5 * M_PI * i / (double) UGEN_MODSINE_TAB_LEN) * (1 << FRACTIONALBITS));
    }
}

#ifdef UGEN_MODSINE_INTERP

// FIXME currently crashing...
int ugen_modsine_tick(ugen_modsine_state_t &state)
{
    double frac, lodiv;
    S32_T *ftab = state.table;
    double val_1;
    S32_T outval;
    S32_T phs, lomask, lobits;
    /* integer phase and increment between 0 and CST_MAXLEN */
    phs = state.phase;
    lomask = state.lomask;
    lodiv = state.lodiv;
    lobits = state.lobits;
    /* do interpolation to lomod precision */
    frac = (phs & lomask) * lodiv;
    ftab += (phs >> lobits);
    val_1 = *ftab++;
    outval = val_1 + (*ftab - val_1) * frac;
    unsafe {
        phs += state.data->inc;
    }
    state.phase = phs & UGEN_MODSINE_PHMASK ;
    return outval;
}

#else

S32_T ugen_modsine_tick(ugen_modsine_state_t &state, S32_T phsmod, S32_T depth)
{
    S32_T *ftab = state.table;
    S32_T outval;
    S32_T phs, stphs, lobits;
    // Must shift 4 bits to go from fixed point to the phase scale
    stphs = state.phase + (fixp_mult(phsmod, depth) << 4);
    phs = (stphs << 2) & (UGEN_MODSINE_PHMASK);
    lobits = state.lobits;
    if (stphs & (UGEN_MODSINE_CST_MAXLEN >> 2)) { // mirrored quarter
        ftab += (~(phs >> lobits)) & (UGEN_MODSINE_PHMASK >> lobits);
    } else {
        ftab += (phs >> lobits);
    }
    if (stphs & (UGEN_MODSINE_CST_MAXLEN >> 1)) { // negative part (second half)
        outval = - *ftab;
    } else {
        outval = *ftab;
    }
    unsafe {
        state.phase += state.data->inc;
    }
    state.phase = state.phase & UGEN_MODSINE_PHMASK;
    return outval;
}

#endif

unsafe void ugen_modsine_ctl(ugen_modsine_controls_t &controls, ugen_modsine_data_t * unsafe data)
{
    data->inc = (S32_T)(controls.freq * data->sicvt);
}
