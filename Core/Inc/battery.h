/*
 * battery.h
 *
 *  Created:	29-Oct-2026
 *  Author:		David Waskevich
 *
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BATTERY_H
#define __BATTERY_H

#include <stdint.h>

typedef struct
{
    uint16_t millivolts;
    uint8_t percent;
    uint8_t bars;
} battery_status_t;

uint16_t battery_measure_voltage(void);
uint16_t battery_filter_ADC_value(uint16_t new_adc_raw);
//uint8_t get_battery_status(uint16_t current_mv);
battery_status_t battery_get_status(void);

#endif