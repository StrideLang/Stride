/*
 * ugen_blosc.h
 *
 *  Created on: May 2, 2014
 *      Author: andres
 */

#ifndef UGEN_BLOSC_H_
#define UGEN_BLOSC_H_

#include "types64bit.h"

#define UGEN_BLOSC_CST_MAXLEN 0x1000000
#define UGEN_BLOSC_PHMASK 0x0FFFFFF
#define UGEN_BLOSC_TAB_LEN 1024

#define UGEN_BLOSC_DEFAULT_FREQ 440.0

//#define UGEN_BLOSC_DATA_SIZE 2

// Oscillator code adapted the audio programming book DVD chapter 3
// Band-limited FM technique by Lazzarini and Timoney

typedef struct {
    S32_T inc;
    S32_T kn2; // 2 times kn
    R32_T sicvt; // Not a parameter, but required for computation
} ugen_blosc_data_t;

typedef struct {
    R32_T freq;
} ugen_blosc_controls_t;

typedef struct {
    ugen_blosc_data_t * unsafe data;
    S32_T table[UGEN_BLOSC_TAB_LEN]; // TODO ideally share this table and only fill it once
    S32_T phase;
    S32_T lenmask;
    S32_T lobits;
    S32_T lomask;
    S32_T maxlen;
    S32_T lodiv;
} ugen_blosc_state_t;

void ugen_blosc_init_data(ugen_blosc_data_t * unsafe data);
void ugen_blosc_init_controls(ugen_blosc_controls_t &controls);
void ugen_blosc_init(ugen_blosc_state_t &state, ugen_blosc_data_t * unsafe data);
S32_T ugen_blosc_tick(ugen_blosc_state_t &state, S32_T input);
void ugen_blosc_ctl(ugen_blosc_controls_t &controls, ugen_blosc_data_t * unsafe data);

#endif /* UGEN_BLOSC_H_ */
