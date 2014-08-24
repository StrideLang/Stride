/*
 *  dsp_server.h
 *
 *  Created on: Mar 26, 2014
 *      Author: andres
 */

#ifndef DSP_SERVER_H_
#define DSP_SERVER_H_

#include "types64bit.h"
#include "server_interface.h"

#include "ugen_includes.h"

interface dsp_param_if {
    ugen_id_t get_data_id();
    [[clears_notification]] void * unsafe  get_data(void * unsafe dsp_buffer);
    [[notification]] slave void data_ready(void);
};

void dsp_server(interface dsp_param_if server dsp_params[],
        interface server_xchange_if client server_xchange,
        swap_buffers buffer[]);

#endif /* DSP_SERVER_H_ */
