/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  *
  * COPYRIGHT(c) 2017 STMicroelectronics
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
#include "main.h"
#include "stm32f7xx_hal.h"
#include "dfsdm.h"
#include "dma.h"
#include "i2c.h"
#include "i2s.h"
#include "quadspi.h"
#include "sai.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "fmc.h"

/* USER CODE BEGIN Includes */
//[[Includes]]
//[[/Includes]]
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
#define SI5351C_ADDRESS_W				0xC0
#define SI5351C_ADDRESS_R				0xC1

#define AK4558_ADDRESS_W				0x20
#define AK4558_ADDRESS_R				0x21

#define SI5351C_NUM_REGS                75
#define AK4558_NUM_REGS					5

#define AUDIO_BUFFER_SIZE				4
#define UART_BUFFER_SIZE				10
#define DFSDM_BUFFER_SIZE				32

  typedef struct
  {
      unsigned char address;	/* 8-bit register address */
      unsigned char value;		/* 8-bit register data */

  } si5351c_register_t;

  si5351c_register_t const si5351c_revb_registers[SI5351C_NUM_REGS] =
  {

      { 0x03, 0xFF },       // Disable Outputs
      { 0x10, 0x80 },       // Power Down Output 0
      { 0x11, 0x80 },       // Power Down Output 1
      { 0x12, 0x80 },       // Power Down Output 2
      { 0x13, 0x80 },       // Power Down Output 3
      { 0x14, 0x80 },       // Power Down Output 4
      { 0x15, 0x80 },       // Power Down Output 5
      { 0x16, 0x80 },       // Power Down Output 6
      { 0x17, 0x80 },		// Power Down Output 7
      { 0x02, 0x40 },		// Do Not Mask: SYS_INIT, LOL_A (Loss of Lock), LOS (Loss of Signal CLKIN)
      { 0x0F, 0x04 },       // Select CLKIN as PLLA reference
      { 0x10, 0x0F },
      { 0x11, 0x0F },
      { 0x12, 0x0F },
      { 0x13, 0x0F },
      { 0x14, 0x8C },
      { 0x15, 0x8C },
      { 0x16, 0x8C },
      { 0x17, 0x8C },
      { 0x1A, 0x0C },
      { 0x1B, 0x35 },
      { 0x1C, 0x00 },
      { 0x1D, 0x0F },
      { 0x1E, 0xF0 },
      { 0x1F, 0x00 },
      { 0x20, 0x09 },
      { 0x21, 0x50 },
      { 0x2A, 0x0C },
      { 0x2B, 0x35 },
      { 0x2C, 0x00 },
      { 0x2D, 0x0F },
      { 0x2E, 0xF0 },
      { 0x2F, 0x00 },
      { 0x30, 0x09 },
      { 0x31, 0x50 },
      { 0x32, 0x00 },
      { 0x33, 0x7D },
      { 0x34, 0x00 },
      { 0x35, 0x10 },
      { 0x36, 0xB0 },
      { 0x37, 0x00 },
      { 0x38, 0x00 },
      { 0x39, 0x10 },
      { 0x3A, 0x00 },
      { 0x3B, 0x04 },
      { 0x3C, 0x00 },
      { 0x3D, 0x07 },
      { 0x3E, 0x20 },
      { 0x3F, 0x00 },
      { 0x40, 0x00 },
      { 0x41, 0x00 },
      { 0x42, 0x00 },
      { 0x43, 0x02 },
      { 0x44, 0x00 },
      { 0x45, 0x10 },
      { 0x46, 0x40 },
      { 0x47, 0x00 },
      { 0x48, 0x00 },
      { 0x49, 0x00 },
      { 0x5A, 0x00 },
      { 0x5B, 0x00 },
      { 0x95, 0x00 },
      { 0x96, 0x00 },
      { 0x97, 0x00 },
      { 0x98, 0x00 },
      { 0x99, 0x00 },
      { 0x9A, 0x00 },
      { 0x9B, 0x00 },
      { 0xA2, 0x00 },
      { 0xA3, 0x00 },
      { 0xA4, 0x00 },
      { 0xB7, 0x12 },
      { 0x75, 0xAC },
      { 0x03, 0xF0 },		// Enable Outputs
      { 0x09, 0xF0 }		// NOEB Pin Control Enable

  };

  typedef struct
  {
		unsigned char address;	/* 8-bit register address */
		unsigned char value;	/* 8-bit register data */
  } ak4558_register_t;

  ak4558_register_t const ak4558_registers[AK4558_NUM_REGS] =
  {
      { 0x03, 0x18 },		// DACs Enables / ADCs Disabled
	  { 0x04, 0x00 },		// Not Required
	  { 0x08, 0xD7 },		// -12.0 dB with HP Amp
	  { 0x09, 0xD7 },		// -12.0 dB with HP Amp
      { 0x00, 0x07 }
  };

  // These need to be replaced with read > modify > write
  ak4558_register_t const ak4558_soft_mute_enable = {0x03, 0x19};
  ak4558_register_t const ak4558_soft_mute_disable = {0x03, 0x18};

  uint8_t LED = 0;
  q15_t I2S_TX_Buffer[AUDIO_BUFFER_SIZE] = {0};
  q15_t *I2S_TX_Buffer_p = (q15_t *) I2S_TX_Buffer;

//[[Declarations]]
//[[/Declarations]]

  //[[Instances]]
  //[[/Instances]]

  uint8_t state = 0;
  uint8_t channel = 0;
  uint8_t UART_RX_Buffer[UART_BUFFER_SIZE] = {0};

  int32_t DFSDM_Buffer_TopLeft[DFSDM_BUFFER_SIZE] = {0};
  int32_t DFSDM_Buffer_TopRight[DFSDM_BUFFER_SIZE] = {0};
  int32_t DFSDM_Buffer_BottomLeft[DFSDM_BUFFER_SIZE] = {0};
  int32_t DFSDM_Buffer_BottomRight[DFSDM_BUFFER_SIZE] = {0};

  q15_t MEMS_TopLeft = 0;			// M4
  q15_t MEMS_TopRight = 0;			// M3
  q15_t MEMS_BottomLeft = 0;		// M1
  q15_t MEMS_BottomRight = 0;		// M2

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void Error_Handler(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

void osc()
{
//[[OSC:Processing]]
//[[OSC:Processing]]
}

void serialOut()
{
//[[SerialOut:Processing]]
//[[SerialOut:Processing]]

}

/* USER CODE END 0 */

int main(void)
{

  /* USER CODE BEGIN 1 */
//[[Initialization]]
//[[/Initialization]]
  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_I2S2_Init();
  MX_I2S3_Init();
  MX_I2C1_Init();
  MX_UART5_Init();
  MX_DFSDM1_Init();
  MX_SAI2_Init();
  MX_QUADSPI_Init();
  MX_FMC_Init();
  MX_TIM3_Init();

  /* USER CODE BEGIN 2 */
  HAL_StatusTypeDef status_i2c;

  for (uint16_t i = 0; i < SI5351C_NUM_REGS; i++)
  {
    status_i2c = HAL_I2C_Master_Transmit(&hi2c1, SI5351C_ADDRESS_W, (uint8_t *) &si5351c_revb_registers[i], 2, 25);
    if (status_i2c == HAL_ERROR)
    {
      while(1)
	  {
		HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin);
		HAL_Delay(50);
	  }
    }
  }

  // Enable Clock Output (Active Low)
  HAL_GPIO_WritePin(CLK_NOEB_GPIO_Port, CLK_NOEB_Pin, GPIO_PIN_RESET);
  HAL_Delay(25);

  // Enable Audio Codec (Active High)
  HAL_GPIO_WritePin(AK4558_PDN_GPIO_Port, AK4558_PDN_Pin, GPIO_PIN_SET);
  HAL_Delay(25);

  for (uint16_t i = 0; i < AK4558_NUM_REGS; i++)
  {
    status_i2c = HAL_I2C_Master_Transmit(&hi2c1, AK4558_ADDRESS_W, (uint8_t *) &ak4558_registers[i], 2, 25);
    if (status_i2c == HAL_ERROR)
    {
      while(1)
	  {
		HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin);
		HAL_Delay(50);
	  }
    }
  }

  // Enable Soft Mute
  status_i2c = HAL_I2C_Master_Transmit(&hi2c1, AK4558_ADDRESS_W, (uint8_t *) &ak4558_soft_mute_enable, 2, 25);
  if (status_i2c == HAL_ERROR)
  {
	  while(1)
	  {
		HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin);
		HAL_Delay(50);
	  }
  }

  // Start DFSDM DMA (MEMS Microphones)
  if (HAL_DFSDM_FilterRegularStart_DMA(&hdfsdm1_filter3, DFSDM_Buffer_TopLeft, DFSDM_BUFFER_SIZE) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_DFSDM_FilterRegularStart_DMA(&hdfsdm1_filter2, DFSDM_Buffer_TopRight, DFSDM_BUFFER_SIZE) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_DFSDM_FilterRegularStart_DMA(&hdfsdm1_filter1, DFSDM_Buffer_BottomRight, DFSDM_BUFFER_SIZE) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_DFSDM_FilterRegularStart_DMA(&hdfsdm1_filter0, DFSDM_Buffer_BottomLeft, DFSDM_BUFFER_SIZE) != HAL_OK)
  {
    Error_Handler();
  }

  // Start Audio DMA
  if (HAL_I2S_Transmit_DMA(&hi2s2, (uint16_t *) I2S_TX_Buffer, AUDIO_BUFFER_SIZE) != HAL_OK)
  {
    Error_Handler();
  }

  HAL_Delay(50);

  // Enable HP Amp
  HAL_GPIO_WritePin(TPA4411_PDN_GPIO_Port, TPA4411_PDN_Pin, GPIO_PIN_SET);
  // Enable Speakers
  HAL_GPIO_WritePin(TPA2012_PDN_GPIO_Port, TPA2012_PDN_Pin, GPIO_PIN_RESET);
  HAL_Delay(25);

  // Disable Soft Mute
  status_i2c = HAL_I2C_Master_Transmit(&hi2c1, AK4558_ADDRESS_W, (uint8_t *) &ak4558_soft_mute_disable, 2, 25);
  if (status_i2c == HAL_ERROR)
  {
	  while(1)
	  {
		HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin);
		HAL_Delay(50);
	  }
  }

  if (HAL_UART_Receive_DMA(&huart5, (uint8_t *) UART_RX_Buffer, UART_BUFFER_SIZE) != HAL_OK)
  {
      Error_Handler();
  }

  if (HAL_TIM_Base_Start_IT(&htim3) !=HAL_OK)
  {
      Error_Handler();
  }
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */
  //[[Processing]]
  //[[/Processing]]
      switch ( LED ) {
            case 0:
                HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin);
                break;
            case 1:
                // HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
                break;
            default:
                break;
        }

        LED += 1;
        LED = LED % 2;
        HAL_Delay(150);
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

    /**Configure the main internal regulator output voltage
    */
  __HAL_RCC_PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /**Initializes the CPU, AHB and APB busses clocks
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 432;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

    /**Activate the Over-Drive mode
    */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }

    /**Initializes the CPU, AHB and APB busses clocks
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK)
  {
    Error_Handler();
  }

  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_DFSDM1_AUDIO|RCC_PERIPHCLK_DFSDM1
                              |RCC_PERIPHCLK_UART5|RCC_PERIPHCLK_SAI1
                              |RCC_PERIPHCLK_SAI2|RCC_PERIPHCLK_I2C1
                              |RCC_PERIPHCLK_I2S;
  PeriphClkInitStruct.I2sClockSelection = RCC_I2SCLKSOURCE_EXT;
  PeriphClkInitStruct.Sai1ClockSelection = RCC_SAI1CLKSOURCE_PIN;
  PeriphClkInitStruct.Sai2ClockSelection = RCC_SAI2CLKSOURCE_PIN;
  PeriphClkInitStruct.Uart5ClockSelection = RCC_UART5CLKSOURCE_PCLK1;
  PeriphClkInitStruct.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
  PeriphClkInitStruct.Dfsdm1AudioClockSelection = RCC_DFSDM1AUDIOCLKSOURCE_SAI1;
  PeriphClkInitStruct.Dfsdm1ClockSelection = RCC_DFSDM1CLKSOURCE_SYSCLK;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_HSE, RCC_MCODIV_1);

    /**Configure the Systick interrupt time
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* USER CODE BEGIN 4 */

//[[AudioProcessing]]
//[[/AudioProcessing]]
// void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef *hi2s)
// {
// 	for (uint8_t i = 0; i < (AUDIO_BUFFER_SIZE / 4); i++)
// 	{
// Processing code here
// 	}
// }

// void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s)
// {
// 	HAL_I2S_TxHalfCpltCallback(hi2s);
// }

void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart)
{

	static float32_t temp = 0.0f;

	if (state == 0)
	{

		channel = UART_RX_Buffer[0];
		memcpy((uint8_t*) &temp, (uint8_t*) &UART_RX_Buffer[1],4);

		state = 1;
	}
	else
	{
		channel = UART_RX_Buffer[5];
		memcpy((uint8_t*) &temp, (uint8_t*) &UART_RX_Buffer[6],4);

		state = 0;
	}

    switch(channel) {
//[[SerialIn:Processing]]
		case 0:
//			if (temp > 1 || temp < 0) temp = 0.5;
//			Gain = temp;
			break;
		case 1:
//			PhaseInc1 = temp;
			break;
		case 2:
//			PhaseInc2 = temp;
			break;
		default:
			break;
//[[/SerialIn:Processing]]
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	HAL_UART_RxHalfCpltCallback(huart);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port,LED_GREEN_Pin);
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler */
  /* User can add his own implementation to report the HAL error return state */
  while(1)
  {
	HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin);
	HAL_Delay(100);
  }
  /* USER CODE END Error_Handler */
}

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
