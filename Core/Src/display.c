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
#define NUM_ELEMENTS				(0)

extern volatile uint32_t LED_display_buffer[7][2];

/* port masks use BSRR lower 16-bits (set mask) to drive cathodes inactive (high) */
#define PORTA_CATHODE_MASK	0x00000700
#define PORTB_CATHODE_MASK	0x00000017
#define PORTF_CATHODE_MASK	0x00000004

/* port masks use BSRR upper 16-bits (reset mask) to drive anodes inactive (low) */
#define PORTA_ANODE_MASK	0x98000000
#define PORTB_ANODE_MASK	0x01e80000

/* 7-segment display look-up table for converting digits 0-9 (plus hex a-f) to individual segments */
const uint8_t SSD_LUT[] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x67, 0x77, 0x7c, 0x58, 0x5e, 0x79, 0x71};

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
	if(0 != value)
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
	for(uint8_t i = 0; i < led_group_array[group][NUM_ELEMENTS]; i++)
	{
		display_led(led_group_array[group][i + 1], (value >> i) & 0x01);
	}	
}

void display_group_toggle(uint8_t group)
{
	for(uint8_t i = 0; i < led_group_array[group][NUM_ELEMENTS]; i++)
	{
		display_led_toggle(led_group_array[group][i + 1]);
	}
}

void display_seven_segment(uint8_t digit, uint8_t value)
{
	if(digit_top_left == digit | digit_top_right == digit | digit_bot_left == digit | digit_bot_right == digit)
	{
		if(CLEAR_DISPLAY == value)
			display_group(digit, 0);
		else display_group(digit, SSD_LUT[value]);
	}
	else if(one_top == digit | one_bot == digit)
	{
		if(CLEAR_DISPLAY == value)
			display_group(digit, 0);
		else if(value > 0)
			display_group(digit, 0x03);
		else display_group(digit, 0);
	}	
}

void display_decimal(uint8_t display, uint16_t value)
{
	uint16_t tempVal, i = 0;
	if(display_top == display | display_bottom == display)
	{	
		if(value > 199)
				value = 199;
		
		if(0 == value)
		{
			display_seven_segment(seven_segment_displays[display][0], value);
			display_seven_segment(seven_segment_displays[display][1], CLEAR_DISPLAY);
			display_seven_segment(seven_segment_displays[display][2], CLEAR_DISPLAY);
		}
		else
		{
			while(0 != value)
			{
				tempVal = value%10;
				display_seven_segment(seven_segment_displays[display][i], tempVal);
				value = value/10;
				i++;
				if(0 == value)
				{
					while(i < NUM_DIGITS)
					{
						display_seven_segment(seven_segment_displays[display][i], CLEAR_DISPLAY);
						i++;
					}
				}
			}			
		}		
	}	
}
