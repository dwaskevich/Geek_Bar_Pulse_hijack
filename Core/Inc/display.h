/*
 * display.h
 *
 *  Created:	24-Sept-2026
 *  Author:		David Waskevich
 *
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DISPLAY_H
#define __DISPLAY_H

#include <stdint.h>

/* enum for passing top/bottom display parameters */
enum 
{
	TOP = 0,
	BOTTOM
};


void display_cathodes_off(void);
void display_anodes_off(void);
void display_clear(void);
void display_all(void);
void display_led(uint16_t led, uint8_t value);
void display_led_toggle(uint16_t led);
void display_group(uint8_t group, uint8_t value);
void display_group_toggle(uint8_t group);

#endif