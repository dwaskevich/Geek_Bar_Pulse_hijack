/*
 * slide_switch.c
 *
 *  Created:	4-Nov-2026
 *  Author:		David Waskevich
 *
 *  Description: 
 *
 *	Update 4-Nov-2026:
 *			- created file
 *  
 */

#include "main.h"
#include "slide_switch.h"
#include "gpio_defines.h"

volatile _Bool slide_switch_event;

void slide_switch_init(void)
{
	/* configure GPIO */
	
	/* enable GPIO port clock(s) */
	__HAL_RCC_GPIOA_CLK_ENABLE();
	
	/* declare an init structure */
	GPIO_InitTypeDef GPIO_InitStruct = {0};	
	
	/* initialize GPIO Port A pin 7 as analog input for ADC */
	GPIO_InitStruct.Pin = SLIDE_SWITCH_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	
	HAL_GPIO_Init(SLIDE_SWITCH_PORT, &GPIO_InitStruct);
}

void slide_switch_config(config_t config_type)
{
	/* declare an init structure */
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	
	/* enable GPIO port clock(s) */
	__HAL_RCC_GPIOA_CLK_ENABLE();
	
	if(CONFIG_ADC == config_type)
	{
		/* configure GPIO Port A pin 7 as analog input for ADC */
		GPIO_InitStruct.Pin = SLIDE_SWITCH_PIN;
		GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
		
		HAL_GPIO_Init(SLIDE_SWITCH_PORT, &GPIO_InitStruct);		
	}
	else if(CONFIG_INTERRUPT == config_type)
	{		
		/* configure GPIO Port A pin 7 as rising/falling edge interrupt source */
		GPIO_InitStruct.Pin = SLIDE_SWITCH_PIN;
		GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
		
		HAL_GPIO_Init(SLIDE_SWITCH_PORT, &GPIO_InitStruct);
		
		/* Enable EXTI interrupt and configure interrupt priority */
		HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
		HAL_NVIC_SetPriority(EXTI4_15_IRQn, 0, 0);
	}
	else return;	
}

void slide_switch_exti_callback(void)
{
	/* still in interrupt context ... just set a flag and process in main loop with scheduler task */
	slide_switch_event = TRUE;
}