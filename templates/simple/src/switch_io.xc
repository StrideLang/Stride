/*
 * switch_io.xc
 *
 *  Created on: Mar 22, 2014
 *      Author: Pacifist
 */

#include "switch_io.h"

#ifdef USE_XSCOPE
#include <xscope.h>
#endif

S32_T counter;

void switch_io(
    streaming chanend I2S_aud,
    streaming chanend c_aud_dsp[]
#ifdef CTL_FROM_DSP
                                ,
    interface ctl_from_dsp_if server ctl_from_dsp
#endif
    )
{
    S32_T inp_samps[NUM_APP_CHANS];
    S32_T out_samps[NUM_APP_CHANS];
    S32_T mix_samps[NUM_APP_CHANS];

    S32_T chan_cnt;
    for (chan_cnt = 0; chan_cnt < NUM_APP_CHANS; chan_cnt++)
    {
        inp_samps[chan_cnt] = 0;
        out_samps[chan_cnt] = 0;
        mix_samps[chan_cnt] = 0;
    }
    counter = 0;

    while(1)
    {
        #pragma loop unroll
        for (chan_cnt = 0; chan_cnt < NUM_APP_CHANS; chan_cnt++)
        {
            I2S_aud :> inp_samps[chan_cnt];
            I2S_aud <: mix_samps[chan_cnt];

#ifdef USE_XSCOPE
        xscope_int(chan_cnt, mix_samps[chan_cnt]);
#endif
        }

        for (chan_cnt = 0; chan_cnt < NUM_APP_CHANS; chan_cnt++)
        {
            c_aud_dsp[chan_cnt] <: inp_samps[chan_cnt];
            c_aud_dsp[chan_cnt] :> out_samps[chan_cnt];
        }


        for (chan_cnt = 0; chan_cnt < NUM_APP_CHANS; chan_cnt++)
        {
            mix_samps[chan_cnt] =  out_samps[chan_cnt] >> 0;
        }

        counter++;
        if (counter == 4410) {
            ctl_from_dsp.data_ready();
            counter = 0;
        }
        select {
        case ctl_from_dsp.get_data() -> ctl_from_dsp_t ctl_from_dsp_data:
            ctl_from_dsp_data.data[0] = mix_samps[0];
            ctl_from_dsp_data.data[1] = mix_samps[1];
#ifndef SINGLE_TILE
            ctl_from_dsp_data.data[2] = mix_samps[2];
            ctl_from_dsp_data.data[3] = mix_samps[3];
#endif
            break;
        default:
            break;
        }

    }

}
