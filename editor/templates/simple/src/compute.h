/*
 * compute.h
 *
 *  Created on: Mar 26, 2014
 *      Author: Pacifist
 */

#ifndef COMPUTE_H_
#define COMPUTE_H_

#include "control_server.h"

#ifdef CONTROL_COMBINABLE
[[combinable]]
#endif
void compute(interface computation_if client comps,
        swap_buffers buffers[]);

#endif /* COMPUTE_H_ */
