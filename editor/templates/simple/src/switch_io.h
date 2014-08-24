/*
 * switch_io.h
 *
 *  Created on: Mar 22, 2014
 *      Author: Pacifist
 */

#ifndef SWITCH_IO_H_
#define SWITCH_IO_H_

#include "app_global.h"
#include "types64bit.h"
#include "server_interface.h"

void switch_io(
    streaming chanend I2S_aud,
    streaming chanend c_aud_dsp[]
#ifdef CTL_FROM_DSP
                                ,
    interface ctl_from_dsp_if server ctl_from_dsp

#endif
);

#endif /* SWITCH_IO_H_ */
