/*
 * control_server.h
 *
 *  Created on: Mar 27, 2014
 *      Author: andres
 */

#ifndef CONTROL_SERVER_H_
#define CONTROL_SERVER_H_

#include "types64bit.h"
#include "server_interface.h"

#include "ugen_includes.h"

// Make control server and control compute combinable functions
// to run them in the same logical core.
// Interestingly, this also frees a timer and 2 chanends on STARTKIT
// but uses slightly more memory.
#define CONTROL_COMBINABLE

typedef struct {
    ugen_id_t index;  // ugen identifier identifier
    R32_T value; // value to be transformed
    S32_T * unsafe data_buffer; // calculated values buffer
//[[Control Server Ugen Controls]]
    ugen_env_controls_t env_0_00_controls;
    ugen_sine_controls_t sine_0_00_controls;
    ugen_noise_controls_t noise_0_00_controls;
    ugen_comp_controls_t comp_0_00_controls;

    ugen_biquad_controls_t biquad_1_00_controls;

    ugen_biquad_controls_t biquad_2_00_controls;
    ugen_sine_controls_t sine_2_00_controls;

    ugen_biquad_controls_t biquad_3_00_controls;
//[[/Control Server Ugen Controls]]
} control_data_t;

interface computation_if {
    control_data_t * unsafe get_compute_data();
    [[clears_notification]] void data_computed();
    [[notification]] slave void data_ready();
};

#ifdef CONTROL_COMBINABLE
[[combinable]]
#endif
void control_server(interface computation_if server comps,
        interface server_xchange_if server server_xchange,
        swap_buffers buffers[]
#ifdef CTL_FROM_DSP
                             , interface ctl_from_dsp_if client ctl_from_dsp
#endif
                );

#endif /* CONTROL_SERVER_H_ */
