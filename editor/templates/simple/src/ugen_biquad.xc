// First index is the dbLevel, in steps of 1.000000 db, first entry is -2.000000 db
// Second index is the filter number - this filter has 5 banks
// Each structure instantiation contains the five coefficients for each biquad:
// -a1/a0, -a2/a0, b0/a0, b1/a0, b2/a0; all numbers are stored in 7.24 fixed point

// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#include <xs1.h>
#include <print.h>

#include <math.h>

#include "app_global.h"
#include "ugen_biquad.h"

// Based on http://www.musicdsp.org/files/Audio-EQ-Cookbook.txt

#ifndef M_PI
#    define M_PI 3.14159265358979323846
#endif

void ugen_biquad_init_data(ugen_biquad_data_t * unsafe data)
{
    ugen_biquad_controls_t tempcontrols;
    ugen_biquad_init_controls(tempcontrols);
    unsafe {
        ugen_biquad_ctl(tempcontrols, data);
    }
}

void ugen_biquad_init_controls(ugen_biquad_controls_t &controls)
{
    controls.cf = 1000.0;
    controls.Q = 1.0;
    controls.dbGain = 0;
    controls.type = BYPASS;
}

void ugen_biquad_init(ugen_biquad_state_t &state, ugen_biquad_data_t * unsafe biquad_data)
{
    state.data = biquad_data;
    for(int i = 0; i <= BANKS; i++) {
        state.b[i].xn1 = 0;
        state.b[i].xn2 = 0;
    }
//    for(int i = 0; i < BANKS; i++) {
//        state.b[i].db = zeroDb;
//        state.desiredDb[i] = zeroDb;
//    }
    unsafe {
//        state.coeffs = biquad_data->coeffs;
//         Output is the same as the input
        ugen_biquad_init_data(biquad_data);
    }

//    printstrln("biwaud INIT!");
    //    state.adjustCounter = BANKS;
    //    state.adjustDelay = 0;


}

S32_T ugen_biquad_tick(ugen_biquad_state_t &state, S32_T xn) {

    unsigned int ynl;
    int ynh;
//    coeff_T coeffs;
//    unsafe {
//    coeffs = (coeff_T) &state.coeffs;
//    }

    for(int j=0; j<BANKS; j++) {
        ynl = (1<<(FRACTIONALBITS-1));        // 0.5, for rounding, could be triangular noise
        ynh = 0;
        // biquad formula:  y[n] = (b0/a0)*x[n] + (b1/a0)*x[n-1] + (b2/a0)*x[n-2]
        //                                      - (a1/a0)*y[n-1] - (a2/a0)*y[n-2]
        // where y[n-1] in stage j is x[n-1] from the previous iteration of stage j+1
        // source: biquad EQ cookbook by Robert Bristow-Johnson
        //
        unsafe{

            {ynh, ynl} = macs( state.data->coeffs[0], xn, ynh, ynl);
            {ynh, ynl} = macs( state.data->coeffs[1], state.b[j].xn1, ynh, ynl);
            {ynh, ynl} = macs( state.data->coeffs[2], state.b[j].xn2, ynh, ynl);
            {ynh, ynl} = macs( state.data->coeffs[3], state.b[j+1].xn1, ynh, ynl);
            {ynh, ynl} = macs( state.data->coeffs[4], state.b[j+1].xn2, ynh, ynl);
        }

//        {ynh, ynl} = macs( coeffs->b0, xn, ynh, ynl);
//        {ynh, ynl} = macs( coeffs->b1, state.b[j].xn1, ynh, ynl);
//        {ynh, ynl} = macs( coeffs->b2, state.b[j].xn2, ynh, ynl);
//        {ynh, ynl} = macs( coeffs->a1, state.b[j+1].xn1, ynh, ynl);
//        {ynh, ynl} = macs( coeffs->a2, state.b[j+1].xn2, ynh, ynl);
        if (sext(ynh,FRACTIONALBITS) == ynh) {
            ynh = (ynh << (32-FRACTIONALBITS)) | (ynl >> FRACTIONALBITS);
        } else if (ynh < 0) {
            ynh = 0x80000000;
        } else {
            ynh = 0x7fffffff;
        }
        state.b[j].xn2 = state.b[j].xn1;
        state.b[j].xn1 = xn;

        xn = ynh;
    }
    state.b[BANKS].xn2 = state.b[BANKS].xn1;
    state.b[BANKS].xn1 = ynh;
//    if (state.adjustDelay > 0) {
//        state.adjustDelay--;
//    } else {
//        state.adjustCounter--;
//        if (state.b[state.adjustCounter].db > state.desiredDb[state.adjustCounter]) {
//            state.b[state.adjustCounter].db--;
//        }
//        if (state.b[state.adjustCounter].db < state.desiredDb[state.adjustCounter]) {
//            state.b[state.adjustCounter].db++;
//        }
//        if (state.adjustCounter == 0) {
//            state.adjustCounter = BANKS;
//        }
//        state.adjustDelay = 40;
//    }
    return xn;
}

S32_T R(R32_T x) {
    if (x >= (1<<(31-FRACTIONALBITS)) || x < -(1<<(31-FRACTIONALBITS)))
    {
        printstrln("Overflow!");
    }
    return floor((1<<FRACTIONALBITS) * x + 0.5);
}

void ugen_biquad_ctl(ugen_biquad_controls_t &controls, ugen_biquad_data_t * unsafe buffer)
{
    R32_T b0, b1, b2, a0, a1, a2;
    R32_T A, sqrtA;
    R32_T dbGain = controls.dbGain;

    R32_T w0 = 2 * M_PI * controls.cf / (R32_T) SAMP_FREQ;
    R32_T cosw0 = cos(w0);
    R32_T alpha = sin(w0)/(2*controls.Q);
    R32_T twoSqrtAAlpha;
    switch (controls.type) {
    case LOWPASS:
        b0 =  (1 - cosw0)/2;
        b1 =   1 - cosw0;
        b2 =  (1 - cosw0)/2;
        a0 =   1 + alpha;
        a1 =  -2*cosw0;
        a2 =   1 - alpha;
        break;
    case HIGHPASS:
        b0 =  (1 + cosw0)/2;
        b1 =   -(1 + cosw0);
        b2 =  (1 + cosw0)/2;
        a0 =   1 + alpha;
        a1 =  -2*cosw0;
        a2 =   1 - alpha;
        break;
    case BANDPASS:
        b0 =   controls.Q*alpha;
        b1 =   0.0;
        b2 =  -controls.Q*alpha;
        a0 =   1 + alpha;
        a1 =  -2*cosw0;
        a2 =   1 - alpha;
        break;
    case BANDPASS2:
        b0 =   alpha;
        b1 =   0.0;
        b2 =   - alpha;
        a0 =   1 + alpha;
        a1 =  -2*cosw0;
        a2 =   1 - alpha;
        break;
    case NOTCH:
        b0 =   1.0;
        b1 =  -2*cosw0;
        b2 =   1.0;
        a0 =   1 + alpha;
        a1 =  -2*cosw0;
        a2 =   1 - alpha;
        break;
    case ALLPASS:
        b0 =   1 - alpha;
        b1 =  -2*cosw0;
        b2 =   1 + alpha;
        a0 =   1 + alpha;
        a1 =  -2*cosw0;
        a2 =   1 - alpha;
        break;
    case PEAKING:
        b0 =   1 + alpha*A;
        b1 =  -2*cosw0;
        b2 =   1 - alpha*A;
        a0 =   1 + alpha/A;
        a1 =  -2*cosw0;
        a2 =   1 - alpha/A;
        break;
    case LOWSHELF:
        A = pow(10.0, dbGain / 40.0); // TODO cookbook is unclear about this is it 20 or 40?
        sqrtA = sqrt(A);
        alpha = sin(w0)/2*sqrt(2);
        twoSqrtAAlpha = 2*sqrtA*alpha;
        b0 =   A*((A+1)-(A-1)*cosw0 + twoSqrtAAlpha);
        b1 = 2*A*((A-1)-(A+1)*cosw0                );
        b2 =   A*((A+1)-(A-1)*cosw0 - twoSqrtAAlpha);
        a0 =     ((A+1)+(A-1)*cosw0 + twoSqrtAAlpha);
        a1 =-2*  ((A-1)+(A+1)*cosw0                );
        a2 =     ((A+1)+(A-1)*cosw0 - twoSqrtAAlpha);
        break;
    case HIGHSHELF:
        A = pow(10.0, dbGain / 40.0);
        sqrtA = sqrt(A);
        alpha = sin(w0)/2*sqrt(2);
        twoSqrtAAlpha = 2*sqrtA*alpha;
        b0 =   A*((A+1)-(A-1)*cosw0 + twoSqrtAAlpha);
        b1 = -2*A*((A-1)-(A+1)*cosw0                );
        b2 =   A*((A+1)-(A-1)*cosw0 - twoSqrtAAlpha);
        a0 =     ((A+1)+(A-1)*cosw0 + twoSqrtAAlpha);
        a1 = 2*  ((A-1)+(A+1)*cosw0                );
        a2 =     ((A+1)+(A-1)*cosw0 - twoSqrtAAlpha);
        break;
    case BYPASS:
        a0 = b0 = 1 << FRACTIONALBITS;
        b1 = b2 = a1 = a2 = 0;
        break;
    }

    unsafe {
        buffer->coeffs[0] = R(b0/a0);
        buffer->coeffs[1] = R(b1/a0);
        buffer->coeffs[2] = R(b2/a0);
        buffer->coeffs[3] = -R(a1/a0);
        buffer->coeffs[4] = -R(a2/a0);
    }
}
