/**
  ******************************************************************************
  * @file    main.c
  * @author  MCU Application Team
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2023 Puya Semiconductor Co.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by Puya under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2016 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private define ------------------------------------------------------------*/
/* djjw */
#define RED_PIN				GPIO_PIN_5
#define BLUE_PIN			GPIO_PIN_2
#define GREEN_PIN			GPIO_PIN_4

#define SWITCH_PORT		GPIOA
#define SWITCH_PIN		GPIO_PIN_7

#define CHARGE_PORT		GPIOF
#define CHARGE_PIN		GPIO_PIN_1

#define ANIMATION_DELAY		(333)


/* Private variables ---------------------------------------------------------*/
/* Handle for the TIM1 Timer ... will be used for display muxing */
TIM_HandleTypeDef		htim1 = {0};

/* Private user code ---------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void APP_SystemClockConfig(void);
static void GPIO_Init(void);
static void TIM1_Init(void);
void Port_WriteLowerByte(GPIO_TypeDef *GPIOx, uint8_t value);
void Port_WriteUpperByte(GPIO_TypeDef *GPIOx, uint8_t value);

/**
  * @brief  Main program.
  * @retval int
  */
int main(void)
{	
  /* Reset of all peripherals, Initializes the Systick. */
  HAL_Init();
  
  /* Configure the system clock */
  APP_SystemClockConfig();
	
	HAL_Delay(1000);
	
	/* djjw - initialize pins */
	GPIO_Init();
	
	/* djjw - initialize TIM1 timer */
	TIM1_Init();
	
	/* Start the TIM1 Base generation in interrupt mode */
  if (HAL_TIM_Base_Start_IT(&htim1) != HAL_OK)
  {
    APP_ErrorHandler();
  }
	
	/* turn on all LEDs */
	Port_WriteLowerByte(GPIOB, 0xe8);
	Port_WriteUpperByte(GPIOB, 0x01);
	Port_WriteUpperByte(GPIOA, 0x98);
	Port_WriteLowerByte(GPIOF, 0x04);
	
	HAL_Delay(1000);
	
	/* turn off all LEDs ... turn off anodes (bit value = 1), leave all cathodes on (bit value = 0) */
	Port_WriteLowerByte(GPIOB, 0x00);
	Port_WriteUpperByte(GPIOB, 0x00);
	Port_WriteUpperByte(GPIOA, 0x00);
	Port_WriteLowerByte(GPIOF, 0x00);

  while (1)
  {
//		HAL_GPIO_WritePin(GPIOA, RED_PIN, HAL_GPIO_ReadPin(GPIOA, SWITCH_PIN));
//		HAL_GPIO_WritePin(GPIOA, BLUE_PIN, HAL_GPIO_ReadPin(CHARGE_PORT, CHARGE_PIN));
//		HAL_GPIO_WritePin(GPIOA, BLUE_PIN, 1);
//		HAL_Delay(50);
//		HAL_GPIO_WritePin(GPIOA, BLUE_PIN, 0);
		for(uint8_t i = 5; i < 8; i++)
		{
			Port_WriteLowerByte(GPIOB, 1 << i);
			HAL_Delay(500);
		}
		Port_WriteLowerByte(GPIOB, 0x00);
	}
}

/**
  * @brief  System Clock Configuration
  * @param  None
  * @retval None
  */
static void APP_SystemClockConfig(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /* Oscillator Configuration */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_LSE; /* Select oscillators HSE, HSI, LSI, LSE */
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;                          /* Enable HSI */
#if defined(RCC_HSIDIV_SUPPORT)
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;                          /* HSI not divided */
#endif
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_24MHz;  /* Configure HSI clock as 8MHz ... djjw changed to 24MHz */
  RCC_OscInitStruct.HSEState = RCC_HSE_OFF;                         /* Disable HSE */
  /*RCC_OscInitStruct.HSEFreq = RCC_HSE_16_32MHz;*/
  RCC_OscInitStruct.LSIState = RCC_LSI_OFF;                         /* Disable LSI */
  RCC_OscInitStruct.LSEState = RCC_LSE_OFF;                         /* Disable LSE */
  /*RCC_OscInitStruct.LSEDriver = RCC_LSEDRIVE_MEDIUM;*/
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_OFF;                     /* Disable PLL */
  /*RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;*/          /* Select HSI as PLL source */
  /* Configure oscillators */
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    APP_ErrorHandler();
  }

  /* Clock source configuration */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1; /* Select clock types HCLK, SYSCLK, PCLK1 */
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSISYS; /* Select HSISYS as the system clock */
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;     /* AHB clock not divide */
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;      /* APB clock not divided */
  /* Configure clock source */
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    APP_ErrorHandler();
  }	
}

/**
  * @brief  djjw ... GPIO Config
  * @param  None
  * @retval None
  */
static void GPIO_Init(void)
{	
	/* enable GPIO port clock(s) */
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOF_CLK_ENABLE();
	
	/* declare an init structure */
	GPIO_InitTypeDef GPIO_InitStruct = {0};	
	
	GPIO_InitStruct.Pin = RED_PIN | BLUE_PIN | GREEN_PIN | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_15;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin = SWITCH_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	
	HAL_GPIO_Init(SWITCH_PORT, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin = GPIO_PIN_1;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	
	HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin = GPIO_PIN_2;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	
	HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
	
}

/**
  * @brief  djjw ... TIM1 Config
  * @param  None
  * @retval None
  */
static void TIM1_Init(void)
{	
	htim1.Instance               = TIM1;                             /* Select TIM1 */
  htim1.Init.Period            = 10000 - 1;                          /* Auto-reload value */
  htim1.Init.Prescaler         = 800 - 1;                         /* Prescaler of 1000-1 */
  htim1.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;           /* No clock division */
  htim1.Init.CounterMode       = TIM_COUNTERMODE_UP;               /* Up counting */
  htim1.Init.RepetitionCounter = 1 - 1;                            /* No repetition counting */
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;   /* Auto-reload register not buffered */
	
  /* Initialize TIM1 */
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    APP_ErrorHandler();
  }
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @param  htim:TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	(void) htim;
  HAL_GPIO_TogglePin(GPIOA, BLUE_PIN);
}

void Port_WriteLowerByte(GPIO_TypeDef *GPIOx, uint8_t value)
{
    uint32_t bsrr_value;

    // High 16 bits of BSRR reset pins (1 clears pin)
    // Low 16 bits of BSRR set pins (1 sets pin)
    // (~value & 0xFF) finds zeros in the byte to turn them into resets
    bsrr_value = ((uint32_t)(~value & 0xFF) << 16) | (value & 0xFF);
    
    GPIOx->BSRR = bsrr_value;
}

void Port_WriteUpperByte(GPIO_TypeDef *GPIOx, uint8_t value)
{
    uint32_t bsrr_value;

    // Shift the 8-bit 'set' values to bits [15:8]
    uint32_t set_mask = ((uint32_t)value) << 8;
    
    // Shift the inverted 8-bit 'reset' values to bits [31:24]
    uint32_t rst_mask = ((uint32_t)(~value & 0xFF)) << 24;
    
    // Combine and execute in a single atomic cycle
    bsrr_value = rst_mask | set_mask;
    GPIOx->BSRR = bsrr_value;
}



/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void APP_ErrorHandler(void)
{
  /* Infinite loop */
  while (1)
  {
  }
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     for example: printf("Wrong parameters value: file %s on line %d\r\n", file, line)  */
  /* Infinite loop */
  while (1)
  {
  }
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT Puya *****END OF FILE******************/
