/*
 * ugen_modsine.h
 *
 *  Created on: Jul 21, 2014
 *      Author: andres
 */

#ifndef UGEN_MODSINE_H_
#define UGEN_MODSINE_H_

#include "types64bit.h"

#define UGEN_MODSINE_TAB_LEN 1024

#define UGEN_MODSINE_DEFAULT_FREQ 440.0

#define UGEN_MODSINE_DATA_SIZE 2

typedef struct {
    S32_T inc;
    R32_T sicvt; // Not a parameter, but required for computation
} ugen_modsine_data_t;

typedef struct {
    R32_T freq;
} ugen_modsine_controls_t;

// TODO should this ugen be modified to allow intialization time setting of waveform?

typedef struct {
    ugen_modsine_data_t * unsafe data;
    S32_T table[UGEN_MODSINE_TAB_LEN]; // TODO ideally share this table and only fill it once
    S32_T phase;
    S32_T lobits;
    S32_T lomask;
    S32_T maxlen;
    S32_T lodiv;
} ugen_modsine_state_t;

void ugen_modsine_init_data(ugen_modsine_data_t * unsafe data);
void ugen_modsine_init_controls(ugen_modsine_controls_t &controls);
void ugen_modsine_init(ugen_modsine_state_t &state, ugen_modsine_data_t * unsafe data);
S32_T ugen_modsine_tick(ugen_modsine_state_t &state, S32_T phsmod, S32_T depth);
unsafe void ugen_modsine_ctl(ugen_modsine_controls_t &controls, ugen_modsine_data_t * unsafe data);


#endif /* UGEN_MODSINE_H_ */
