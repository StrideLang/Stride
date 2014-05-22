/******************************************************************************\
 * File:	codec.xc
 *
 * Description: Codec Configuration
 *
 * Version: 0v1
 * Build:
 *
 * The copyrights, all other intellectual and industrial
 * property rights are retained by XMOS and/or its licensors.
 * Terms and conditions covering the use of this code can
 * be found in the Xmos End User License Agreement.
 *
 * Copyright XMOS Ltd 2012
 *
 * In the case where this code is a modification of existing code
 * under a separate license, the separate license terms are shown
 * below. The modifications to the code are still covered by the
 * copyright notice above.
 *
\******************************************************************************/

#include "codec.h"

#define DO_READ_STATUS 0

/*****************************************************************************/
void codec_config( // Configure Codec
	unsigned samFreq, // Sample Frequency
	unsigned mClk // Master Clock
)
{
    timer t;
    unsigned time;
// 20140222-JT  SET "tmp" to 0x00;
    //unsigned tmp;
    unsigned tmp = 0x00;

// 20140222-JT  THIS IS NOT BEING USED
    unsigned char data[1] = {0};

    /* Set CODEC in reset */
// 20140222-JT  THIS SET THE RESET PIN TO HIGH AND THAT'S WRONG I.E CODEC IS ACTIVE!
    //tmp = P_GPIO_COD_RST_N;


    /* Set master clock select appropriately */
    if ((samFreq % 22050) == 0)
    {
        tmp &= ~P_GPIO_MCLK_SEL;
    }
    else //if((samFreq % 24000) == 0)
    {
        tmp |= P_GPIO_MCLK_SEL;
    }

    /* Output to port */
    p_gpio <: tmp;

    /* Hold in reset for 2ms while waiting for MCLK to stabilise */
    t :> time;
// 20140222-JT      TIMER RUNS AT 100 MHZ -> 10 ticks / nSec
    time += 200000;
// 20140222-JT      CHANGED "int _" to "void"
    //t when timerafter(time) :> int _;
    t when timerafter(time) :> void;

    /* CODEC out of reset */
    tmp |= P_GPIO_COD_RST_N;
    p_gpio <: tmp;

    /* Set power down bit in the CODEC over I2C */
    //IIC_REGWRITE(CODEC_DEV_ID_ADDR, 0x01);
// 20140222-JT      CODEC_DEV_ID_ADDR MUST BE REPLACED WITH CODEC_PWR_CTRL_ADDR
    IIC_REGWRITE(CODEC_PWR_CTRL_ADDR, 0x01);

// 20140222-JT  THIS IS NOT YET AVAILABLE
#if DO_READ_STATUS
    /* Read CODEC device ID to make sure everything is OK */
    IIC_REGREAD(CODEC_DEV_ID_ADDR, data);

    int codec_dev_id = data[0];
    if (((codec_dev_id & 0xF0) >> 4) != 0xC)
    {
        printstr("Unexpected CODEC Device ID, expected 0xC, got ");
        printhex(codec_dev_id);
    }
#endif

    /* Now set all registers as we want them :
    Mode Control Reg:
    Set FM[1:0] as 11. This sets Slave mode.
    Set MCLK_FREQ[2:0] as 010. This sets MCLK to 512Fs in Single, 256Fs in Double and 128Fs in Quad Speed Modes.
// 20140222-JT  THE FOLLOWING LINES ARE JUST THE MCLK RATES. HAVE NOTHING TO DO WITH REGISTER.
    This means 24.576MHz for 48k and 22.5792MHz for 44.1k.
    Set Popguard Transient Control.
    So, write 0x35. */
    IIC_REGWRITE(CODEC_MODE_CTRL_ADDR,    0x35);

    /* ADC & DAC Control Reg:
       Leave HPF for ADC inputs continuously running.
       Digital Loopback: OFF
       DAC Digital Interface Format: I2S
       ADC Digital Interface Format: I2S
       So, write 0x09. */
    IIC_REGWRITE(CODEC_ADC_DAC_CTRL_ADDR, 0x09);

    /* Transition Control Reg:
       No De-emphasis. Don't invert any channels. Independent vol controls. Soft Ramp and Zero Cross enabled.*/
    IIC_REGWRITE(CODEC_TRAN_CTRL_ADDR,    0x60);

    /* Mute Control Reg: Turn off AUTO_MUTE */
    IIC_REGWRITE(CODEC_MUTE_CTRL_ADDR,    0x00);

    /* DAC Chan A Volume Reg:
       We don't require vol control so write 0x00 (0dB) */
    IIC_REGWRITE(CODEC_DACA_VOL_ADDR,     0x00);

    /* DAC Chan B Volume Reg:
       We don't require vol control so write 0x00 (0dB)  */
    IIC_REGWRITE(CODEC_DACB_VOL_ADDR,     0x00);

    /* Clear power down bit in the CODEC over I2C */
    IIC_REGWRITE(CODEC_PWR_CTRL_ADDR, 0x00);


//  20140222-JT  WAIT FOR 0.4 seconds for OUTPUT TO REACH VQ (POPGUARD)
    t :> time;
    time += 40000000;
    t when timerafter(time) :> void;

} // codec_config
/*****************************************************************************/
// codec.xc
