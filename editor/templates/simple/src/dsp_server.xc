/*
 * dsp_server.xc
 *
 *  Created on: Mar 26, 2014
 *      Author: andres
 */

#include <xs1.h>
#include <print.h>
#include <string.h>

#include "app_global.h"
#include "dsp_server.h"

void dsp_server(interface dsp_param_if server dsp_params[],
        interface server_xchange_if client server_xchange,
        swap_buffers buffers[])
{
    unsigned data_ready_bits[NUM_APP_CHANS];
    for (int i = 0; i < NUM_APP_CHANS; i++) data_ready_bits[i] = 0;

//[[DSP Server Ugen Init]]
    ugen_sine_data_t * unsafe sine_0_00_data;
    ugen_env_data_t * unsafe env_0_00_data;
    ugen_noise_data_t * unsafe noise_0_00_data;
    ugen_comp_data_t * unsafe comp_0_00_data;

    ugen_biquad_data_t * unsafe biquad_1_00_data;
    ugen_blosc_data_t * unsafe blosc_1_00_data;

    ugen_biquad_data_t * unsafe biquad_2_00_data;

    ugen_biquad_data_t * unsafe biquad_3_00_data;
    unsafe {
        sine_0_00_data = (ugen_sine_data_t *) buffers[0].sine_0_00_data;
        env_0_00_data = (ugen_env_data_t *) buffers[0].env_0_00_data;
        noise_0_00_data = (ugen_noise_data_t *) buffers[0].noise_0_00_data;
        comp_0_00_data = (ugen_comp_data_t *) buffers[0].comp_0_00_data;

        biquad_1_00_data = (ugen_biquad_data_t *) buffers[0].biquad_1_00_data;
        blosc_1_00_data = (ugen_blosc_data_t *) buffers[0].blosc_1_00_data;

        biquad_2_00_data = (ugen_biquad_data_t *) buffers[0].biquad_2_00_data;

        biquad_3_00_data = (ugen_biquad_data_t *) buffers[0].biquad_3_00_data;

        ugen_sine_init_data(sine_0_00_data);
        ugen_env_init_data(env_0_00_data);
        ugen_noise_init_data(noise_0_00_data);
        ugen_comp_init_data(comp_0_00_data);

        ugen_biquad_init_data(biquad_1_00_data);

        ugen_biquad_init_data(biquad_2_00_data);

        ugen_biquad_init_data(biquad_3_00_data);

    }
//[[/DSP Server Ugen Init]]

//[[DSP Server Process]]
    while (1) {
        select {
        case dsp_params[0].get_data_id() -> ugen_id_t ugen_id:
//                printstrln("DSP Server: Moving pointers 1.");
            if (data_ready_bits[0] & (1 << (SINE_0_00_CONTROLS & 0x3F))) {
                ugen_id = SINE_0_00_CONTROLS;
            } else if (data_ready_bits[0] & (1 << (ENV_0_00_CONTROLS & 0x3F))) {
                ugen_id = ENV_0_00_CONTROLS;
            } else if (data_ready_bits[0] & (1 << (NOISE_0_00_CONTROLS & 0x3F))) {
                ugen_id = NOISE_0_00_CONTROLS;
            } else if (data_ready_bits[0] & (1 << (BIQUAD_1_00_CONTROLS & 0x3F))) {
                ugen_id = BIQUAD_1_00_CONTROLS;
            }
            break;
        case dsp_params[1].get_data_id() -> ugen_id_t ugen_id:
//                printstrln("DSP Server: Moving pointers 1.");
            if (data_ready_bits[0] & (1 << (BIQUAD_1_00_CONTROLS & 0x3F))) {
                ugen_id = BIQUAD_1_00_CONTROLS;
            } else if (data_ready_bits[0] & (1 << (BLOSC_1_00_CONTROLS & 0x3F))) {
                ugen_id = BLOSC_1_00_CONTROLS;
            }
            break;
        case dsp_params[2].get_data_id() -> ugen_id_t ugen_id:
//                printstrln("DSP Server: Moving pointers 1.");
            if (data_ready_bits[0] & (1 << (BIQUAD_2_00_CONTROLS & 0x3F))) {
                ugen_id = BIQUAD_2_00_CONTROLS;
            }
            break;
        case dsp_params[3].get_data_id() -> ugen_id_t ugen_id:
        //                printstrln("DSP Server: Moving pointers 1.");
            if (data_ready_bits[0] & (1 << (BIQUAD_3_00_CONTROLS & 0x3F))) {
                ugen_id = BIQUAD_3_00_CONTROLS;
            }
            break;
        case dsp_params[0].get_data(void * unsafe dsp_pointer) -> void * unsafe new_dsp_ptr:
            unsafe {
//                printstrln("DSP Server: Moving pointers 1.");
                if (data_ready_bits[0] & (1 << (SINE_0_00_CONTROLS & 0x3F))) {
                    new_dsp_ptr = sine_0_00_data;
                    sine_0_00_data = dsp_pointer;
                } else if (data_ready_bits[0] & (1 << (ENV_0_00_CONTROLS & 0x3F))) {
                    new_dsp_ptr = env_0_00_data;
                    env_0_00_data = dsp_pointer;
                } else if (data_ready_bits[0] & (1 << (NOISE_0_00_CONTROLS & 0x3F))) {
                    new_dsp_ptr = noise_0_00_data;
                    noise_0_00_data = dsp_pointer;
                } else if (data_ready_bits[0] & (1 << (BIQUAD_1_00_CONTROLS & 0x3F))) {
                    new_dsp_ptr = biquad_1_00_data;
                    biquad_1_00_data = dsp_pointer;
                }

            }
            break;
        case dsp_params[1].get_data(void * unsafe dsp_pointer) -> void * unsafe new_dsp_ptr:
            unsafe {
                if (data_ready_bits[1] & (1 << (BIQUAD_1_00_CONTROLS & 0x3F))) {
                    new_dsp_ptr = biquad_1_00_data;
                    biquad_1_00_data = dsp_pointer;
                }
                if (data_ready_bits[1] & (1 << (BLOSC_1_00_CONTROLS & 0x3F))) {
                    new_dsp_ptr = blosc_1_00_data;
                    blosc_1_00_data = dsp_pointer;
                }
            }
            break;
        case dsp_params[2].get_data(void * unsafe dsp_pointer) -> void * unsafe new_dsp_ptr:
            unsafe {
                if (data_ready_bits[2] & (1 << (BIQUAD_2_00_CONTROLS & 0x3F))) {
                    new_dsp_ptr = biquad_2_00_data;
                    biquad_2_00_data = dsp_pointer;
                }
            }
            break;
        case dsp_params[3].get_data(void * unsafe dsp_pointer) -> void * unsafe new_dsp_ptr:
            unsafe {
                if (data_ready_bits[3] & (1 << (BIQUAD_3_00_CONTROLS & 0x3F))) {
                    new_dsp_ptr = biquad_3_00_data;
                    biquad_3_00_data = dsp_pointer;
                }
            }
            break;
        case server_xchange.data_ready():
            xchange_t data = server_xchange.get_data();
            switch (data.index) {
            case SINE_0_00_CONTROLS:
                unsafe {
                    memcpy(sine_0_00_data, data.data, sizeof(ugen_sine_data_t));
                }
                data_ready_bits[0] |= 1 << (SINE_0_00_CONTROLS & 0x3F);
                dsp_params[0].data_ready();
                break;
            case ENV_0_00_CONTROLS:
                unsafe {
                    memcpy(env_0_00_data, data.data, sizeof(ugen_env_data_t));
                }
                data_ready_bits[0] |= 1 << (ENV_0_00_CONTROLS & 0x3F);
                dsp_params[0].data_ready();
                break;
            case NOISE_0_00_CONTROLS:
                unsafe {
                    memcpy(noise_0_00_data, data.data, sizeof(ugen_noise_data_t));
                }
                data_ready_bits[0] |= 1 << (NOISE_0_00_CONTROLS & 0x3F);
                dsp_params[0].data_ready();
                break;
            case BIQUAD_1_00_CONTROLS:
                unsafe {
                    memcpy(biquad_1_00_data, data.data,  sizeof(ugen_biquad_data_t));
                }
                data_ready_bits[1] |= 1 << (BIQUAD_1_00_CONTROLS & 0x3F);
                dsp_params[1].data_ready();
                break;
            case BLOSC_1_00_CONTROLS:
                unsafe {
                    memcpy(blosc_1_00_data, data.data,  sizeof(ugen_blosc_data_t));
                }
                data_ready_bits[1] |= 1 << (BLOSC_1_00_CONTROLS & 0x3F);
                dsp_params[1].data_ready();
                break;
            case BIQUAD_2_00_CONTROLS:
                unsafe {
                    memcpy(biquad_2_00_data, data.data,  sizeof(ugen_biquad_data_t));
                }
                data_ready_bits[2] |= 1 << (BIQUAD_2_00_CONTROLS& 0x3F);
                dsp_params[2].data_ready();
                break;
            case BIQUAD_3_00_CONTROLS:
                unsafe {
                    memcpy(biquad_3_00_data, data.data,  sizeof(ugen_biquad_data_t));
                }
                data_ready_bits[3] |= 1 << (BIQUAD_3_00_CONTROLS & 0x3F);
                dsp_params[3].data_ready();
                break;
            }
            break;
        }
    }
//[[/DSP Server Process]]
}
