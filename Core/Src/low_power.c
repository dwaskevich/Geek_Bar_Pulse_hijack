/*
 * low_power.c
 *
 *  Created:	12-Nov-2026
 *  Author:		David Waskevich
 *
 *  Description: 
 *
 *	Update 12-Nov-2026:
 *			- created file
 *  
 */

#include "main.h"
#include "low_power.h"
#include "display.h"
#include "leds.h"
#include "scheduler.h"
#include "slide_switch.h"

#define ANIMATION_DELAY			(250)

/**
  * @brief  Low power stop mode
  * @param  _Bool value to enable or disable low-power mode
  * @retval None
	*
	*	djjw ... prepares system for and enters low power stop.
	*
	*	Note ... all tasks are suspended, calling function must reschedule.
	*
  */
void enter_low_power(_Bool enable)
{	
	/* UI animation */
	HAL_TIM_PWM_Start(&htim3, RED_CHANNEL);
	HAL_TIM_PWM_Start(&htim3, GREEN_CHANNEL);
	HAL_TIM_PWM_Start(&htim3, BLUE_CHANNEL);
	
	__HAL_TIM_SET_COMPARE(&htim3, RED_CHANNEL, 1023);
	HAL_Delay(ANIMATION_DELAY);
	__HAL_TIM_SET_COMPARE(&htim3, BLUE_CHANNEL, 1023);
	HAL_Delay(ANIMATION_DELAY);
	__HAL_TIM_SET_COMPARE(&htim3, GREEN_CHANNEL, 1023);
	HAL_Delay(ANIMATION_DELAY);
	__HAL_TIM_SET_COMPARE(&htim3, RED_CHANNEL, 0);
	HAL_Delay(ANIMATION_DELAY);
	__HAL_TIM_SET_COMPARE(&htim3, BLUE_CHANNEL, 0);
	HAL_Delay(ANIMATION_DELAY);
	__HAL_TIM_SET_COMPARE(&htim3, GREEN_CHANNEL, 0);
		
	if(1 == enable)
	{
		scheduler_suspend_all_tasks();
		slide_switch_config(CONFIG_INTERRUPT); /* reconfigure slide switch input for wakeup interrupt */
		
		/* fill display then fade out */
		display_all();
		for(uint8_t i = slice_1; i <= slice_23; i++)
		{
			display_group(i, 0);
			__HAL_TIM_SET_COMPARE(&htim3, GREEN_CHANNEL, 700 - (30 * (i - slice_1)));
			HAL_Delay(100);			
		}
		__HAL_TIM_SET_COMPARE(&htim3, GREEN_CHANNEL, 0);

		HAL_TIM_PWM_Stop(&htim3, RED_CHANNEL);
		HAL_TIM_PWM_Stop(&htim3, GREEN_CHANNEL);
		HAL_TIM_PWM_Stop(&htim3, BLUE_CHANNEL);
		__HAL_RCC_TIM3_CLK_DISABLE();
		
		display_cathodes_off(); /* turn off all display cathodes */
			
		/* put GPIO in low-power state (analog is best choice to avoid leakage current) */
		GPIO_InitTypeDef GPIO_InitStruct = {0};	
		
		/* GPIOA */
		GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_15;
		GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
		
		/* GPIOB */
		GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8;
		GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
		
		/* stop TIM16 timer and suspend SysTick */
		HAL_TIM_Base_Stop_IT(&htim16);
		HAL_SuspendTick();
		
		/* Enter STOP mode with interrupt wakeup */
		HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
		
		/* exit from stop ... reconfigure clocks and gpio, restart timer and SysTick */
		APP_SystemClockConfig();
		
		slide_switch_config(CONFIG_ADC); /* restore slide switch function to ADC input */
		
		/* return GPIO Port A LED pins to outputs */
		GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_15;
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
		
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
		
		/* return GPIO Port B LED pins to outputs */
		GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8;
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
		
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
		
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
		
		HAL_ResumeTick();
		HAL_TIM_Base_Start_IT(&htim16);
	}
}