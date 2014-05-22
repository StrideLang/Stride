/*
 * ugen_comp.xc
 *
 *  Created on: May 1, 2014
 *      Author: andres
 */

/*
 * ugen_comp.xc
 *
 *  Created on: Apr 11, 2014
 *      Author: andres
 */

#include <print.h>
#include <safestring.h>
#include <xscope.h>

#include "app_global.h"
#include "ugen_comp.h"

#include "integer_math.h"
#include <math.h>

void ugen_comp_init_data(ugen_comp_data_t * unsafe data) {
    ugen_comp_controls_t tempcontrols;
    ugen_comp_init_controls(tempcontrols);
    unsafe {
        ugen_comp_ctl(tempcontrols, data);
    }
}

void ugen_comp_init_controls(ugen_comp_controls_t &controls) {
    controls.threshold = -20.0;
    controls.att_time = 0.01;
    controls.rel_time = 0.05;
    controls.ratio = 2.0;

    controls.lookahead_time = 0;
}

void ugen_comp_init(ugen_comp_state_t &state, ugen_comp_data_t * unsafe data) {
    state.data = data;
    state.gainTarget = 1 << FRACTIONALBITS;
    state.curGain = 1 << FRACTIONALBITS;

    safememset((unsigned char*) state.la_delay, 0,
            sizeof(S32_T) * MAX_LOOKAHEAD_SAMPLES);

    state.la_delay_write = 0;

    safememset((unsigned char*) state.env_delay, 0,
            sizeof(S32_T) * MAX_LOOKAHEAD_SAMPLES);
    state.env_delay_write = 0;
    state.env_accumulator = 0;

    ugen_comp_init_data(data);
}

S32_T ugen_comp_tick(ugen_comp_state_t &state, S32_T input) {
    S32_T outval, dest;

    S32_T th, invratio, ac, rc, la_delay_delta;
    unsafe
    {
        th = state.data->threshold;
        invratio = state.data->invratio;
        ac = state.data->attack_constant;
        rc = state.data->release_constant;
        la_delay_delta = state.data->la_delay_delta;
    }
    state.env_accumulator += fixp_abs(input) >> ENV_WINDOW_SHIFT;

    state.env_accumulator -= state.env_delay[state.env_delay_write];
    state.env_delay[state.env_delay_write] = fixp_abs(input)
            >> ENV_WINDOW_SHIFT;
    if (++(state.env_delay_write) == MAX_ENV_WINDOW_SAMPLES) {
        state.env_delay_write = 0; // TODO this should be done using bit masking
    }
    if (state.env_accumulator > th) {
        // TODO would be nicer in dB scale instead of linear scale
        dest = float2fixed(fixed2float(th)/
                (fixed2float(th + fixp_mult(state.env_accumulator -th, invratio))));
    } else {
        dest = 1 << FRACTIONALBITS;
    }
    if (state.curGain > dest) { //attack
        state.curGain += fixp_mult((dest - state.curGain), ac);
    } else { //release
        state.curGain += fixp_mult((dest - state.curGain), rc);
    }
    S32_T la_read_index = state.la_delay_write - la_delay_delta;
    if (la_read_index < 0) {
        la_read_index += MAX_LOOKAHEAD_SAMPLES;
    }
//#ifdef USE_XSCOPE
//        xscope_int(2, dest);
//#endif

    outval = fixp_mult(state.la_delay[la_read_index], state.curGain);
    state.la_delay[state.la_delay_write] = input;
    if (++(state.la_delay_write) == MAX_LOOKAHEAD_SAMPLES) {
        state.la_delay_write = 0;
    }
//    // TODO implement knee?
//
    return outval;
}

unsafe void ugen_comp_ctl(ugen_comp_controls_t &controls,
        ugen_comp_data_t * unsafe data) {
    data->threshold = float2fixed(powf(10.0, (controls.threshold * 0.05))); // from dB to linear amp
    data->invratio = float2fixed(1.0/controls.ratio); // to calculate gain reduction factor
    data->attack_constant = float2fixed(1.0 / (controls.att_time * SAMP_FREQ));
    data->release_constant = float2fixed(1.0 / (controls.rel_time * SAMP_FREQ));
    data->la_delay_delta = controls.lookahead_time * SAMP_FREQ
    ;
    if (data->la_delay_delta >= MAX_LOOKAHEAD_SAMPLES) {
        data->la_delay_delta = MAX_LOOKAHEAD_SAMPLES - 1;
    }
}
