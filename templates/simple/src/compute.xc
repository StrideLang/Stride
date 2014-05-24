/*
 * compute.xc
 *
 *  Created on: Mar 26, 2014
 *      Author: Pacifist
 */

#include <xs1.h>
#include <platform.h>
#include <print.h>
//#include <stdio.h>

#include <math.h>

#include "app_global.h"
#include "compute.h"
#include "types64bit.h"

#ifdef CONTROL_COMBINABLE
[[combinable]]
#endif
void compute(interface computation_if client comps,
            swap_buffers buffers[])
{
    while(1) {
        select {
            case comps.data_ready():
                control_data_t * unsafe ctr_data = comps.get_compute_data();

                unsafe {
                switch(ctr_data->index) {
//[[Compute Process]]
                case BIQUAD_1_00_CONTROLS:
                    ugen_biquad_data_t * unsafe data = (ugen_biquad_data_t *) ctr_data->data_buffer;
                    unsafe {
                        ctr_data->biquad_1_00_controls.cf = ctr_data->value;
//                            printstr("gate: ");
//                            printintln(ctr_data->env_controls.gate);
                    }

                    ugen_biquad_ctl(ctr_data->biquad_1_00_controls, data);

                    break;
                case SINE_0_00_CONTROLS:
                    ugen_sine_data_t * unsafe data = (ugen_sine_data_t *) ctr_data->data_buffer;
                    unsafe {
                        ctr_data->sine_0_00_controls.freq = ctr_data->value;
                    }
                    ugen_sine_ctl(ctr_data->sine_0_00_controls, data);

                    break;
                case ENV_0_00_CONTROLS:
                    ugen_env_data_t * unsafe data = (ugen_env_data_t *) ctr_data->data_buffer;
                    unsafe {
                        ctr_data->env_0_00_controls.gate = ctr_data->value;
                    }
                    ugen_env_ctl(ctr_data->env_0_00_controls, data);
                    break;
                case NOISE_0_00_CONTROLS:
                    ugen_noise_data_t * unsafe data = (ugen_noise_data_t *) ctr_data->data_buffer;
                    unsafe {
                        ctr_data->noise_0_00_controls.sampholdfreq = ctr_data->value;
                    }
                    ugen_noise_ctl(ctr_data->noise_0_00_controls, data);
                    break;
                }
//[[/Compute Process]]
                comps.data_computed();
                break;
                }

        }
    }
}
