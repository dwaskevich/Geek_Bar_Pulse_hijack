/*
 * slide_switch.h
 *
 *  Created:	4-Nov-2026
 *  Author:		David Waskevich
 *
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SLIDE_SWITCH_H
#define __SLIDE_SWITCH_H

typedef enum
{
	CONFIG_ADC,
	CONFIG_INTERRUPT	
}config_t;

void slide_switch_init(void);
void slide_switch_config(config_t config_type);
void slide_switch_exti_callback(void);

#endif