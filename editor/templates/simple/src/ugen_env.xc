/*
 * ugen_env.xc
 *
 *  Created on: Apr 11, 2014
 *      Author: andres
 */

#include <print.h>

#include "app_global.h"
#include "ugen_env.h"

#include "integer_math.h"

void ugen_env_init_data(ugen_env_data_t * unsafe data)
{
    ugen_env_controls_t tempcontrols;
    ugen_env_init_controls(tempcontrols);
    unsafe {
        ugen_env_ctl(tempcontrols, data);
    }
}

void ugen_env_init_controls(ugen_env_controls_t &controls)
{
    controls.att = 0.5f;
    controls.dec = 0.1f;
    controls.sus = 0.4f;
    controls.rel = 2.0f;
    controls.gate = 0;
}

void ugen_env_init(ugen_env_state_t &state, ugen_env_data_t * unsafe data)
{
    state.data = data;
    state.cur_value = 0;
    state.stage = UGEN_ENV_DONE;
    ugen_env_init_data(data);
}

S32_T ugen_env_tick(ugen_env_state_t &state)
{
    S32_T outval;
    unsafe {
        ugen_env_data_t * unsafe data = state.data;
        int gate = data->gate;
        if (gate == 0 && state.stage != UGEN_ENV_DONE
                && state.stage != UGEN_ENV_ATT && state.stage != UGEN_ENV_DEC) {
            state.stage = UGEN_ENV_REL;
        } else if (gate && (state.stage == UGEN_ENV_DONE || state.stage == UGEN_ENV_REL)) {
            state.stage = UGEN_ENV_ATT;
        }
        switch (state.stage) {
        case UGEN_ENV_ATT:

            state.cur_value += data->incrs[UGEN_ENV_ATT];
            if (state.cur_value >= (1 << FRACTIONALBITS)) {
                state.stage = UGEN_ENV_DEC;
            }

            break;
        case UGEN_ENV_DEC:
            state.cur_value += data->incrs[UGEN_ENV_DEC];
            if (state.cur_value <= data->sus_level) {
                state.stage = UGEN_ENV_SUS;
            }

            break;
        case UGEN_ENV_REL:
            state.cur_value += data->incrs[UGEN_ENV_REL];
            if (state.cur_value <= 0) {
                state.cur_value = 0;
                state.stage = UGEN_ENV_DONE;
            }

            break;
        case UGEN_ENV_SUS:
            break;
        case UGEN_ENV_DONE:
            break;
        }
        outval = state.cur_value;
    }
    return outval;
}

unsafe void ugen_env_ctl(ugen_env_controls_t &controls,
        ugen_env_data_t * unsafe data)
{
    data->gate = controls.gate;
    data->incrs[UGEN_ENV_ATT] = float2fixed(1.0 /(controls.att * SAMP_FREQ));
    data->incrs[UGEN_ENV_DEC] = float2fixed(-(1.0 - controls.sus) / (controls.dec * SAMP_FREQ));
    data->incrs[UGEN_ENV_SUS] = 0;
    data->sus_level = float2fixed(controls.sus);
    data->incrs[UGEN_ENV_REL] = float2fixed(-controls.sus /(controls.rel * SAMP_FREQ));
}
