/*
 * ugen_env.h
 *
 *  Created on: Apr 11, 2014
 *      Author: andres
 */

#ifndef UGEN_ENV_H_
#define UGEN_ENV_H_

#include "types64bit.h"

typedef enum {
    UGEN_ENV_ATT = 0,
    UGEN_ENV_DEC = 1,
    UGEN_ENV_SUS = 2,
    UGEN_ENV_REL = 3,
    UGEN_ENV_DONE = 4,
} ugen_env_stage_t;


#define UGEN_ENV_DATA_SIZE 6

typedef struct {
    S32_T gate; // gate is enconded in bit 4. bits 1-3 encode stage
    S32_T incrs[4];
    S32_T sus_level;
} ugen_env_data_t;

typedef struct {
    R32_T att;
    R32_T dec;
    R32_T sus;
    R32_T rel;
    S32_T gate;
} ugen_env_controls_t;

typedef struct {
    ugen_env_data_t * unsafe data;
    ugen_env_stage_t stage; // current stage
    S32_T cur_value;
} ugen_env_state_t;

void ugen_env_init_data(ugen_env_data_t * unsafe data);
void ugen_env_init_controls(ugen_env_controls_t &controls);
void ugen_env_init(ugen_env_state_t &state, ugen_env_data_t * unsafe data);
S32_T ugen_env_tick(ugen_env_state_t &state);
unsafe void ugen_env_ctl(ugen_env_controls_t &controls, ugen_env_data_t * unsafe data);

#endif /* UGEN_ENV_H_ */
