/*
 * battery.c
 *
 *  Created:	29-Oct-2026
 *  Author:		David Waskevich
 *
 *  Description: Battery measurement utilities
 *
 *	Update 29-Oct-2026:
 *			- created file
 *  
 */

#include "battery.h"
#include "main.h"

const uint16_t Battery_Percent_To_mV[101] = {
    3400, 3410, 3420, 3430, 3440, 3450, 3460, 3470, 3480, 3490, // 0% - 9%
    3500, 3505, 3510, 3514, 3518, 3522, 3526, 3530, 3534, 3537, // 10% - 19%
    3539, 3540, 3541, 3542, 3543, 3543, 3544, 3546, 3549, 3553, // 20% - 29%
    3540, 3548, 3556, 3564, 3572, 3580, 3591, 3603, 3614, 3626, // 30% - 39%
    3637, 3649, 3660, 3672, 3683, 3695, 3701, 3704, 3707, 3710, // 40% - 49%
    3713, 3716, 3719, 3722, 3725, 3728, 3731, 3734, 3737, 3740, // 50% - 59%
    3745, 3751, 3756, 3762, 3767, 3773, 3778, 3784, 3789, 3795, // 60% - 69%
    3700, 3714, 3728, 3742, 3756, 3770, 3784, 3798, 3812, 3826, // 70% - 79%
    3840, 3854, 3868, 3882, 3896, 3910, 3925, 3940, 3955, 3970, // 80% - 89%
    3985, 4000, 4015, 4030, 4045, 4060, 4078, 4096, 4114, 4132, // 90% - 99%
    4150                                                        // 100%
};

uint16_t battery_measure_voltage(void)
{
	uint32_t adc_value;
	uint16_t smooth_vrefint_sample = 0;
	
	/* Start ADC */
	HAL_ADC_Start(&hadc);

	/* wait for ADC conversion to complete */
	HAL_ADC_PollForConversion(&hadc, 1000);

	/* Get ADC value */
	adc_value = HAL_ADC_GetValue(&hadc);
	
	HAL_ADC_Stop(&hadc);
	
	smooth_vrefint_sample = battery_filter_ADC_value(adc_value);
	
	if(smooth_vrefint_sample > 0)
	{
		return (1200 * 4095) / smooth_vrefint_sample;
	}
	
	return 0;
	
};

uint16_t battery_filter_ADC_value(uint16_t new_adc_raw)
{
	static uint32_t filtered_adc_accumulator = 0;
	
	/* seed the filter accumulator on your very first sample to avoid a slow ramp-up from 0 */
	if(filtered_adc_accumulator == 0)
	{
			filtered_adc_accumulator = (uint32_t)new_adc_raw << 4;
			return new_adc_raw;
	}
	
	/* 
		 IIR Filter Equation: 
		 Accumulator = (Accumulator * 15 / 16) + NewSample		 
		 This mimics a physical RC hardware low-pass filter with 1/16 weight.
	*/
	filtered_adc_accumulator = ((filtered_adc_accumulator * 15) + ((uint32_t)new_adc_raw << 4)) >> 4;
	
	/* scale back down from 16.4 fixed-point to return a standard 12-bit value */
	return (uint16_t)(filtered_adc_accumulator >> 4);	
}


battery_status_t battery_get_status(void)
{
	battery_status_t battery_status = {0};
	
	battery_status.millivolts = battery_measure_voltage();
	
	/* use look-up table to find percent of capacity */
	if(battery_status.millivolts >= Battery_Percent_To_mV[100])
		battery_status.percent = 100;
	if(battery_status.millivolts <= Battery_Percent_To_mV[0])
		battery_status.percent = 0;	
	for(uint8_t i = 0; i < 100; i++)
	{
		if(battery_status.millivolts >= Battery_Percent_To_mV[i] && battery_status.millivolts <= Battery_Percent_To_mV[i+1])
		{
			/* return closest matching index boundary */
			battery_status.percent = ((battery_status.millivolts - Battery_Percent_To_mV[i]) < (Battery_Percent_To_mV[i+1] - battery_status.millivolts)) ? i : i + 1;
		}
	}
	
	/* determine fuel gauge */
	if (battery_status.percent >= 99)
			battery_status.bars = 5;
	else if (battery_status.percent >= 80)
			battery_status.bars = 4;
	else if (battery_status.percent >= 50)
			battery_status.bars = 3;
	else if (battery_status.percent >= 20)
			battery_status.bars = 2;
	else if (battery_status.percent >= 2)
			battery_status.bars = 1;
	else
			battery_status.bars = 0;
	
	return battery_status;
}