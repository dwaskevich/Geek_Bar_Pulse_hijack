/*
 * exti.c
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
#include "gpio_defines.h"
#include "slide_switch.h"

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	/* still in interrupt context ... dispatch callbacks to handle gpio-specific tasks */
	switch(GPIO_Pin)
	{
		case SLIDE_SWITCH_PIN:
			slide_switch_exti_callback();
			break;

		case CHARGE_PIN:
//			charging_exti_callback();
			break;
		
		default:
  		break;
	}
	
}