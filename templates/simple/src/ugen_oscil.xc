/*
 * ugen_oscil.xc
 *
 *  Created on: Apr 6, 2014
 *      Author: andres
 */

#include "app_global.h"
#include "ugen_oscil.h"

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


void ugen_oscil_init_data(ugen_oscil_data_t * unsafe data)
{
    ugen_oscil_controls_t tempcontrols;
    ugen_oscil_init_controls(tempcontrols);
    unsafe {
        ugen_oscil_ctl(tempcontrols, data);
    }
}
void ugen_oscil_init_controls(ugen_oscil_controls_t & controls)
{
    controls.freq = 440;
    //FIXME finish this ugen
}
void ugen_oscil_init(ugen_oscil_state_t &state,  ugen_oscil_data_t * unsafe data)
{
//    printstrln("Init oscil");
    state.data = data;
    ugen_oscil_init_data(state.data);
    long ltest,lobits,lomod,length = UGEN_OSCIL_TAB_LEN;
    for(ltest=length,lobits=0; (ltest & UGEN_OSCIL_CST_MAXLEN) == 0; lobits++, ltest<<=1);

    state.maxlen = UGEN_OSCIL_CST_MAXLEN;
//    state.ftable = gtable->table;
//    state.ftlen = TAB_LEN;
    /* bitmask for ftable length */
    state.lenmask = UGEN_OSCIL_TAB_LEN-1;
    state.lobits = lobits;
    /* set interpolation precision per sample */
    lomod = UGEN_OSCIL_CST_MAXLEN / UGEN_OSCIL_TAB_LEN;
    state.lomask = lomod - 1;
    state.lodiv = 1.0/(double)lomod;
    state.phase = 0;


//    printintln(state.inc);
    for (int i = 0; i < UGEN_OSCIL_TAB_LEN; i++) {
        // Must scale to DAC maximum (1 << 15)
        state.table[i] = (S32_T) (sin(2.0 * M_PI * i / (double) UGEN_OSCIL_TAB_LEN) * (1 << FRACTIONALBITS));
    }


}

#ifdef UGEN_OSCIL_INTERP

int ugen_oscil_tick_interp(ugen_oscil_state_t &state)
{
    S32_T * table = state.table;
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
    ftab = table + (phs >> lobits);
    val_1 =
    *ftab++;
    outval = val_1 + (*ftab - val_1) * frac;
    phs += inc;
    state.phase = phs & UGEN_OSCIL_PHMASK ;
    return outval;
}

#else

S32_T ugen_oscil_tick(ugen_oscil_state_t &state)
{
    S32_T *ftab = state.table;
    S32_T outval;
    S32_T phs, lomask, lobits;
    /* integer phase and increment between 0 and CST_MAXLEN */
    phs = state.phase;
    lomask = state.lomask;
    lobits = state.lobits;
    ftab += (phs >> lobits);
    outval = *ftab++;
    unsafe {
        phs += state.data->inc;
    }
    state.phase = phs & UGEN_OSCIL_PHMASK;
    return outval;
}

#endif

unsafe void ugen_oscil_ctl(ugen_oscil_controls_t &controls, ugen_oscil_data_t * unsafe data)
{
    switch(controls.shape) {
    case UGEN_OSCIL_SHAPE_SAW:
        break;
    case UGEN_OSCIL_SHAPE_SQUARE:
        break;
    case UGEN_OSCIL_SHAPE_PULSE:
        break;
    case UGEN_OSCIL_SHAPE_PULSEBI:
        break;
    }

    data->inc = (S32_T)(controls.freq * data->sicvt);
}
