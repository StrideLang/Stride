/*
 * ugen_blosc.xc
 *
 *  Created on: May 2, 2014
 *      Author: andres
 */

#include "math.h"
#include <print.h>

#include "app_global.h"
#include "ugen_blosc.h"
#include "integer_math.h"


#ifndef M_PI
#    define M_PI 3.14159265358979323846
#endif

//#define UGEN_BLOSC_INTERP  // TODO Maybe a better idea to set by instace of ugens rather than by ugen

void ugen_blosc_init_data(ugen_blosc_data_t * unsafe data)
{
    ugen_blosc_controls_t tempcontrols;
    ugen_blosc_init_controls(tempcontrols);
    unsafe {
        data->sicvt = (R32_T) UGEN_BLOSC_CST_MAXLEN / SAMP_FREQ;
    }
    unsafe {
        ugen_blosc_ctl(tempcontrols, data);
    }
}

void ugen_blosc_init_controls(ugen_blosc_controls_t &controls)
{
    controls.freq = UGEN_BLOSC_DEFAULT_FREQ;
}

void ugen_blosc_init(ugen_blosc_state_t &state,  ugen_blosc_data_t * unsafe data)
{
    state.data = data;
    ugen_blosc_init_data(state.data);
    S32_T ltest,lobits,lomod,length = UGEN_BLOSC_TAB_LEN;
    for(ltest=length,lobits=0; (ltest & UGEN_BLOSC_CST_MAXLEN) == 0; lobits++, ltest<<=1);

    state.maxlen = UGEN_BLOSC_CST_MAXLEN;
    /* bitmask for ftable length */
    state.lenmask = UGEN_BLOSC_TAB_LEN-1;
    state.lobits = lobits;
    /* set interpolation precision per sample */
    lomod = UGEN_BLOSC_CST_MAXLEN / UGEN_BLOSC_TAB_LEN;
    state.lomask = lomod - 1;
    state.lodiv = 1.0/(double)lomod;
    state.phase = 0;

    for (int i = 0; i < UGEN_BLOSC_TAB_LEN; i++) {
        state.table[i] = (S32_T) (sin(2.0 * M_PI * i / (double) UGEN_BLOSC_TAB_LEN) * (1 << FRACTIONALBITS));
    }
}

#ifdef UGEN_BLOSC_INTERP

// FIXME currently crashing...
int ugen_blosc_tick(ugen_blosc_state_t &state)
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
    state.phase = phs & UGEN_BLOSC_PHMASK ;
    return outval;
}

#else

S32_T ugen_blosc_tick(ugen_blosc_state_t &state, S32_T input)
{
    S32_T *ftab = state.table;
    S32_T *ftab2 = state.table;
    S32_T outval;
    S32_T phs, lobits, lomask,lodiv;
    S32_T kden;
    S32_T kn2;

    /* integer phase and increment between 0 and CST_MAXLEN */
    phs = state.phase;
    lobits = state.lobits;
    lomask = state.lomask;
    lodiv = state.lodiv;
//    frac = (phs & lomask) * lodiv;
    ftab += (phs >> lobits);
    kden = *ftab++;
    unsafe {
        kn2 = state.data->kn2;
    }
    // FIXME finish interpolation
//    if (ftab == state.table + UGEN_BLOSC_TAB_LEN) {
//        ftab = state.table ; // FIXME implement guard point to avoid this check
//    }
//    kden += float2fixed((*ftab - val_1) * frac);


    if (kden != 0) {
        S32_T phs2 = phs*(kn2 + 1);
        phs2 = phs2 & UGEN_BLOSC_PHMASK;
//        knum tablei kph*(2*kn+1),itb,1,0,1
        ftab2 += (phs2  >> lobits);
        outval = ((1 << FRACTIONALBITS)/kn2)*(*ftab2/kden - 1);
    } else  {
        outval = 1 << FRACTIONALBITS;
    }

    unsafe {
        phs += state.data->inc >> 1; // Divide by two in original algorithm
    }
    state.phase = phs & UGEN_BLOSC_PHMASK;
    return outval;
}

#endif

void ugen_blosc_ctl(ugen_blosc_controls_t &controls, ugen_blosc_data_t * unsafe data)
{
    unsafe {
        data->inc = (S32_T)(controls.freq * data->sicvt);
        data->kn2 = float2fixed(SAMP_FREQ /controls.freq);
        data->inc = (S32_T)(controls.freq * data->sicvt);
    }
}
