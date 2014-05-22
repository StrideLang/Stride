/*
 * ugen_reverb1.xc
 *
 *  Created on: Apr 17, 2014
 *      Author: andres
 */

#include "ugen_reverb1.h"

void ugen_reverb1_init_data(ugen_reverb1_data_t * unsafe data)
{
    ugen_reverb1_controls_t tempcontrols;
//    ugen_reverb1_init_controls(tempcontrols);
    unsafe {
        ugen_reverb1_ctl(tempcontrols, data);
    }
}

void ugen_reverb1_init(ugen_reverb1_state_t &state, ugen_reverb1_data_t * unsafe data)
{
//    for (int i = 0; i < 4; i++) {
//        memset(state.comb[i].comb_line, 0, UGEN_REVERB1_COMB_LENGTH * sizeof(S32_T));
//        state.comb[i].readpos = 0;
//    }
//    // TODO calculate these values properly
//    state.comb[0].writedelta = 4000;
//    state.comb[1].writedelta = 3700;
//    state.comb[2].writedelta = 3500;
//    state.comb[3].writedelta = 3256;
//    for (int i = 0; i < 2; i++) {
//        memset(state.allpass[i].comb_line, 0, UGEN_REVERB1_COMB_LENGTH * sizeof(S32_T));
//        state.allpass[i].readpos = 0;
//    }
//    // TODO calculate these values properly
//    state.allpass[0].writedelta = 1000;
//    state.allpass[1].writedelta = 330;
//    state.allpass[2].writedelta = 110;
}

S32_T ugen_reverb1_tick(ugen_reverb1_state_t &state, S32_T sample)
{


}

unsafe void ugen_reverb1_ctl(ugen_reverb1_controls_t &controls, ugen_reverb1_data_t * unsafe data)
{

}
