/*
 * ugen_noise.h
 *
 *  Created on: Apr 29, 2014
 *      Author: andres
 */

#ifndef UGEN_NOISE_H_
#define UGEN_NOISE_H_

#include "types64bit.h"
#include <random.h>

#define UGEN_NOISE_DATA_SIZE 1

typedef struct {
    S32_T numsamps;
} ugen_noise_data_t;

typedef struct {
    R32_T sampholdfreq; // sample and hold frequency 0 means no sample and hold
} ugen_noise_controls_t;

typedef struct {
    ugen_noise_data_t * unsafe data;
    S32_T counter;
    S32_T prev_samp;
    random_generator_t random_generator;
} ugen_noise_state_t;

void ugen_noise_init_data(ugen_noise_data_t * unsafe data);
void ugen_noise_init_controls(ugen_noise_controls_t &controls);
void ugen_noise_init(ugen_noise_state_t &state, ugen_noise_data_t * unsafe data);
S32_T ugen_noise_tick(ugen_noise_state_t &state);
unsafe void ugen_noise_ctl(ugen_noise_controls_t &controls, ugen_noise_data_t * unsafe data);

#endif /* UGEN_NOISE_H_ */
