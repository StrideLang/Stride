#ifndef __CODEC_H
#define __CODEC_H

#include "stm32f7xx_hal.h"
#include <stdint.h>

typedef struct
{
  uint32_t  (*Init)(uint16_t, uint16_t, uint8_t, uint8_t, uint32_t);
  void      (*DeInit)(void);
  uint32_t  (*ReadID)(uint16_t);
  uint32_t  (*Play)(uint16_t, uint16_t*, uint16_t);
  uint32_t  (*Pause)(uint16_t);
  uint32_t  (*Resume)(uint16_t);
  uint32_t  (*Stop)(uint16_t, uint32_t);
  uint32_t  (*SetFrequency)(uint16_t, uint32_t);
  uint32_t  (*SetOutputVolume)(uint16_t, uint8_t);
  uint32_t  (*SetInputVolume)(uint16_t, uint8_t);
  uint32_t  (*SetMute)(uint16_t, uint32_t);
  uint32_t  (*SetOutputMode)(uint16_t, uint8_t);
  uint32_t  (*Reset)(uint16_t);
} AUDIO_DrvTypeDef;

/* Codec output DEVICE */
#define OUTPUT_DEVICE_SPEAKER                 ((uint16_t)0x0001)
#define OUTPUT_DEVICE_HEADPHONE               ((uint16_t)0x0002)
#define OUTPUT_DEVICE_BOTH                    ((uint16_t)0x0003)
#define OUTPUT_DEVICE_AUTO                    ((uint16_t)0x0004)
#define INPUT_DEVICE_DIGITAL_MICROPHONE_1     ((uint16_t)0x0100)
#define INPUT_DEVICE_DIGITAL_MICROPHONE_2     ((uint16_t)0x0200)
#define INPUT_DEVICE_INPUT_LINE_1             ((uint16_t)0x0300)
#define INPUT_DEVICE_INPUT_LINE_2             ((uint16_t)0x0400)
#define INPUT_DEVICE_BOTH                     ((uint16_t)0x0500)

/* Volume Levels values */
#define DEFAULT_VOLMIN                0x00
#define DEFAULT_VOLMAX                0xFF
#define DEFAULT_VOLSTEP               0x04

#define AUDIO_PAUSE                   0
#define AUDIO_RESUME                  1

/* Codec POWER DOWN modes */
#define CODEC_PDWN_HW                 1
#define CODEC_PDWN_SW                 2

/* MUTE commands */
#define AUDIO_MUTE_ON                 1
#define AUDIO_MUTE_OFF                0

/* AUDIO FREQUENCY */
#define AUDIO_FREQUENCY_192K          ((uint32_t)192000)
#define AUDIO_FREQUENCY_96K           ((uint32_t)96000)
#define AUDIO_FREQUENCY_48K           ((uint32_t)48000)
#define AUDIO_FREQUENCY_44K           ((uint32_t)44100)
#define AUDIO_FREQUENCY_32K           ((uint32_t)32000)
#define AUDIO_FREQUENCY_22K           ((uint32_t)22050)
#define AUDIO_FREQUENCY_16K           ((uint32_t)16000)
#define AUDIO_FREQUENCY_11K           ((uint32_t)11025)
#define AUDIO_FREQUENCY_8K            ((uint32_t)8000)

//#define VOLUME_OUT_CONVERT(Volume)    (((Volume) > 100)? 100:((uint8_t)(((Volume) * 63) / 100)))
//#define VOLUME_IN_CONVERT(Volume)     (((Volume) >= 100)? 239:((uint8_t)(((Volume) * 240) / 100)))

#define WM8994_ID                     0x8994
#define WM8994_CHIPID_ADDR            0x00

/* High Layer codec functions */
uint32_t wm8994_Init(uint16_t DeviceAddr, uint16_t OutputInputDevice, uint8_t OutputVolume, uint8_t InputVolume, uint32_t AudioFreq);
void     wm8994_DeInit(void);
uint32_t wm8994_ReadID(uint16_t DeviceAddr);
uint32_t wm8994_Play(uint16_t DeviceAddr, uint16_t* pBuffer, uint16_t Size);
uint32_t wm8994_Pause(uint16_t DeviceAddr);
uint32_t wm8994_Resume(uint16_t DeviceAddr);
uint32_t wm8994_Stop(uint16_t DeviceAddr, uint32_t Cmd);
uint32_t wm8994_SetOutputVolume(uint16_t DeviceAddr, uint8_t OutputVolume);
uint32_t wm8994_SetInputVolume(uint16_t DeviceAddr, uint8_t InputVolume);
uint32_t wm8994_SetMute(uint16_t DeviceAddr, uint32_t Cmd);
uint32_t wm8994_SetOutputMode(uint16_t DeviceAddr, uint8_t Output);
uint32_t wm8994_SetFrequency(uint16_t DeviceAddr, uint32_t AudioFreq);
uint32_t wm8994_Reset(uint16_t DeviceAddr);

#endif /* __CODEC_H */
