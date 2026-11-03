/*
 * ui.h
 *
 *  Created:	2-Nov-2026
 *  Author:		David Waskevich
 *
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __UI_H
#define __UI_H

typedef enum
{
	fast = 110,
	medium = 220,
	slow = 440
} rotate_cycle_speed_t;

void heartbeat(void);
void rotate_cycle_cw(void);
void rotate_cycle_ccw(void);


#endif