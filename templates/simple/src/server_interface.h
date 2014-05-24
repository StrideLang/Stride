/*
 * server_interface.h
 *
 *  Created on: Mar 27, 2014
 *      Author: andres
 */

#ifndef SERVER_INTERFACE_H_
#define SERVER_INTERFACE_H_

#include "types64bit.h"

#define CTL_FROM_DSP

//[[Server Controls]]
typedef enum {
    SINE_0_00_CONTROLS,
    ENV_0_00_CONTROLS,
    NOISE_0_00_CONTROLS,
    COMP_0_00_CONTROLS,

    BIQUAD_1_00_CONTROLS = 0x40, // 6 bits to encode number (0-64) -> 64 controls per core
    BLOSC_1_00_CONTROLS,

    BIQUAD_2_00_CONTROLS = 0x80,
    SINE_2_00_CONTROLS,

    BIQUAD_3_00_CONTROLS = 0xC0,

//    BIQUAD_4_00_CONTROLS = 0x100,
    //    BIQUAD_5_00_CONTROLS = 0x140,
    //    BIQUAD_6_00_CONTROLS = 0x180,
    //    BIQUAD_7_00_CONTROLS = 0x1C0,
} ugen_id_t;
//[[/Server Controls]]

typedef struct {
    ugen_id_t index;
    S32_T data[8]; // 8 is the maximum size of passed data
//    int datasize;
} xchange_t;

interface server_xchange_if {
    [[clears_notification]] xchange_t get_data();
    [[notification]] slave void data_ready(void);
};

typedef struct {
//    S32_T index;
    S32_T data[8];
//    int datasize;
} ctl_from_dsp_t;

interface ctl_from_dsp_if {
    [[clears_notification]] ctl_from_dsp_t get_data();
    [[notification]] slave void data_ready(void);
};

// TODO is there a way to know these sizes from the code?
//[[Server Buffers]]
typedef struct {
    S32_T sine_0_00_data[2];
    S32_T env_0_00_data[6];
    S32_T noise_0_00_data[1];
    S32_T comp_0_00_data[6];

    S32_T biquad_1_00_data[5];
    S32_T blosc_1_00_data[2];

    S32_T biquad_2_00_data[5];
    S32_T sine_2_00_data[2];

    S32_T biquad_3_00_data[5];
} swap_buffers;
//[[/Server Buffers]]

#endif /* SERVER_INTERFACE_H_ */
