/*
 * ugen_oscil.h
 *
 *  Created on: Apr 6, 2014
 *      Author: andres
 */

#ifndef UGEN_OSCIL_H_
#define UGEN_OSCIL_H_

#include "types64bit.h"

#define UGEN_OSCIL_CST_MAXLEN 0x1000000
#define UGEN_OSCIL_PHMASK 0x0FFFFFF
#define UGEN_OSCIL_TAB_LEN 4096

#define UGEN_OSCIL_DATA_SIZE 2

typedef struct {
    S32_T inc;
    double sicvt; // Not a parameter, but required for computation
} ugen_oscil_data_t;

//FIXME the design of this ugen is bad! How can we refill a table from the control server?
// it's to big to move across the control and DSP tiles, and we don't want to calculate
// This on the DSP... hoe to handle this??

typedef enum {
    UGEN_OSCIL_SHAPE_SAW = 0,
    UGEN_OSCIL_SHAPE_SQUARE = 1,
    UGEN_OSCIL_SHAPE_PULSE = 2,
    UGEN_OSCIL_SHAPE_PULSEBI = 3
} ugen_oscil_shape_t;

typedef struct {
    S32_T num_harm;
    ugen_oscil_shape_t shape;
    R32_T freq;
} ugen_oscil_controls_t;

typedef struct {
    ugen_oscil_data_t * unsafe data;
    S32_T table[UGEN_OSCIL_TAB_LEN];
    S32_T phase;
    S32_T lenmask;
    S32_T lobits;
    S32_T lomask;
    S32_T maxlen;
    S32_T lodiv;
} ugen_oscil_state_t;

void ugen_oscil_init_data(ugen_oscil_data_t * unsafe data);
void ugen_oscil_init_controls(ugen_oscil_controls_t & controls);
void ugen_oscil_init(ugen_oscil_state_t &state, ugen_oscil_data_t * unsafe data);
S32_T ugen_oscil_tick(ugen_oscil_state_t &state);
unsafe void ugen_oscil_ctl(ugen_oscil_controls_t & controls, ugen_oscil_data_t * unsafe data);

#endif /* UGEN_OSCIL_H_ */
