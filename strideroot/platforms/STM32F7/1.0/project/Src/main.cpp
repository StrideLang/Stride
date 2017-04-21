/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  *
  * COPYRIGHT(c) 2016 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"
#include "dma.h"
#include "i2c.h"
#include "sai.h"
#include "gpio.h"

/* USER CODE BEGIN Includes */
#include "codec.h"
#include "math.h"
#include "arm_math.h"

//[[Includes]]

//[[/Includes]]
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
#define AUDIO_I2C_ADDRESS           ((uint16_t)0x34)
extern AUDIO_DrvTypeDef wm8994_drv;

#define BUFFER_SIZE				2
static uint16_t Buffer[BUFFER_SIZE] = { 0,0 };

// 0x00 = -57 dB
// 0x27 = -18dB
// 0x2D = -12 dB
// 0x33 =  -6 dB
// 0x39 =   0 db
// 0x3F =  +6 db
static uint32_t OutputVolume = 0x27;

// 0x00 = -71.625 dB
// 0x90 = -18.000 dB
// 0xA0 = -12.000 dB
// 0xB0 =  -6.000 dB
// 0xC0 =   0.000 dB
// 0cFF = +17.625 dB

static uint32_t InputVolume = 0xA0;


//[[Declarations]]
static float32_t PhaseInc_f32[4];
static float32_t Phase_f32[4];
static float32_t Partials_f32[4];
static float32_t Sample_f32;
static q15_t Sample_q15;

//Rate for CODEC hardcoded 'AUDIO_FREQUENCY_48K'
//static float32_t Fs = 48000;


static float32_t Freq[4] = { 110.00, 3.0, 440.0, 10.0 };
static float32_t Amp[4] = { 0.4, 0.3, 0.2, 0.1 };

//[[/Declarations]]


//[[Instances]]
//[[/Instances]]


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/
static void Codec_Init(void);
/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

int main(void)
{

  /* USER CODE BEGIN 1 */

//[[Initialization]]

//[[/Initialization]]

  /* USER CODE END 1 */

  /* Enable I-Cache-------------------------------------------------------------*/
  SCB_EnableICache();

  /* Enable D-Cache-------------------------------------------------------------*/
  SCB_EnableDCache();

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_I2C3_Init();
  MX_SAI2_Init();

  /* USER CODE BEGIN 2 */

  __HAL_SAI_ENABLE(&hsai_BlockA2);

  Codec_Init();

  HAL_SAI_Transmit_IT(&hsai_BlockA2,(uint8_t *) Buffer, BUFFER_SIZE );

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

  /* USER CODE END WHILE */

    //[[Processing]]
    //[[/Processing]]
  /* USER CODE BEGIN 3 */

  }
    //[[Cleanup]]
    //[[/Cleanup]]
  /* USER CODE END 3 */

}

/** System Clock Configuration
*/
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;

  __HAL_RCC_PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 432;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  HAL_PWREx_EnableOverDrive();

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7);

  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SAI2|RCC_PERIPHCLK_I2C3;
  PeriphClkInitStruct.PLLI2S.PLLI2SN = 344;
  PeriphClkInitStruct.PLLI2S.PLLI2SP = 1;
  PeriphClkInitStruct.PLLI2S.PLLI2SR = 2;
  PeriphClkInitStruct.PLLI2S.PLLI2SQ = 7;
  PeriphClkInitStruct.PLLI2SDivQ = 1;
  PeriphClkInitStruct.Sai2ClockSelection = RCC_SAI2CLKSOURCE_PLLI2S;
  PeriphClkInitStruct.I2c3ClockSelection = RCC_I2C3CLKSOURCE_PCLK1;
  HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);

  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* USER CODE BEGIN 4 */
void Codec_Init(void)
{
	uint32_t deviceid = 0x00;
	deviceid = wm8994_drv.ReadID(AUDIO_I2C_ADDRESS);

	if((deviceid) == WM8994_ID)
	{
		wm8994_drv.Reset(AUDIO_I2C_ADDRESS);
		wm8994_drv.Init(AUDIO_I2C_ADDRESS, OUTPUT_DEVICE_HEADPHONE, OutputVolume, InputVolume, AUDIO_FREQUENCY_48K);

		HAL_GPIO_TogglePin(GPIOI,GPIO_PIN_1);
	}
}

void HAL_SAI_TxCpltCallback(SAI_HandleTypeDef *hsai)
{


	for (uint8_t i = 0; i < 4 ; i++)
	{

//[[AudioProcessing]]

		Sample_f32 = 0.0f;

		for (uint8_t j = 0; j < 4 ; j++)
		{
			Phase_f32[j] += PhaseInc_f32[j];

			if (Phase_f32[j] > 6.28318530718f) Phase_f32[j] -= 6.28318530718f;

			Partials_f32[j] = arm_sin_f32 (Phase_f32[j]);

			Sample_f32 += Partials_f32[j] * Amp[j];
		}

		Sample_f32  = Partials_f32[0] * Partials_f32[1] * Amp[0] + Partials_f32[2] * Partials_f32[3] * Amp[1];

		arm_float_to_q15(&Sample_f32,&Sample_q15,1);

		hsai_BlockA2.Instance->DR = Sample_q15;
		hsai_BlockA2.Instance->DR = Sample_q15;


//[[/AudioProcessing]]
	}

	__HAL_SAI_ENABLE_IT(&hsai_BlockA2,SAI_IT_FREQ);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == GPIO_PIN_11 )
	{

	}
}


/* USER CODE END 4 */

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

/**
  * @}
  */

/**
  * @}
*/

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
