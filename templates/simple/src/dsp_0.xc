/*
 * generator.xc
 *
 *  Created on: Mar 23, 2014
 *      Author: andres
 */


#include <print.h>

#include "app_global.h"

#include "dsp_0.h"
#include "integer_math.h"

//[[DSP Ugen Structs]]
ugen_sine_state_t sine_0_00;
ugen_env_state_t env_0_00;
ugen_noise_state_t noise_0_00;
ugen_comp_state_t comp_0_00;
//[[/DSP Ugen Structs]]


void dsp_0_init_ugens(swap_buffers& buffer)
{
    unsafe {
//[[DSP Init Ugens]]
        ugen_sine_init(sine_0_00, (ugen_sine_data_t *) &(buffer.sine_0_00_data[0]));
        ugen_env_init(env_0_00, (ugen_env_data_t *) &(buffer.env_0_00_data[0]));
        ugen_noise_init(noise_0_00, (ugen_noise_data_t *) &(buffer.noise_0_00_data[0]));
        ugen_comp_init(comp_0_00, (ugen_comp_data_t *) &(buffer.comp_0_00_data[0]));
//[[/DSP Init Ugens]]
    }
}

inline void dsp_0_process_ugens(S32_T &inp_samp, S32_T &out_samp)
{
//[[DSP Process Ugens]]
//    out_samp = ugen_sine_tick(sine_0_00);
//    out_samp = ugen_noise_tick(noise_0_00);
    out_samp = fixp_mult(ugen_noise_tick(noise_0_00), ugen_env_tick(env_0_00));

    out_samp = ugen_comp_tick(comp_0_00, out_samp);
//[[/DSP Process Ugens]]
}

void dsp_0(streaming chanend c_dsp, interface dsp_param_if client dsp_params, swap_buffers& buffer)
{
    S32_T inp_samp = 0;
    S32_T out_samp = 0;

    dsp_0_init_ugens(buffer);

//    printstrln("After initUgens dsp0");

    while(1)
    {
        c_dsp :> inp_samp;
        c_dsp <: out_samp;

        dsp_0_process_ugens(inp_samp, out_samp);

//[[DSP Pointer Swap]]
        select {
            case dsp_params.data_ready():
                ugen_id_t ugen_id = dsp_params.get_data_id();
                unsafe {
                    switch(ugen_id) {
                    case SINE_0_00_CONTROLS:
                        sine_0_00.data = dsp_params.get_data(sine_0_00.data);
                        break;
                    case ENV_0_00_CONTROLS:
                        env_0_00.data = dsp_params.get_data(env_0_00.data);
                        break;
                    case NOISE_0_00_CONTROLS:
                        noise_0_00.data = dsp_params.get_data(noise_0_00.data);
                        break;
                    case COMP_0_00_CONTROLS:
                        comp_0_00.data = dsp_params.get_data(comp_0_00.data);
                        break;
                    }
                }
                break;
            default:
                break;
        }
//[[/DSP Pointer Swap]]

    }
}
