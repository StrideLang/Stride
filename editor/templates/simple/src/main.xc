/******************************************************************************\
 * File:	main.xc
 *
* Modified from project found here https://github.com/xcore/sw_audio_effects
\******************************************************************************/

#include "main.h"
#include <xs1.h>

#include "audio_io.h"
#include "switch_io.h"
#include "dsp_server.h"
#include "control_server.h"
#include "server_interface.h"
#include "compute.h"

//[[Main DSP includes]]
#include "dsp_0.h"
#include "dsp_1.h"
#include "dsp_2.h"
#include "dsp_3.h"
//[[/Main DSP includes]]

#ifdef USE_XSCOPE
#include <xscope.h>
/*****************************************************************************/
void xscope_user_init( void ) // 'C' constructor function (NB called before main)
{
    xscope_register( 4
        ,XSCOPE_CONTINUOUS ,"Signal1" ,XSCOPE_INT ,"n"
        ,XSCOPE_CONTINUOUS ,"Signal2" ,XSCOPE_INT ,"n"
        ,XSCOPE_CONTINUOUS ,"Signal3" ,XSCOPE_INT ,"n"
        ,XSCOPE_CONTINUOUS ,"Signal4" ,XSCOPE_INT ,"n"
//        ,XSCOPE_CONTINUOUS ,"GainReduction" ,XSCOPE_INT ,"n"
    ); // xscope_register

    xscope_config_io( XSCOPE_IO_BASIC ); // Enable XScope printing
} // xscope_user_init
#endif // ifdef USE_XSCOPE


// How to guarantee these are on the right tile?
// if assigned to tile, the compiler complains:
// ../src/main.xc:43: error: a variable declaration prefixed with on must declare an object of type port or clock
swap_buffers dsp_buffers_A[NUM_APP_CHANS];
swap_buffers dsp_buffers_B[NUM_APP_CHANS];

swap_buffers ctl_buffers_A[NUM_APP_CHANS];
swap_buffers ctl_buffers_B[NUM_APP_CHANS];

int main (void)
{
	streaming chan c_aud_I2S;
	streaming chan c_aud_dsp[NUM_APP_CHANS];
	interface ctl_from_dsp_if c_ctl_from_dsp;
    interface dsp_param_if dsp_params[NUM_APP_CHANS];
    interface computation_if comps;
    interface server_xchange_if server_xchange;

	par
	{
        // Audio
	    on tile[DSP_TILE]: audio_io( c_aud_I2S );
        on tile[DSP_TILE]: dsp_server(dsp_params, server_xchange, dsp_buffers_B);
        // Audio mixing and routing
        on tile[DSP_TILE]: switch_io( c_aud_I2S , c_aud_dsp, c_ctl_from_dsp);
        // DSP
//[[Main DSP]]
        on tile[DSP_TILE]: dsp_0(c_aud_dsp[0], dsp_params[0], dsp_buffers_A[0]);
        on tile[DSP_TILE]: dsp_1(c_aud_dsp[1], dsp_params[1], dsp_buffers_A[1]);
        on tile[DSP_TILE]: dsp_2(c_aud_dsp[2], dsp_params[2], dsp_buffers_A[2]);
        on tile[DSP_TILE]: dsp_3(c_aud_dsp[3], dsp_params[3], dsp_buffers_A[3]);
//[[/Main DSP]]
		// Control I/O
#ifdef CONTROL_COMBINABLE
		on tile[CTL_TILE]:
		[[combine]]
            par {
                control_server(comps, server_xchange, ctl_buffers_B,
                        c_ctl_from_dsp);
                        // Computation of parameters from control data.
                compute(comps, ctl_buffers_A);
            }
#else
        on tile[CTL_TILE]: control_server(comps, server_xchange, ctl_buffers_B,
                c_ctl_from_dsp);
                        // Computation of parameters from control data.
        on tile[CTL_TILE]: compute(comps, ctl_buffers_A);
#endif

	}

	return 0;
} // main
/*****************************************************************************/
// main.xc
