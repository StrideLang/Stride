/*
 * ugen_sine.h
 *
 *  Created on: Apr 17, 2014
 *      Author: andres
 */

#ifndef UGEN_SINE_H_
#define UGEN_SINE_H_

#include "types64bit.h"

#define UGEN_SINE_CST_MAXLEN 0x1000000
#define UGEN_SINE_PHMASK 0x0FFFFFF
#define UGEN_SINE_TAB_LEN 1024

#define UGEN_SINE_DEFAULT_FREQ 440.0

#define UGEN_SINE_DATA_SIZE 2

typedef struct {
    S32_T inc;
    R32_T sicvt; // Not a parameter, but required for computation
} ugen_sine_data_t;

typedef struct {
    R32_T freq;
} ugen_sine_controls_t;

// TODO should this ugen be modified to allow intialization time setting of waveform?

typedef struct {
    ugen_sine_data_t * unsafe data;
    S32_T table[UGEN_SINE_TAB_LEN]; // TODO ideally share this table and only fill it once
    S32_T phase;
    S32_T lenmask;
    S32_T lobits;
    S32_T lomask;
    S32_T maxlen;
    S32_T lodiv;
} ugen_sine_state_t;

void ugen_sine_init_data(ugen_sine_data_t * unsafe data);
void ugen_sine_init_controls(ugen_sine_controls_t &controls);
void ugen_sine_init(ugen_sine_state_t &state, ugen_sine_data_t * unsafe data);
S32_T ugen_sine_tick(ugen_sine_state_t &state);
unsafe void ugen_sine_ctl(ugen_sine_controls_t &controls, ugen_sine_data_t * unsafe data);


#endif /* UGEN_SINE_H_ */
