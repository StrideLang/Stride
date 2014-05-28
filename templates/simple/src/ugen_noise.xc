/*
 * ugen_noise.xc
 *
 *  Created on: Apr 29, 2014
 *      Author: andres
 */

#include "app_global.h"
#include "ugen_noise.h"

//#include <print.h>

void ugen_noise_init_data(ugen_noise_data_t * unsafe data)
{
    ugen_noise_controls_t tempcontrols;
    ugen_noise_init_controls(tempcontrols);
    unsafe {
        ugen_noise_ctl(tempcontrols, data);
    }
}

void ugen_noise_init_controls(ugen_noise_controls_t &controls)
{
    controls.sampholdfreq = 0; // no samphold
}

void ugen_noise_init(ugen_noise_state_t &state, ugen_noise_data_t * unsafe data)
{
    state.data = data;
    state.counter = 0;
    state.prev_samp = 0;
    state.random_generator = random_create_generator_from_seed(0);
    ugen_noise_init_data(data);
}

S32_T ugen_noise_tick(ugen_noise_state_t &state)
{
    S32_T output;
    unsafe {
        if (state.counter >= state.data->numsamps) {
            unsigned rand = random_get_random_number(state.random_generator);
//            unsigned sign = (rand & (1 << 31)) >> 31;
            output = ((S32_T) rand)/(1<<7); // TODO find better way to scale this.
            state.counter = 0;
            state.prev_samp = output;
        } else {
            output = state.prev_samp;
        }
    }

    state.counter++;
    return output;
}

unsafe void ugen_noise_ctl(ugen_noise_controls_t &controls, ugen_noise_data_t * unsafe data)
{
    if (controls.sampholdfreq >= SAMP_FREQ || controls.sampholdfreq == 0) {
        data->numsamps = 1;
    } else {
        data->numsamps = SAMP_FREQ/controls.sampholdfreq;
    }
}
