/*
 * adc.c
 *
 *  Created:	11-Nov-2026
 *  Author:		David Waskevich
 *
 *  Description: 
 *
 *	Update 11-Nov-2026:
 *			- created file
 *  
 */
 
 #include "main.h"
 #include "adc.h"
 
/* Handles for the ADC */
ADC_HandleTypeDef hadc = {0};
ADC_ChannelConfTypeDef sConfig = {0};
 
 /**
  * @brief  ADC configuration function
  * @param  None
  * @retval None
  */
void adc_init(void)
{
  __HAL_RCC_ADC_FORCE_RESET();
  __HAL_RCC_ADC_RELEASE_RESET(); /* Reset ADC */
  __HAL_RCC_ADC_CLK_ENABLE(); /* Enable ADC clock */
  
  hadc.Instance = ADC1;

  /* ADC calibration */
  if (HAL_ADCEx_Calibration_Start(&hadc) != HAL_OK)                 
  {
    APP_ErrorHandler();
  }
  
  hadc.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;              /* Set ADC clock */
  hadc.Init.Resolution = ADC_RESOLUTION_12B;                        /* 12-bit resolution for converted data */
  hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;                        /* Right-alignment for converted data */
  hadc.Init.ScanConvMode = ADC_SCAN_DIRECTION_FORWARD;              /* Scan sequence direction: forward */
  hadc.Init.EOCSelection = ADC_EOC_SINGLE_CONV;                     /* Single sampling */
  hadc.Init.LowPowerAutoWait = ENABLE;                              /* Enable wait for conversion mode*/
  hadc.Init.ContinuousConvMode = DISABLE;                           /* Single conversion mode */
  hadc.Init.DiscontinuousConvMode = DISABLE;                        /* Disable discontinuous mode */
  hadc.Init.ExternalTrigConv = ADC_SOFTWARE_START;                  /* Software triggering */
  hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;   /* No external trigger edge */
  hadc.Init.DMAContinuousRequests = DISABLE;                        /* Disable DMA */
  hadc.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;                     /* When an overload occurs, overwrite the previous value  */
	hadc.Init.SamplingTimeCommon = ADC_SAMPLETIME_239CYCLES_5;         /* Set sampling time */

  /* ADC initialization */
  if (HAL_ADC_Init(&hadc) != HAL_OK)                                
  {
    APP_ErrorHandler();
  }
}

/**
  * @brief  ADC convert function
  * @param  Channel: The channel to be converted 
  * @retval ADC converted value
  */
uint32_t adc_convert(uint32_t channel)
{
  uint32_t adc_value;	
	hadc.Instance = ADC1;
	
	if(ADC_CHANNEL_VREFINT == channel)
	{
		hadc.Init.SamplingTimeCommon = ADC_SAMPLETIME_239CYCLES_5; /* Set sampling time */
	}
	else if(ADC_CHANNEL_7 == channel)
	{
		hadc.Init.SamplingTimeCommon = ADC_SAMPLETIME_41CYCLES_5; /* Set sampling time */
	}
	else
	{
		hadc.Init.SamplingTimeCommon = ADC_SAMPLETIME_239CYCLES_5; /* Set sampling time */
	}
	
	/* ADC initialization */
  if (HAL_ADC_Init(&hadc) != HAL_OK)                                
  {
    APP_ErrorHandler();
  }
  
  /* Clear all channels */
  WRITE_REG(ADC1->CHSELR,0);
  
  /* Config selected channels */
  sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
  sConfig.Channel = channel;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK) /* Configure ADC Channel */
  {
    APP_ErrorHandler();
  }
  
  /* ADC Start */
  HAL_ADC_Start(&hadc);
  
  /* Polling for ADC Conversion */
  HAL_ADC_PollForConversion(&hadc, 1000);
  
  /* Get ADC Value */
  adc_value = HAL_ADC_GetValue(&hadc);
	
	HAL_ADC_Stop(&hadc);
  
  /* Clear all channels */
  WRITE_REG(ADC1->CHSELR,0);
  
  return adc_value;
}
