/*
 * gpio_defines.h
 *
 *  Created:	25-Oct-2026
 *  Author:		David Waskevich
 *
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __GPIO_DEFINES_H
#define __GPIO_DEFINES_H

#define SLIDE_SWITCH_PORT		GPIOA
#define SLIDE_SWITCH_PIN		GPIO_PIN_7
#define CHARGE_PORT					GPIOF
#define CHARGE_PIN					GPIO_PIN_1

#define RED_PIN							GPIO_PIN_5
#define BLUE_PIN						GPIO_PIN_2
#define GREEN_PIN						GPIO_PIN_4

#define RED_CHANNEL					TIM_CHANNEL_2
#define BLUE_CHANNEL				TIM_CHANNEL_1
#define GREEN_CHANNEL				TIM_CHANNEL_3


#endif