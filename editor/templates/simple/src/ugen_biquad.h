
#ifndef UGEN_BIQUAD_H
#define UGEN_BIQUAD_H

#include "types64bit.h"

#define BANKS 1

//typedef struct {S32_T b0, b1, b2, a1, a2;} coeff_T;

typedef enum {
    LOWPASS,
    HIGHPASS,
    BANDPASS,
    BANDPASS2,
    NOTCH,
    ALLPASS,
    PEAKING,
    LOWSHELF,
    HIGHSHELF,
    BYPASS
} ugen_biquad_type_t;


#define UGEN_BIQUAD_DATA_SIZE 5

typedef struct {
    S32_T coeffs[5];
} ugen_biquad_data_t;

typedef struct {
    ugen_biquad_data_t * unsafe data;
    struct {int xn1; int xn2; int db;} b[BANKS+1];
//    int adjustDelay;
//    int adjustCounter;
//    int desiredDb[BANKS];
} ugen_biquad_state_t;

typedef struct {
    R32_T cf;
    R32_T Q;
    R32_T dbGain;
    ugen_biquad_type_t type;
} ugen_biquad_controls_t;

void ugen_biquad_init_data(ugen_biquad_data_t * unsafe data);
void ugen_biquad_init_controls(ugen_biquad_controls_t &controls);
void ugen_biquad_init(ugen_biquad_state_t &state, ugen_biquad_data_t * unsafe biquad_data);
S32_T ugen_biquad_tick(ugen_biquad_state_t &state, S32_T sample);
void ugen_biquad_ctl(ugen_biquad_controls_t &controls, ugen_biquad_data_t * unsafe buffer);

#endif // UGEN_BIQUAD_H
