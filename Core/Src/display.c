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
#include "py32f0xx_hal.h"

/* port masks use BSRR lower 16-bits (set mask) to drive cathodes inactive (high) */
#define PORTA_CATHODE_MASK	0x00000700
#define PORTB_CATHODE_MASK	0x00000017
#define PORTF_CATHODE_MASK	0x00000004

/* port masks use BSRR upper 16-bits (reset mask) to drive anodes inactive (low) */
#define PORTA_ANODE_MASK	0x98000000
#define PORTB_ANODE_MASK	0x01e80000

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
