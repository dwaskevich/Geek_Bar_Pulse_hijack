/*
 * adc.h
 *
 *  Created:	11-Nov-2026
 *  Author:		David Waskevich
 *
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ADC_H
#define __ADC_H

#include <stdint.h>

void adc_init(void);
uint32_t adc_convert(uint32_t channel);

#endif