/*
 * odo_basic.xc
 *
 *  Created on: Mar 6, 2014
 *      Author: andres
 */

#include <xs1.h>
#include <platform.h>
#include <timer.h>

#include <print.h>

#include <limits.h>


/*****************************************************************************/
//#define USE_XSCOPE 1

#ifdef USE_XSCOPE
#include <xscope.h>

void xscope_user_init( void ) // 'C' constructor function (NB called before main)
{
    xscope_register( 1
        ,XSCOPE_CONTINUOUS ,"Output" ,XSCOPE_INT ,"n"
//        ,XSCOPE_CONTINUOUS ,"Left out" ,XSCOPE_INT ,"n"
//        ,XSCOPE_CONTINUOUS ,"Mod" ,XSCOPE_FLOAT ,"n"
    ); // xscope_register

    xscope_config_io( XSCOPE_IO_BASIC ); // Enable XScope printing
} // xscope_user_init
#endif // ifdef USE_XSCOPE


typedef signed int S32_T;

//[[Basic Config]]
#define NUM_IN_CHANS 4
#define NUM_OUT_CHANS 4

#define SAMPLE_RATE 44100
//[[/Basic Config]]

// ------------------ UGENS

//[Shared data]]
#define TABLE1_LEN 2048
S32_T table1[TABLE1_LEN];
//[/Shared data]]

//[[Ugen structs]]
typedef struct {
    S32_T phs;
} OSCDATA;

typedef struct {
    S32_T phs;
    S32_T att_incr;
    S32_T dec_incr;
    S32_T sus_lvl;
    S32_T rel_incr;
    unsigned char mode; //0 - attack 1 - decay 2- release
} ENVDATA;

typedef struct {
    float gain;
} GAINDATA;

//[[/Ugen structs]]

// ---------------------------------------- Audio I/O

interface audio_if {
  void samples_ready(S32_T val[NUM_IN_CHANS]);
  void fill_output(S32_T val[NUM_OUT_CHANS]);
};

[[combinable]]
void audio_io_thread(interface audio_if client audio_io)
{
    timer tmr;
    unsigned int t;
    unsigned int incr = XS1_TIMER_MHZ * 1000000.0/44100.0;
//    printintln(incr);
    tmr :> t;
    S32_T inp_samps[NUM_IN_CHANS];
    S32_T out_samps[NUM_OUT_CHANS];

    inp_samps[0] = 10;
    while (1) {
        select {
            case tmr when timerafter(t) :> void :
                audio_io.samples_ready(inp_samps);
                audio_io.fill_output(out_samps);

#ifdef USE_XSCOPE
                xscope_int(0,out_samps[0]);
#endif
                t += incr;
                break;
        }
    }
}

// Control I/O processing ------------------

// GPIO
#include "startkit_gpio.h"

//[[Control globals]]
#define NUM_CTLS 2

typedef struct {
    float value;
    // maybe type here or an additional field for name?
} ctl_t;

ctl_t controls[NUM_CTLS]; // TODO should set to some default values

interface control_in_if {
  void setControl1(float val);
  void setControl2(float val);
};

interface param_if {
  void setParam1(float val);
  void setParam2(float val);
};
//[[/Control globals]]

[[distributable]]
void process_control_in(interface control_in_if server c, interface param_if client parameter_set)
{
    while (1) {
        select {
            //[[Control Processing]]
        case c.setControl1(float val):
                controls[0].value = val;
                parameter_set.setParam1(val);

        break;
        case c.setControl2(float val):
                controls[1].value += val;
                parameter_set.setParam2(controls[1].value);

        break;
            //[[/Control Processing]]
        }
    }
}

//[[Control Input]]
void control_thread(client interface startkit_led_if i_led,
        client interface startkit_button_if i_button,
        client interface slider_if i_slider_x,
        client interface slider_if i_slider_y,
        client interface control_in_if control_conn)
{
    int slider_pos_x, slider_pos_y;

    while(1) {

      select{
          case i_slider_y.changed_state():            //CHange frequency (ie. step size)
              slider_pos_y = i_slider_y.get_coord();
              i_slider_y.get_slider_state();          //necessary to clear notification
              printint(slider_pos_y);
              printstrln(" slider_pos_y");
              break;

          case i_slider_x.changed_state():            //CHange modulation depth (ie. amplitude)
              slider_pos_x = i_slider_x.get_coord();
              i_slider_x.get_slider_state();          //necessary to clear notification
              printint(slider_pos_x);
              printstrln(" slider_pos_x");
              control_conn.setControl2(1.0);
              break;
          case i_button.changed():
            if (i_button.get_value() == BUTTON_DOWN){
//                printstrln("Button down");
                control_conn.setControl1(1.0);

            }
            if (i_button.get_value() == BUTTON_UP){
//              printstrln("Button up");
              control_conn.setControl1(0.0);

            }
            break;
      }
    }
}
//[[/Control Input]]

// -------------- Audio processing function


void process(interface audio_if server audio_io,
        interface param_if server param)
{
    S32_T out_buffer[NUM_OUT_CHANS];
    //[[Init Ugens]]
    for (int i = 0; i < TABLE1_LEN; i++) {
        table1[i] = (S32_T) (i * 65530 / (float)TABLE1_LEN);
    }

    OSCDATA oscdata1;
    oscdata1.phs = 0;

    ENVDATA envdata1;
    envdata1.phs = 0;
    envdata1.att_incr = 100;
    envdata1.dec_incr = 100;
    envdata1.sus_lvl = 20000 ;
    envdata1.rel_incr = 10;

    envdata1.mode = 0;

    GAINDATA gaindata1;
    gaindata1.gain = 1.0;
    //[[/Init Ugens]]

    while(1) {
        select {
            case audio_io.samples_ready(S32_T samples[NUM_IN_CHANS]):
                //[[Audio Processing]]
                S32_T chan_cnt;
                S32_T audio_sample;
            #pragma loop unroll
                for (chan_cnt = 0; chan_cnt < NUM_IN_CHANS; chan_cnt++) { // TODO do something with audio IO
//                    audio_io :> inp_samps[chan_cnt];
                }
                for (chan_cnt = 0; chan_cnt < NUM_OUT_CHANS; chan_cnt++) {
                    out_buffer[chan_cnt] = table1[oscdata1.phs] * gaindata1.gain;

        //          Tick ugens
                    if (++oscdata1.phs >= TABLE1_LEN) {
                        oscdata1.phs = 0;
                    }
                }
                //[[/Audio Processing]]

            break;
            case audio_io.fill_output(S32_T samples[NUM_OUT_CHANS]):
                 S32_T chan_cnt;
                 for (chan_cnt = 0; chan_cnt < NUM_OUT_CHANS; chan_cnt++) {
                     samples[chan_cnt] = out_buffer[chan_cnt]; // Better with a memset or pass the pointer?
                 }

            break;

            case param.setParam1(float val):
                gaindata1.gain = val;

            break;

            case param.setParam2(float val):


            break;

        }
    }

}

// ----------------------- Main
//[[Main function]]
#define CONTROL_TILE 0
#define DSP_TILE 0

on tile[CONTROL_TILE]: startkit_gpio_ports gpio_ports =
  {XS1_PORT_32A, XS1_PORT_4A, XS1_PORT_4B, XS1_CLKBLK_3};

int main() {
    // Control interfaces need to be defined here:
    startkit_led_if i_led;
    startkit_button_if i_button;
    slider_if i_slider_x, i_slider_y;


    // Connection interfaces. Arrays allow connecting more flexibly between cores.
    interface control_in_if control_io[1];
    interface param_if param[1];
    interface audio_if audio_in[1];


    par {
        on tile[CONTROL_TILE]: audio_io_thread(audio_in[0]);
        on tile[CONTROL_TILE]: control_thread(i_led, i_button, i_slider_x, i_slider_y, control_io[0]);
        on tile[CONTROL_TILE]: startkit_gpio_driver(i_led, i_button,
                                         i_slider_x,
                                         i_slider_y,
                                         gpio_ports);
        on tile[CONTROL_TILE]: [[distribute]] process_control_in(control_io[0], param[0]);
        on tile[DSP_TILE]: process(audio_in[0], param[0]);
    }
    return 0;
}

//[[/Main function]]
