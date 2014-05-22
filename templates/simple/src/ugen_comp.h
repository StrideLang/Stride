/*
 * ugen_comp.h
 *
 *  Created on: May 1, 2014
 *      Author: andres
 */

#ifndef UGEN_COMP_H_
#define UGEN_COMP_H_

#include "types64bit.h"


#define UGEN_COMP_DATA_SIZE 6

#define MAX_LOOKAHEAD_SAMPLES 512

#define MAX_ENV_WINDOW_SAMPLES 512
#define ENV_WINDOW_SHIFT 9 // 2^ENV_WINDOW_SHIFT = MAX_ENV_WINDOW_SAMPLES

typedef struct {
    S32_T la_delay_delta;
    S32_T attack_constant;
    S32_T release_constant;
    S32_T threshold;
    S32_T invratio;
} ugen_comp_data_t;

typedef struct {
    R32_T threshold; // in dB FS value
    R32_T ratio; // inverse ratio factor
    R32_T att_time; // in dB FS value
    R32_T rel_time; // inverse ratio factor

    R32_T lookahead_time;
//    R32_T env_smoothing; // A value between 0-1 to move between smoothing and no smoothing
} ugen_comp_controls_t;

typedef struct {
    ugen_comp_data_t * unsafe data;

    S32_T gainTarget;
    S32_T curGain;

    S32_T la_delay[MAX_LOOKAHEAD_SAMPLES];
    S32_T la_delay_write;

    S32_T env_delay[MAX_ENV_WINDOW_SAMPLES];
    S32_T env_delay_write;
    S32_T env_accumulator;
} ugen_comp_state_t;

void ugen_comp_init_data(ugen_comp_data_t * unsafe data);
void ugen_comp_init_controls(ugen_comp_controls_t &controls);
void ugen_comp_init(ugen_comp_state_t &state, ugen_comp_data_t * unsafe data);
S32_T ugen_comp_tick(ugen_comp_state_t &state, S32_T input);
unsafe void ugen_comp_ctl(ugen_comp_controls_t &controls, ugen_comp_data_t * unsafe data);


#endif /* UGEN_COMP_H_ */
