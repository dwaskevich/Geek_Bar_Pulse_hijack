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

typedef enum
{
	SWITCH_CLOSED = 0,
	SWITCH_NORMAL,
	SWITCH_PULSE
}slide_switch_position_t;

void slide_switch_init(void);
void slide_switch_config(config_t config_type);
slide_switch_position_t slide_switch_get_position(void);
void slide_switch_exti_callback(void);

#endif