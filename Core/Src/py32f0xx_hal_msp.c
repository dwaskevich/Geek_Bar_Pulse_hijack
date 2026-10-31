/**
  ******************************************************************************
  * @file    py32f0xx_hal_msp.c
  * @author  MCU Application Team
  * @brief   This file provides code for the MSP Initialization
  *          and de-Initialization codes.
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

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* External functions --------------------------------------------------------*/

/**
  * @brief Initialize global MSP
  */
void HAL_MspInit(void)
{
  __HAL_RCC_SYSCFG_CLK_ENABLE();
  __HAL_RCC_PWR_CLK_ENABLE();
}

/**
  * @brief Initialize TIM-related MSP
	*
	*	djjw ... copied from PY32F0xx Firmware Master SDK download (Timer code example)
	*
  */
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
	(void) htim;
	
	/* Enable TIM16 clock */
	__HAL_RCC_TIM16_CLK_ENABLE();
	
	/* Set interrupt priority */
	HAL_NVIC_SetPriority(TIM16_IRQn, 0, 0);
	/* Enable TIM1 interrupt */
	HAL_NVIC_EnableIRQ(TIM16_IRQn);
}

/**
  * @brief Initialize TIM-related MSP
	*
	*	djjw ... copied from PY32F0xx Firmware Master SDK download (TIM1 PWM code example)
	*
  */
void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef *htim)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	
	(void) htim;
	
	/* Enable TIM3 clock */
	__HAL_RCC_TIM3_CLK_ENABLE();
 
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;        /* Alternate function mode */
	GPIO_InitStruct.Pull = GPIO_NOPULL;            /* No Pull-up */
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	
	/*Initialize GPIO PA2 (Channel 1) - Blue */
	GPIO_InitStruct.Pin = BLUE_PIN;
	GPIO_InitStruct.Alternate = GPIO_AF13_TIM3;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	/*Initialize GPIO PA4 (Channel 3) - Green */
	GPIO_InitStruct.Pin = GREEN_PIN;
	GPIO_InitStruct.Alternate = GPIO_AF13_TIM3;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	/*Initialize GPIO PA5 (Channel 2) - Red */
	GPIO_InitStruct.Pin = RED_PIN;
	GPIO_InitStruct.Alternate = GPIO_AF13_TIM3;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	
}

/************************ (C) COPYRIGHT Puya *****END OF FILE******************/
