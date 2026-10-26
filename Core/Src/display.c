/*
 * display.c
 *
 *  Created:	24-Sept-2026
 *  Author:		David Waskevich
 *
 *  Description: 
 *
 *	Update 24-Sept-2026:
 *			- created file
 *  
 */

#include "display.h"
#include "leds.h"
#include "py32f0xx_hal.h"

#define BSRR_RESET_SHIFT		(16)

extern volatile uint32_t LED_display_buffer[7][2];

/* port masks use BSRR lower 16-bits (set mask) to drive cathodes inactive (high) */
#define PORTA_CATHODE_MASK	0x00000700
#define PORTB_CATHODE_MASK	0x00000017
#define PORTF_CATHODE_MASK	0x00000004

/* port masks use BSRR upper 16-bits (reset mask) to drive anodes inactive (low) */
#define PORTA_ANODE_MASK	0x98000000
#define PORTB_ANODE_MASK	0x01e80000

//const uint32_t arc_top_array[] = {arc_top_1, arc_top_2, arc_top_3, arc_top_4, arc_top_5};

void display_cathodes_off(void)
{
	GPIOA->BSRR = PORTA_CATHODE_MASK;
	GPIOB->BSRR = PORTB_CATHODE_MASK;
	GPIOF->BSRR = PORTF_CATHODE_MASK;	
}

void display_anodes_off(void)
{
	GPIOA->BSRR = PORTA_ANODE_MASK;
	GPIOB->BSRR = PORTB_ANODE_MASK;
}

void display_clear(void)
{
	/* clear display buffer */
	for(uint8_t i = 0; i < NUM_LED_CATHODES; i++)
	{
		LED_display_buffer[i][0] = 0;
		LED_display_buffer[i][1] = 0;
	}
}

void display_all(void)
{
	for(uint8_t i = 0; i < MAX_LEDS; i++)
	{
		display_led(i, 1);
	}	
}

void display_led(uint16_t led, uint8_t value)
{
	if(1 == value)
	{
		/* add display element bit to BSRR low (set) word */
		LED_display_buffer[LED_port_position[led].scan_line][0] |= LED_port_position[led].set_segment_GPIOA;
		LED_display_buffer[LED_port_position[led].scan_line][1] |= LED_port_position[led].set_segment_GPIOB;		
	}
	else
	{
		/* remove display element bit from BSRR low (set) word */
		LED_display_buffer[LED_port_position[led].scan_line][0] &= ~LED_port_position[led].set_segment_GPIOA;
		LED_display_buffer[LED_port_position[led].scan_line][1] &= ~LED_port_position[led].set_segment_GPIOB;
		
		/* add display element bit to BSRR high (reset) word */
		LED_display_buffer[LED_port_position[led].scan_line][0] |= LED_port_position[led].set_segment_GPIOA << BSRR_RESET_SHIFT;
		LED_display_buffer[LED_port_position[led].scan_line][1] |= LED_port_position[led].set_segment_GPIOB << BSRR_RESET_SHIFT;
	}
}

void display_led_toggle(uint16_t led)
{
	/* invert display element bit in BSRR low (set) word */
	LED_display_buffer[LED_port_position[led].scan_line][0] ^= LED_port_position[led].set_segment_GPIOA;
	LED_display_buffer[LED_port_position[led].scan_line][1] ^= LED_port_position[led].set_segment_GPIOB;
	
	/* invert display element bit in BSRR high (reset) word */
	LED_display_buffer[LED_port_position[led].scan_line][0] ^= LED_port_position[led].set_segment_GPIOA << BSRR_RESET_SHIFT;
	LED_display_buffer[LED_port_position[led].scan_line][1] ^= LED_port_position[led].set_segment_GPIOB << BSRR_RESET_SHIFT;
}

void display_group(uint8_t group, uint8_t value)
{
	for(uint8_t i = 0; i < led_group_array[group][0]; i++)
	{
		display_led(led_group_array[group][i + 1], value);
	}	
}

void display_group_toggle(uint8_t group)
{
	for(uint8_t i = 0; i < led_group_array[group][0]; i++)
	{
		display_led_toggle(led_group_array[group][i + 1]);
	}
}