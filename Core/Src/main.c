/**
  ******************************************************************************
  * @file    main.c
  * @author  MCU Application Team
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * Date:    24-Sept-2026
  * Author:  David Waskevich
  *
  * Description: Geek Bar Pulse hijack
	*
  * Geek Bar Pulse reverse engineered documentation:
  *    https://github.com/schlae/VapeRE/tree/main/BD0027
  *              
  *
  * Hardware:    Puya PY32f030EK28 Cortex-M0
  *              -> 64k Flash, 8k SRAM, 32-QFN 5x5 package
	*							 https://www.py32.org/en/mcu/PY32F030xx.html
  *
  * IDE:         Keil MDK-ARM v5.43a uVision 5
  *
  * Wiring:      SWD interface exposed on USB-C charging port (CC1/CC2)
	*							   -> SWDIO - PA13, SWCLK - PA14
  *                -> NOTE -> Normal state of unit is in deep sleep (or stop), SWD interface
  *                    is normally unavailable. Slide switch wakes unit and provides
	*								     small connect window. Can also plug in charger to wake unit.
  *              See schematic for feature element mapping (i.e. RGB LEDs, display, etc)
	*							   -> https://github.com/schlae/VapeRE/tree/main/BD0027
  *              Charging indicator from LP4068 Li-Ion controller - PF1
  *              Charge current programming output - PF0
  *              Slide switch - PA7 (pulls low in one position, high in remaining two)
  *              RGB LED ring (16 tri-color smt LEDs)
  *							  -> Red PA5, Green PA4, Blue PA2
  *							  -> note - each color driven by FET
  *              MOSFET coil drives:
  *							  -> TB1 - PA1, Sense input PA6
  *							  -> TB2 - PA0, Sense input PA3
  *              Microphone (puff sensor) input:
  *							  -> PF3
  *
  * Peripheral setup:
  *              UART - USART1 ... re-purpose SWD via AF registers:
	*									-> Rx = PA13 (AF8), Tx = PA14 (AF1)
  *              LED matrix timer - TIM16 (auto-reload mode, 1.6 ms interrupt)
  *              Low power timer - LPTIM (? interrupt)
  *              RGB PWM:
  *							  - Red   (PA5) TIM3_CH2 (AF13)
  *							  - Green (PA4) TIM3_CH3 (AF13)
  *							  - Blue  (PA2) TIM3_CH1 (AF13)
  *              TB1/TB2 PWM (MOSFET coil drives):
  *							  - TB1 (PA1) TIM1_CH4 (AF13)
  *							  - TB2 (PA0) TIM1_CH3 (AF13)
  *              Program/Debug - SWD ... PA13 (SWDIO)/PA14 (SWDCK)
  *
  *
  * Usage:       #include display.h, leds.h, gpio_defines.h
  *              NOTE - 
  *
  * Update 26-Sept-2026:
  *		- added LED mapping for 53 multiplexed LEDs ... 7 x 8 matrix (leds.h)
  *		- muxing controlled by TIM16 PeriodElapsedCallback
	*		- verified mapping with test animation
  *
  * Update 22-Oct-2026:
  *		- corrected device selection (mistakenly created project with py32f003x8
	*			selected as device for Target_1 instead of py32f030x8)
	*			-> tried to change it in "Options for Target_1" but Puya device script
	*				 has an error that loads py32f031 startup file as well ... which
	*				 causes compile to fail. Had to use RTE to uncheck the automatic
	*				 device startup and add startup_py32F030x8.s and system_py32f0xx.c
	*				 manually to the project's source group.
	*		- created leds.c source file and moved led matrix to the .c file to
	*			   conserve flash storage
  *		- updated leds.h to declare the led matrix as extern
  *
  * Update 23-Oct-2026:
  *		- removed reset_segment from LED_port_position structure, replaced with
	*			  simple left shift of set_segment value (<< BSRR_RESET_SHIFT (16))
	*		- first pass slide switch gpio configuration (exti wakeup source)
  *
  * Update 24-Oct-2026:
  *		- implemented test code to measure low-power stop current
	*			-> 75-79 uA
  *		- created enter_low_power() function
	*		- moved LED mux timer to TIM16 (reserve TIM1 for TB1/TB2 PWM)
	*			-> updated py32f0xx_it.c
	*			-> updated py32f0xx_hal_msp.c
  *
  * Update 25-Oct-2026:
  *		- implemented charging port detection and handling
	*			-> configured charge input (PF1) as falling edge interrupt
	*			-> serves as wakeup source, disables sleep/stop mode
  *		- implemented low-level display functions
  *		- created gpio_defines.h to hold pin configuration info
  *
  * Update 26-Oct-2026:
  *		- implemented high-level 7-segment display functions
	*			-> display_seven_segment() ... wrties to an individual digit
	*			-> display_decimal() ... writes decimal value to upper or lower
	*			    2.1 digit display
	*			-> display_hex() ... writes hex value to upper or lower 2.1
	*			    digit display
  *		- enabled ADC to measure battery voltage
	*			-> updated py32f0xx_hal_conf.h to enable ADC HAL APIs
	*			-> value displayed as millivolts (2 most significant digits
	*			    displayed on upper display, 2 least significant on lower)
  *		- added inactivity timer to enter stop mode after 20 seconds
  *
  * Update 27-Oct-2026:
  *		- added a clear_display function to clear all 2.1 digits of 7-seg displays
  *		- created 23 vertical slices to facilitate screen fade-out
	*			-> added to led_group_array[]
  *
  * Update 28-Oct-2026:
  *		- improved ADC Vrefint measurement:
	*			-> first order IIR low pass filter to smooth reading (1/16 weight)
	*			-> 101-point fixed lookup table/array to convert voltage to percent
	*			-> empirical results on open unit: DMM = 3.89v, firmware = 3.884
  *		- added display_group_fill() to fill a group based on the value passed
  *
  * Update 29-Oct-2026:
  *		- moved battery status to .c/.h files
  *
  * Update 30-Oct-2026:
  *		- set up RGB PWM:
	*			-> moved RGB pin #defines to main.h (for access in py32f00xx_hal_msp.c)
	*			-> using TIM3, Channels 1, 2, 3 ... gpio mapped via AF13
	*			-> TIM3_Init() in main.c
	*				-> includes base timer as well as TIM_OC_InitTypeDef PWM-specific config
	*			-> HAL_TIM_PWM_MspInit() in py32f0xx_hal_msp.c
	*				-> handles TIM3 clock enable and alternate function gpio config
  *		- updated demo application to include PWM effects
  *		- updated readme.md
  *
  * Update 2-Nov-2026:
  *		- implemented task scheduler
  *		- created test tasks in ui.c
  *
  * Update 11-Nov-2026:
  *		- restructured firmware architecture
  *			-> added board-specific initializations (init functions moved to individual source files)
  *
  * Update 12-Nov-2026:
  *		- fixed display_group_fill()
  *		- implemented ADC functions
  *		- implemented low power
  *		- updated slide switch:
  *			-> configured for ADC Channel 7 for normal reads
  *			-> reconfigured for interrupt wakeup source in low power
	*
  * <h2><center>&copy; Copyright (c) 2023 Puya Semiconductor Co.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by Puya under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2016 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "gpio_defines.h"
#include "display.h"
#include "leds.h"
#include "slide_switch.h"
#include "battery.h"
#include "scheduler.h"
#include "ui.h"
#include "low_power.h"
#include "adc.h"

/* Private define ------------------------------------------------------------*/
/* djjw */

#define LED_OFF							(0)
#define LED_ON							(1)

#define ANIMATION_DELAY			(250)
#define PATTERN_DELAY				(50)
#define GROUP_DELAY					(100)

#define DEBOUNCE_LOCKOUT_MS	(5)

#define DEBUG_DEVELOP				0

#define SET_RED_COMPARE(x)  	(__HAL_TIM_SET_COMPARE(&htim3, RED_CHANNEL, x))
#define SET_GREEN_COMPARE(x)  (__HAL_TIM_SET_COMPARE(&htim3, GREEN_CHANNEL, x))
#define SET_BLUE_COMPARE(x)  	(__HAL_TIM_SET_COMPARE(&htim3, BLUE_CHANNEL, x))

/* Private variables ---------------------------------------------------------*/
/* Handle for the TIM3 Timer ... will be used for RGB LED PWM */
TIM_HandleTypeDef htim3 = {0};
static int16_t animation_cycle_count = 1;
_Bool low_power_enable_flag = TRUE; /* global variable to authorize entering low power */
uint32_t inactivity_timestamp;

/* Private user code ---------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void APP_SystemClockConfig(void);
static void GPIO_Init(void);
static void TIM3_Init(void);
static void run_test_patterns(void);

/**
  * @brief  Main program.
  * @retval int
  */
int main(void)
{
	taskId_t heartbeat_task_id, slide_switch_task_id, rotate_cycle_cw_task_id, rotate_cycle_ccw_task_id, rotate_cycle_alternate_task_id, battery_status_task_id, battery_voltage_task_id;
	
  /* Reset of all peripherals, Initializes the Systick. */
  HAL_Init();
  
  /* Configure the system clock */
  APP_SystemClockConfig();
	
	HAL_Delay(2000); /* safety time delay in case SWD port bricked by firmware ... remove later */
	
	/* initialize board-level features */
	leds_init();
	adc_init();
	slide_switch_init();
	scheduler_init();
	
	/* djjw - initialize pins */
	GPIO_Init();
	
	/* djjw - initialize TIM3 (RGB PWM) timer */
	TIM3_Init();
	
	/* register tasks (note - can be rescheduled or suspended in runtime using task_id) */
	heartbeat_task_id = scheduler_register_task(heartbeat, 500, 0);
	slide_switch_task_id = scheduler_register_task(slide_switch_process_events, 5, 0);
	rotate_cycle_cw_task_id = scheduler_register_task(rotate_cycle_cw, MEDIUM, 0);
	rotate_cycle_ccw_task_id = scheduler_register_task(rotate_cycle_ccw, 0, 0);
	rotate_cycle_alternate_task_id = scheduler_register_task(rotate_cycle_alternate, 0, 0);
	battery_status_task_id = scheduler_register_task(post_battery_status, 1000, 0);
	battery_voltage_task_id = scheduler_register_task(post_battery_voltage, 1250, 0);

#if !DEBUG_DEVELOP

	
#endif
	
#if DEBUG_DEVELOP
	
	/* Basic hex file */
	while(1)
	{
		display_led_toggle(rocket);
		HAL_Delay(1000);
	}
	
#endif
	
  while(1)
	{		
		/* read charging indicator ... disable low_power mode and illuninate green LEDs if charging is active */
		if(HAL_GPIO_ReadPin(CHARGE_PORT, CHARGE_PIN) == 0)
		{
			SET_GREEN_COMPARE(1023);
			HAL_TIM_PWM_Start(&htim3, GREEN_CHANNEL);
		}
		else
		{
			SET_GREEN_COMPARE(0);
			HAL_TIM_PWM_Stop(&htim3, GREEN_CHANNEL);
		}		
		low_power_enable_flag = HAL_GPIO_ReadPin(CHARGE_PORT, CHARGE_PIN);
		
		run_test_patterns();		
		scheduler_run_tasks();
		
		/* testing rescheduling */
		static uint32_t counter = 0;
		static _Bool reschedule_flag = 1;
		if(counter++ > 100000 && reschedule_flag)
		{
			scheduler_suspend_task(rotate_cycle_cw_task_id);
			scheduler_reschedule_task(rotate_cycle_alternate_task_id, SLOW, 0);
			reschedule_flag = 0;
		}
		
		/* put system in low power if inactive for a period of time */
		if(HAL_GetTick() > inactivity_timestamp + 60000)
		{
			enter_low_power(low_power_enable_flag);
			
			/* restart after wake-up */
			inactivity_timestamp = HAL_GetTick();
			scheduler_reschedule_task(slide_switch_task_id, 5, 0);
			scheduler_reschedule_task(heartbeat_task_id, 500, 0);
			scheduler_reschedule_task(rotate_cycle_cw_task_id, MEDIUM, 0);
			scheduler_reschedule_task(battery_status_task_id, 1000, 0);
			scheduler_reschedule_task(battery_voltage_task_id, 1250, 0);
			animation_cycle_count = 1;
			reschedule_flag = 1;
			counter = 0;
		}
	}	
}

/**
  * @brief  System Clock Configuration
  * @param  None
  * @retval None
	*
	*	djjw ... mostly defaults, changed HSI calibration to 24MHz (sets HSI to 24MHz)
	*
  */
void APP_SystemClockConfig(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /* Oscillator Configuration */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_LSE; /* Select oscillators HSE, HSI, LSI, LSE */
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;                          /* Enable HSI */
#if defined(RCC_HSIDIV_SUPPORT)
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;                          /* HSI not divided */
#endif
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_24MHz; /* Configure HSI clock as 8MHz ... djjw changed to 24MHz */
  RCC_OscInitStruct.HSEState = RCC_HSE_OFF;                         /* Disable HSE */
  /*RCC_OscInitStruct.HSEFreq = RCC_HSE_16_32MHz;*/
  RCC_OscInitStruct.LSIState = RCC_LSI_OFF;                         /* Disable LSI */
  RCC_OscInitStruct.LSEState = RCC_LSE_OFF;                         /* Disable LSE */
  /*RCC_OscInitStruct.LSEDriver = RCC_LSEDRIVE_MEDIUM;*/
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_OFF;                     /* Disable PLL */
  /*RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;*/          /* Select HSI as PLL source */
  /* Configure oscillators */
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    APP_ErrorHandler();
  }

  /* Clock source configuration */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1; /* Select clock types HCLK, SYSCLK, PCLK1 */
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSISYS; /* Select HSISYS as the system clock */
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;     /* AHB clock not divide */
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;      /* APB clock not divided */
  /* Configure clock source */
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    APP_ErrorHandler();
  }	
}

/**
  * @brief  GPIO Config
  * @param  None
  * @retval None
	*
	*	djjw ... mapping Geek Bar Pulse PCB to GPIOs
	*				-> (https://github.com/schlae/VapeRE/tree/main/BD0027
	*
	*	copied from PUYA SDK example
	*
  */
static void GPIO_Init(void)
{	
	/* enable GPIO port clock(s) */
	__HAL_RCC_GPIOF_CLK_ENABLE();
	
	/* declare an init structure */
	GPIO_InitTypeDef GPIO_InitStruct = {0};	
	
	/* initialize GPIO Port F charge pin as falling edge exti input */
	GPIO_InitStruct.Pin = CHARGE_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	
	HAL_GPIO_Init(CHARGE_PORT, &GPIO_InitStruct);
	
	/* Enable EXTI interrupt and configure interrupt priority */
  HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);
  HAL_NVIC_SetPriority(EXTI0_1_IRQn, 0, 0);
	
	/* initialize GPIO Port F LED pin as output */
	GPIO_InitStruct.Pin = GPIO_PIN_2;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	
	HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
}

/**
  * @brief  TIM3 Config
  * @param  None
  * @retval None
	*
	*	djjw ... PWM control for RGB LEDs (1kHz frequency, 10-bit resolution)
	*
	*	copied from PUYA SDK example
	*
  */
static void TIM3_Init(void)
{
	TIM_OC_InitTypeDef sConfig;
	
	htim3.Instance = TIM3;																					/* Select TIM3 */
  htim3.Init.Period            = 1024 - 1;												/* Auto-reload value */
  htim3.Init.Prescaler         = 24 - 1;													/* Prescaler of 800-1 */
  htim3.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;					/* No clock division */
  htim3.Init.CounterMode       = TIM_COUNTERMODE_UP;							/* Up counting */
  htim3.Init.RepetitionCounter = 1 - 1;														/* No repetition counting */
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;	/* Auto-reload register not buffered */
	
  /* Initialize timer base */
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    APP_ErrorHandler();
  }
  
  sConfig.OCMode       = TIM_OCMODE_PWM1;							/* Configure as PWM1 output */
  sConfig.OCPolarity   = TIM_OCPOLARITY_HIGH;					/* OC channel output is active high level */
  sConfig.OCFastMode   = TIM_OCFAST_DISABLE;					/* Fast output mode is disabled */
  sConfig.OCNPolarity  = TIM_OCNPOLARITY_HIGH;				/* OCN channel output is active high level */
  sConfig.OCNIdleState = TIM_OCNIDLESTATE_RESET;			/* OC1N output is low in idle state */
  sConfig.OCIdleState  = TIM_OCIDLESTATE_RESET;				/* OC1 output is low in idle state */
	sConfig.Pulse				 = 0;														/* initial value */
	
	if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfig, TIM_CHANNEL_1) != HAL_OK)
  {
    APP_ErrorHandler();
  }
	
	if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfig, TIM_CHANNEL_2) != HAL_OK)
  {
    APP_ErrorHandler();
  }
	
	if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfig, TIM_CHANNEL_3) != HAL_OK)
  {
    APP_ErrorHandler();
  }
	
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void APP_ErrorHandler(void)
{
  /* Infinite loop */
  while (1)
  {
  }
}

static void run_test_patterns(void)
{
	for(uint8_t i = 0; i < animation_cycle_count; i++)
	{
		/* animate peripheral RGB LEDs */
		HAL_TIM_PWM_Start(&htim3, RED_CHANNEL);
		HAL_TIM_PWM_Start(&htim3, GREEN_CHANNEL);
		HAL_TIM_PWM_Start(&htim3, BLUE_CHANNEL);
		
		SET_RED_COMPARE(1023);
		HAL_Delay(ANIMATION_DELAY);
		SET_BLUE_COMPARE(1023);
		HAL_Delay(ANIMATION_DELAY);
		SET_GREEN_COMPARE(1023);
		HAL_Delay(ANIMATION_DELAY);
		SET_RED_COMPARE(0);
		HAL_Delay(ANIMATION_DELAY);
		SET_BLUE_COMPARE(0);
		HAL_Delay(ANIMATION_DELAY);
		SET_GREEN_COMPARE(0);
		HAL_Delay(ANIMATION_DELAY);
		
		/* testing led array ... */			
		display_clear(); /* clear display */
		
		HAL_Delay(250);
		
		SET_RED_COMPARE(1023);
		
		/* illuminate each display elements and leave trail */
		for(uint8_t i = 0; i < MAX_LEDS; i++)
		{
			display_led(i, LED_ON);

			HAL_Delay(PATTERN_DELAY);
		}
		
		HAL_Delay(250);		
		
		/* pick up trail of display elements (in reverse order) */
		for(uint8_t i = 0; i < MAX_LEDS; i++)
		{
			display_led(MAX_LEDS - 1 - i, LED_OFF);
			
			HAL_Delay(PATTERN_DELAY);
		}
		
		SET_RED_COMPARE(0);
		
		/* flash display */
		HAL_Delay(250);
		SET_GREEN_COMPARE(1023);
		HAL_Delay(250);
		for(uint8_t i = 0; i < 3; i++)
		{
			display_all();
			HAL_Delay(250);
			display_clear();
			HAL_Delay(250);
		}
		HAL_Delay(250);
		
		/* testing groups ... leave trail */
		for(uint8_t i = one_top; i <= bar_bottom; i++)
		{
			for(uint8_t j = 0; j < led_group_array[i][0]; j++)
			{
				display_led(led_group_array[i][j + 1], LED_ON);
				
				HAL_Delay(PATTERN_DELAY);
			}
			HAL_Delay(250);
		}
		
		HAL_Delay(250);
		
		/* testing groups ... pick up trail (in reverse order) */
		for(uint8_t i = one_top; i <= bar_bottom; i++)
		{			
			for(uint8_t j = 0; j < led_group_array[bar_bottom - i][0]; j++)
			{
				display_led(led_group_array[bar_bottom - i][led_group_array[bar_bottom - i][0] - 1 - j + 1], LED_OFF);
				
				HAL_Delay(PATTERN_DELAY);
			}
			HAL_Delay(250);
		}
		
		HAL_Delay(250);
		
		/* testing groups with display_group() */
		for(uint8_t i = one_top; i <= bar_bottom; i++)
		{
			display_group(i, 0xff);
			HAL_Delay(GROUP_DELAY);
		}
		
		HAL_Delay(250);
		
		/* testing groups with display_group() */
		for(uint8_t i = one_top; i <= bar_bottom; i++)
		{			
			display_group(bar_bottom - i, 0x00);
			HAL_Delay(GROUP_DELAY);
		}
		
		HAL_Delay(250);
		
		SET_GREEN_COMPARE(0);
		
		for(uint8_t count = 0; count <= 199; count++)
		{
			SET_BLUE_COMPARE(count * 5);
			display_decimal(display_top, count);
			display_decimal(display_bottom, 199 - count);
			HAL_Delay(100);
		}
		
		SET_BLUE_COMPARE(1023);
		
		HAL_Delay(250);
		
		inactivity_timestamp = HAL_GetTick();
	}

//	display_led(rocket, LED_ON);
	
	animation_cycle_count = 0;
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     for example: printf("Wrong parameters value: file %s on line %d\r\n", file, line)  */
  /* Infinite loop */
  while (1)
  {
  }
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT Puya *****END OF FILE******************/
