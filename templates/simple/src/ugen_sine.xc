/*
 * ugen_sine.xc
 *
 *  Created on: Apr 17, 2014
 *      Author: andres
 */

#include "app_global.h"
#include "ugen_sine.h"

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

#ifndef M_PI
#    define M_PI 3.14159265358979323846
#endif

//#define UGEN_SINE_INTERP  // TODO Maybe a better idea to set by instace of ugens rather than by ugen

void ugen_sine_init_data(ugen_sine_data_t * unsafe data)
{
    ugen_sine_controls_t tempcontrols;
    ugen_sine_init_controls(tempcontrols);
    unsafe {
        data->sicvt = (R32_T) UGEN_SINE_CST_MAXLEN / SAMP_FREQ;
    }
    unsafe {
        ugen_sine_ctl(tempcontrols, data);
    }
}

void ugen_sine_init_controls(ugen_sine_controls_t &controls)
{
    controls.freq = UGEN_SINE_DEFAULT_FREQ;
}

void ugen_sine_init(ugen_sine_state_t &state,  ugen_sine_data_t * unsafe data)
{
    state.data = data;
    ugen_sine_init_data(state.data);
    long ltest,lobits,lomod,length = UGEN_SINE_TAB_LEN;
    for(ltest=length,lobits=0; (ltest & UGEN_SINE_CST_MAXLEN) == 0; lobits++, ltest<<=1);

    state.maxlen = UGEN_SINE_CST_MAXLEN;
    /* bitmask for ftable length */
    state.lenmask = UGEN_SINE_TAB_LEN-1;
    state.lobits = lobits;
    /* set interpolation precision per sample */
    lomod = UGEN_SINE_CST_MAXLEN / UGEN_SINE_TAB_LEN;
    state.lomask = lomod - 1;
    state.lodiv = 1.0/(double)lomod;
    state.phase = 0;

    for (int i = 0; i < UGEN_SINE_TAB_LEN; i++) {
        state.table[i] = (S32_T) (sin(2.0 * M_PI * i / (double) UGEN_SINE_TAB_LEN) * (1 << FRACTIONALBITS));
    }
}

#ifdef UGEN_SINE_INTERP

// FIXME currently crashing...
int ugen_sine_tick(ugen_sine_state_t &state)
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
    state.phase = phs & UGEN_SINE_PHMASK ;
    return outval;
}

#else

S32_T ugen_sine_tick(ugen_sine_state_t &state)
{
    S32_T *ftab = state.table;
    S32_T outval;
    S32_T phs, lobits;
    /* integer phase and increment between 0 and CST_MAXLEN */
    phs = state.phase;
    lobits = state.lobits;
    ftab += (phs >> lobits);
    outval = *ftab;
    unsafe {
        phs += state.data->inc;
    }
    state.phase = phs & UGEN_SINE_PHMASK;
    return outval;
}

#endif

unsafe void ugen_sine_ctl(ugen_sine_controls_t &controls, ugen_sine_data_t * unsafe data)
{
    data->inc = (S32_T)(controls.freq * data->sicvt);
}
