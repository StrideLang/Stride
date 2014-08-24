/*
 * control_server.xc
 *
 *  Created on: Mar 27, 2014
 *      Author: andres
 */

#include <xs1.h>
#include <platform.h>
#include <print.h>
#include <string.h> // For memcpy

#include "app_global.h"
#include "control_server.h"

on tile[CTL_TILE]: port buttonleds = XS1_PORT_32A;
on tile[CTL_TILE] : port ctl_led1 = XS1_PORT_1D;
on tile[CTL_TILE] : port ctl_led2 = XS1_PORT_1A;

typedef enum button_val {
  BUTTON_UP,
  BUTTON_DOWN
} button_val_t;

#ifdef CONTROL_COMBINABLE
[[combinable]]
#endif
void control_server(interface computation_if server comps,
        interface server_xchange_if server server_xchange,
        swap_buffers buffers[],
        interface ctl_from_dsp_if client ctl_from_dsp)
{
    control_data_t server_ctr_data;
    int control_pending = -1;

    ctl_led1 <: 1;

//[[Control Server Ugen Init]]
    ugen_env_init_controls(server_ctr_data.env_0_00_controls);
    ugen_sine_init_controls(server_ctr_data.sine_0_00_controls);
    ugen_noise_init_controls(server_ctr_data.noise_0_00_controls);
    ugen_comp_init_controls(server_ctr_data.comp_0_00_controls);

    ugen_biquad_init_controls(server_ctr_data.biquad_1_00_controls);

    ugen_biquad_init_controls(server_ctr_data.biquad_2_00_controls);
    ugen_sine_init_controls(server_ctr_data.sine_2_00_controls);

    ugen_biquad_init_controls(server_ctr_data.biquad_3_00_controls);

    unsafe {
        ugen_env_init_data((ugen_env_data_t *) buffers[0].env_0_00_data);
        ugen_sine_init_data((ugen_sine_data_t *) buffers[0].sine_0_00_data);
        ugen_noise_init_data((ugen_noise_data_t *) buffers[0].noise_0_00_data);
        ugen_comp_init_data((ugen_comp_data_t *) buffers[0].comp_0_00_data);

        ugen_biquad_init_data((ugen_biquad_data_t *) buffers[0].biquad_1_00_data);

        ugen_biquad_init_data((ugen_biquad_data_t *) buffers[0].biquad_2_00_data);
        ugen_sine_init_data((ugen_sine_data_t *) buffers[0].sine_2_00_data);

        ugen_biquad_init_data((ugen_biquad_data_t *) buffers[0].biquad_3_00_data);
    }
//[[/Control Server Ugen Init]]

//[[Control Server Control Init]]
    R32_T Fi = 500; // Temporary accumulator for frequency values

    unsigned button_val = BUTTON_DOWN;
    timer tmr;
    int time;
    tmr :> time;

    int button_debounce_max_count = 50;
    int button_debounce_count = button_debounce_max_count;

    unsigned butdata;
//[[/Control Server Control Init]]

//    printstrln("control_server init.");
    while (1) {
        select {
            case comps.data_computed():
                    server_xchange.data_ready();
            break;
        case server_xchange.get_data() -> xchange_t xchange_data:
            xchange_t data;
            data.index = control_pending;
            unsigned int datasize;

//[[Control Server Control Process]]
            switch(control_pending) {
            case ENV_0_00_CONTROLS:
                datasize = sizeof(ugen_env_data_t);
                break;
            case SINE_0_00_CONTROLS:
                datasize = sizeof(ugen_sine_data_t);
                break;
            case NOISE_0_00_CONTROLS:
                datasize = sizeof(ugen_noise_data_t);
                break;
            case COMP_0_00_CONTROLS:
                datasize = sizeof(ugen_comp_data_t);
                break;
            case BIQUAD_1_00_CONTROLS:
                datasize = sizeof(ugen_biquad_data_t);
                break;
            case BIQUAD_2_00_CONTROLS:
                datasize = sizeof(ugen_biquad_data_t);
                break;
            case SINE_2_00_CONTROLS:
                datasize = sizeof(ugen_sine_data_t);
                break;
            case BIQUAD_3_00_CONTROLS:
                datasize = sizeof(ugen_biquad_data_t);
                break;
            }
            memcpy(data.data, server_ctr_data.data_buffer, datasize);
            xchange_data = data;

            control_pending = -1;
            break;

            case comps.get_compute_data() -> control_data_t * unsafe ctr_data:
                server_ctr_data.index = control_pending;
                unsafe {
                    switch(control_pending) {
                    case ENV_0_00_CONTROLS:
                        server_ctr_data.data_buffer =  buffers[0].env_0_00_data;
                        ctr_data = &server_ctr_data;
                        server_ctr_data.value = button_val == BUTTON_DOWN ? 1:0;
                        break;
                    case SINE_0_00_CONTROLS:
                        server_ctr_data.data_buffer =  buffers[0].sine_0_00_data;
                        ctr_data = &server_ctr_data;
                        break;
                    case NOISE_0_00_CONTROLS:
                        server_ctr_data.data_buffer =  buffers[0].noise_0_00_data;
                        ctr_data = &server_ctr_data;
                        server_ctr_data.value = 100.0;
                        break;
                    case COMP_0_00_CONTROLS:
                        server_ctr_data.data_buffer =  buffers[0].comp_0_00_data;
                        ctr_data = &server_ctr_data;
                        break;
                    case BIQUAD_1_00_CONTROLS:
                        server_ctr_data.data_buffer =  buffers[1].biquad_1_00_data;
                        ctr_data = &server_ctr_data;
                        break;
                    case BIQUAD_2_00_CONTROLS:
                        server_ctr_data.data_buffer =  buffers[2].biquad_2_00_data;
                        ctr_data = &server_ctr_data;
                        break;
                    case BIQUAD_3_00_CONTROLS:
                        server_ctr_data.data_buffer =  buffers[3].biquad_3_00_data;
                        ctr_data = &server_ctr_data;
                        break;
                    }
                }
            break;
//[[/Control Server Control Process]]
        case ctl_from_dsp.data_ready():
        ctl_from_dsp_t data = ctl_from_dsp.get_data();
//        printintln(data.data[0]);
        break;
//[[/Control Server Input Process]]
        case tmr when timerafter(time) :> void:
            if (control_pending != -1) break;
            buttonleds :> butdata;
            time += 10000;
            if (button_debounce_count >= button_debounce_max_count) {
                button_val_t new_val = (butdata & 1) ? BUTTON_UP : BUTTON_DOWN;
                if (new_val != button_val) {
                    ctl_led1 <: new_val;
                    button_val = new_val;
                    button_debounce_count = 0;
//                    printintln(button_val);
//                    control_pending = NOISE_0_00_CONTROLS;
                    control_pending = ENV_0_00_CONTROLS;
                    comps.data_ready();  // FIXME : This call should block as ther eis no way of knowing if more than a single message needs processing
                    if (new_val == 1) {
//                        comps.data_ready();
                        Fi += 500;
                        if (Fi > 18000) {
                         Fi = 100;
                        }
                    }
                }
            }
            else {
                button_debounce_count++;
            }
            break;
//[[/Control Server Input Process]]
        }
    }
}
