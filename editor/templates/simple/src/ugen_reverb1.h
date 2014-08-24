/*
 * ugen_reverb1.h
 *
 *  Created on: Apr 17, 2014
 *      Author: andres
 */

#ifndef UGEN_REVERB1_H_
#define UGEN_REVERB1_H_

#include "types64bit.h"

#define UGEN_REVERB1_COMB_LENGTH 4096
#define UGEN_REVERB1_ALLPASS_LENGTH 1024

typedef struct {
    S32_T dummy;
} ugen_reverb1_data_t;

typedef struct {
    R32_T freq;
} ugen_reverb1_controls_t;

typedef struct {
    S32_T readpos;
    S32_T writedelta;
    S32_T comb_line[UGEN_REVERB1_COMB_LENGTH];
} ugen_comb_state_t;

typedef struct {
    S32_T readpos;
    S32_T writedelta;
    S32_T ap_line[UGEN_REVERB1_ALLPASS_LENGTH];
} ugen_allpass_state_t;

typedef struct {
    ugen_reverb1_data_t * unsafe data;
    ugen_comb_state_t comb[4];
} ugen_reverb1_state_t;

void ugen_reverb1_init_data(ugen_reverb1_data_t * unsafe data);
void ugen_reverb1_init(ugen_reverb1_state_t &state, ugen_reverb1_data_t * unsafe data);
S32_T ugen_reverb1_tick(ugen_reverb1_state_t &state, S32_T sample);
unsafe void ugen_reverb1_ctl(ugen_reverb1_controls_t &controls, ugen_reverb1_data_t * unsafe data);



#endif /* UGEN_REVERB1_H_ */
