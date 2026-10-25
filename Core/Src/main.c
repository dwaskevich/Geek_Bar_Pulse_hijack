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
	*							 SWDIO - PA13, SWCLK - PA14
  *              NOTE -> Normal state of unit is in deep sleep (or stop), SWD interface
  *                 is normally unavailable. Slide switch wakes unit and provides
	*									small connect window. Can also plug in charger to wake uinit.
  *              See schematic for feature element mapping (i.e. RGB LEDs, display, etc)
	*									-> https://github.com/schlae/VapeRE/tree/main/BD0027
  *              Charge indicator - PF1
  *              Slide switch - PA7 (pulls low in one position, high in remaining two)
  *
  * Peripheral setup:
  *              UART - USART1 ... re-purpose SWD via AF registers:
	*									-> Rx = PA13 (AF8), Tx = PA14 (AF1)
  *              LED matrix timer - TIM1 (auto-reload mode, 1.6 ms interrupt)
  *              Application time-out timer - TIM2 (oneshot mode, 10s interrupt)
  *              Low power timer - LPTIM (? interrupt)
  *              Hardware-based delay - TIM4 (.5 us count)
  *              I2C1 - future use (CapSense buttons)
  *              Program/Debug - SWD ... PA13 (SWDIO)/PA14 (SWDCK)
  *
  *
  * Usage:       #include display.h, leds.h
  *              NOTE - 
  *
  * Update 26-Sept-2026:
  *		- added LED mapping (leds.h)
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
	*		- created an leds.c source file and moved led matrix to the .c file.
  *		- updated leds.h to declare the led matrix as extern
  *
  * Update 23-Oct-2026:
  *		- removed reset_segment from LED_port_position structure, replaced with
	*			simple left shift of set_segment value (<< BSRR_RESET_SHIFT (16))
	*		- first pass slide switch gpio configuration (exti wakeup source)
  *
  * Update 24-Oct-2026:
  *		- implemented test code to measure low-power stop current
	*			-> 75-79 uA
  *		- created enter_low_power() function
  *
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
#include "display.h"
#include "leds.h"

/* Private define ------------------------------------------------------------*/
/* djjw */
#define RED_PIN							GPIO_PIN_5
#define BLUE_PIN						GPIO_PIN_2
#define GREEN_PIN						GPIO_PIN_4

#define CHARGE_PORT					GPIOF
#define CHARGE_PIN					GPIO_PIN_1

#define NUM_LED_CATHODES		(7)

#define ANIMATION_DELAY			(250)
#define PATTERN_DELAY				(25)

#define BSRR_RESET_SHIFT		(16)

#define DEBOUNCE_LOCKOUT_MS	(5)

#define DEBUG_DEVELOP				0

/* Private variables ---------------------------------------------------------*/
/* Handle for the TIM1 Timer ... will be used for display muxing */
TIM_HandleTypeDef		htim1 = {0};

/* LED display 7-anode array (note - anodes span GPIO Port A and B) */
volatile uint32_t LED_display_buffer[7][2] = {0};

volatile uint8_t  switch_event_pending = 0;   /* slide switch interrupt flag */
volatile uint32_t switch_event_timestamp = 0; /* used for switch debounce */

/* Private user code ---------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void APP_SystemClockConfig(void);
static void GPIO_Init(void);
static void TIM1_Init(void);
static void enter_low_power(void);

/**
  * @brief  Main program.
  * @retval int
  */
int main(void)
{	
  /* Reset of all peripherals, Initializes the Systick. */
  HAL_Init();
  
  /* Configure the system clock */
  APP_SystemClockConfig();
	
	HAL_Delay(2000); /* safety time delay in case SWD port bricked by firmware ... remove later */
	
	/* djjw - initialize pins */
	GPIO_Init();
	
	/* djjw - initialize TIM1 timer */
	TIM1_Init();
	
	/* Start the TIM1 Base generation in interrupt mode */
  if (HAL_TIM_Base_Start_IT(&htim1) != HAL_OK)
  {
    APP_ErrorHandler();
  }

#if DEBUG_DEVELOP
//	while(1);
	
//	while(1)
//	{
//		/* read charging indicator ... illuninate BLUE LEDs if charging is active */
//		HAL_GPIO_WritePin(GPIOA, BLUE_PIN, HAL_GPIO_ReadPin(CHARGE_PORT, CHARGE_PIN) ? 0 : 1);		
//	}
	
	while(1)
	{
		if(HAL_GPIO_ReadPin(SLIDE_SWITCH_PORT, SLIDE_SWITCH_PIN) == 0)
			HAL_GPIO_WritePin(GPIOA, RED_PIN, 0);
		else
			HAL_GPIO_WritePin(GPIOA, RED_PIN, 1);
	}
	
#endif

  while(1)
	{
		uint32_t last_processed_time = 0;
		static int16_t animation_cycle_count = 3;
		
		/* --- Deferred Interrupt Processing Area --- */
    if (switch_event_pending) 
    {
      switch_event_pending = 0; /* clear pending flag */
      
      /* check the timestamp captured by ISR */
      if ((switch_event_timestamp - last_processed_time) >= DEBOUNCE_LOCKOUT_MS)
      {
        /* debounce lockout window expired ... switch closure validated */
        last_processed_time = switch_event_timestamp;
        
        /* read slide switch pin */
        if (HAL_GPIO_ReadPin(SLIDE_SWITCH_PORT, SLIDE_SWITCH_PIN) == GPIO_PIN_RESET)
        {
          /* falling edge ... slide switch moved to "close", prepare for low-power stop */
          HAL_GPIO_TogglePin(GPIOA, RED_PIN);
					HAL_Delay(50);
					HAL_GPIO_TogglePin(GPIOA, RED_PIN);
					HAL_Delay(50);
					HAL_GPIO_TogglePin(GPIOA, RED_PIN);
					HAL_Delay(50);
					HAL_GPIO_TogglePin(GPIOA, RED_PIN);
					HAL_Delay(50);
					HAL_GPIO_TogglePin(GPIOA, RED_PIN);
					HAL_Delay(50);
					HAL_GPIO_WritePin(GPIOA, RED_PIN, 0);
					HAL_GPIO_WritePin(GPIOA, BLUE_PIN, 0);
					
					enter_low_power();
					
					animation_cycle_count = 3; /* reset run-mode animation loop count */
        }
        else /* rising edge - slide switch moved to "pulse" */
        {
          HAL_GPIO_WritePin(GPIOA, BLUE_PIN, 1);
        }
      }			
		}
		
		for(uint8_t i = 0; i < animation_cycle_count; i++)
		{
			/* animate peripheral RGB LEDs */
			HAL_GPIO_TogglePin(GPIOA, RED_PIN);
			HAL_Delay(ANIMATION_DELAY);
			HAL_GPIO_TogglePin(GPIOA, BLUE_PIN);
			HAL_Delay(ANIMATION_DELAY);
			HAL_GPIO_TogglePin(GPIOA, GREEN_PIN);
			HAL_Delay(ANIMATION_DELAY);
			HAL_GPIO_TogglePin(GPIOA, RED_PIN);
			HAL_Delay(ANIMATION_DELAY);
			HAL_GPIO_TogglePin(GPIOA, BLUE_PIN);
			HAL_Delay(ANIMATION_DELAY);
			HAL_GPIO_TogglePin(GPIOA, GREEN_PIN);
			HAL_Delay(ANIMATION_DELAY);
			
			/* testing led array ...
					-> illminate individual display element one at a time */
			for(uint8_t i = 0; i < MAX_LEDS; i++)
			{
				/* write display element bit to BSSR low (set) word to illuminate selected display element
						-> NOTE - this is destructive to all other display elements associated with this scan line,
											to preserve other display elements use |= (see "leave trail" loop below) */
				LED_display_buffer[LED_port_position[i].scan_line][0] = LED_port_position[i].set_segment_GPIOA;
				LED_display_buffer[LED_port_position[i].scan_line][1] = LED_port_position[i].set_segment_GPIOB;

				/* hold display element illumination for a pre-defined delay */
				HAL_Delay(PATTERN_DELAY);
				
				/* remove display element bit from BSRR low (set) word */
				LED_display_buffer[LED_port_position[i].scan_line][0] &= ~LED_port_position[i].set_segment_GPIOA;
				LED_display_buffer[LED_port_position[i].scan_line][1] &= ~LED_port_position[i].set_segment_GPIOB;

				/* add display element bit to BSRR high (reset) word to extinguish display element */
				LED_display_buffer[LED_port_position[i].scan_line][0] = LED_port_position[i].set_segment_GPIOA << BSRR_RESET_SHIFT;
				LED_display_buffer[LED_port_position[i].scan_line][1] = LED_port_position[i].set_segment_GPIOB << BSRR_RESET_SHIFT;
			}
			
			/* clear display buffer */
			for(uint8_t i = 0; i < NUM_LED_CATHODES; i++)
			{
				LED_display_buffer[i][0] = 0;
				LED_display_buffer[i][1] = 0;
			}
			
			HAL_Delay(500);
			
			/* leave trail of display elements (i.e. OR new bit selection with previous value) */
			for(uint8_t i = 0; i < MAX_LEDS; i++)
			{
				LED_display_buffer[LED_port_position[i].scan_line][0] |= LED_port_position[i].set_segment_GPIOA;
				LED_display_buffer[LED_port_position[i].scan_line][1] |= LED_port_position[i].set_segment_GPIOB;

				HAL_Delay(PATTERN_DELAY);
			}
			
			HAL_Delay(500);		
			
			/* pick up trail of display elements (in reverse order) */
			for(uint8_t i = 0; i < MAX_LEDS; i++)
			{
				/* remove display element bit from BSRR low (set) word */
				LED_display_buffer[LED_port_position[MAX_LEDS - 1 - i].scan_line][0] &= ~LED_port_position[MAX_LEDS - 1 - i].set_segment_GPIOA;
				LED_display_buffer[LED_port_position[MAX_LEDS - 1 - i].scan_line][1] &= ~LED_port_position[MAX_LEDS - 1 - i].set_segment_GPIOB;
				
				/* add display element bit to BSRR high (reset) word */	
				LED_display_buffer[LED_port_position[MAX_LEDS - 1 - i].scan_line][0] |= LED_port_position[MAX_LEDS - 1 - i].set_segment_GPIOA << BSRR_RESET_SHIFT;
				LED_display_buffer[LED_port_position[MAX_LEDS - 1 - i].scan_line][1] |= LED_port_position[MAX_LEDS - 1 - i].set_segment_GPIOB << BSRR_RESET_SHIFT;
				
				HAL_Delay(PATTERN_DELAY);
			}
			
			HAL_Delay(500);
			
		}
		
		animation_cycle_count = 0;
		
		LED_display_buffer[LED_port_position[rocket].scan_line][0] = LED_port_position[rocket].set_segment_GPIOA;
		LED_display_buffer[LED_port_position[rocket].scan_line][1] = LED_port_position[rocket].set_segment_GPIOB;
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
static void APP_SystemClockConfig(void)
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
	*				-> (https://github.com/schlae/VapeRE/tree/main/BD0027)
	*
  */
static void GPIO_Init(void)
{	
	/* enable GPIO port clock(s) */
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOF_CLK_ENABLE();
	
	/* declare an init structure */
	GPIO_InitTypeDef GPIO_InitStruct = {0};	
	
	/* initialize GPIO Port A LED pins as outputs */
	GPIO_InitStruct.Pin = RED_PIN | BLUE_PIN | GREEN_PIN | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_15;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	/* initialize GPIO Port A Slide Switch pin as both edge interrupt and enable NVIC */
	GPIO_InitStruct.Pin = SLIDE_SWITCH_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	
	HAL_GPIO_Init(SLIDE_SWITCH_PORT, &GPIO_InitStruct);
	
	/* Enable EXTI interrupt */
  HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
	
  /* Configure interrupt priority */
  HAL_NVIC_SetPriority(EXTI4_15_IRQn, 2, 0);
	
	/* initialize GPIO Port B LED pins as outputs */
	GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	/* initialize GPIO Port F charge pin as input */
	GPIO_InitStruct.Pin = CHARGE_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	
	HAL_GPIO_Init(CHARGE_PORT, &GPIO_InitStruct);
	
	/* initialize GPIO Port F LED pin as output */
	GPIO_InitStruct.Pin = GPIO_PIN_2;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	
	HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
}

/**
  * @brief  TIM1 Config
  * @param  None
  * @retval None
	*
	*	djjw ... LED display mux timer (600 Hz refresh) - 24MHz/40(prescaler) = 600,000, period of 1000 = 600Hz
	*
  */
static void TIM1_Init(void)
{	
	htim1.Instance               = TIM1;                             /* Select TIM1 */
  htim1.Init.Period            = 1000 - 1;                         /* Auto-reload value */
  htim1.Init.Prescaler         = 40 - 1;                           /* Prescaler of 40 - 1 */
  htim1.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;           /* No clock division */
  htim1.Init.CounterMode       = TIM_COUNTERMODE_UP;               /* Up counting */
  htim1.Init.RepetitionCounter = 1 - 1;                            /* No repetition counting */
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;   /* Auto-reload register not buffered */
	
  /* Initialize TIM1 */
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    APP_ErrorHandler();
  }
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @param  htim:TIM handle
  * @retval None
	*
	*	djjw ... LED display mux ISR (600Hz refresh rate)
	*
	*	Notes on HAL interrupt plumbing:
	*		- hardware interrupt vector table is mapped in startup_py32f030x8.s
	*		- handler must be placed in py32f0xx_it.c
	*			-> reference example in puya's sdk TIMER1 TIM1_Update_IT example:
	*				 PY32F0xx_Firmware-master\PY32F0xx_Firmware-master\Projects\PY32F030-STK\Example\TIMER1\TIM1_Update_IT\Src
	*		- py32f0xx_hal_tim.c has a weak definition of the callback:
	*			-> __weak void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
	*		- HAL_TIM_PeriodElapsedCallback definition here overrides weak definition
	*			in SDK source file
	*
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{	
	/* port masks use BSRR lower 16-bits (set mask) to drive cathodes inactive (high) */
	#define PORTA_CATHODE_MASK	0x00000700
	#define PORTB_CATHODE_MASK	0x00000017
	
	/* port masks use BSRR upper 16-bits (reset mask) to drive anodes inactive (low) */
	#define PORTA_ANODE_MASK	0x98000000
	#define PORTB_ANODE_MASK	0x01e80000
	
	(void) htim; /* suppress compiler warning (if more than one timer, use htim->Instance) */
	static uint8_t cathode_counter = 0;
	
	/* Constant mapping of cathode activation masks (Upper 16-bits of BSRR for Reset) */
	/* Structure: { GPIOA_ResetMask, GPIOB_ResetMask } */
	static const uint32_t CATHODE_BITMASKS[NUM_LED_CATHODES][2] = {
			{ 0x04000000, 0x00000000 }, // Case 0: TOP_LEDS (PA10)
			{ 0x02000000, 0x00000000 }, // Case 1: BOT_LEDS (PA9)
			{ 0x01000000, 0x00000000 }, // Case 2: LED_BOTRIGHT (PA8)
			{ 0x00000000, 0x00100000 }, // Case 3: MID_LEDS (PB4)
			{ 0x00000000, 0x00040000 }, // Case 4: LED_BOTLEFT (PB2)
			{ 0x00000000, 0x00020000 }, // Case 5: LED_TOPRIGHT (PB1)
			{ 0x00000000, 0x00010000 }  // Case 6: LED_TOPLEFT (PB0)
	};
	
	/* turn off all cathodes to prevent ghosting while updating next anode group */
	GPIOA->BSRR = PORTA_CATHODE_MASK;
	GPIOB->BSRR = PORTB_CATHODE_MASK;
	
	/* clear previous anode values from GPIO ports */
	GPIOA->BSRR = PORTA_ANODE_MASK;
	GPIOB->BSRR = PORTB_ANODE_MASK;
	
	/* load GPIO Port A and B with anode values for next cathode in scan */
	GPIOA->BSRR = LED_display_buffer[cathode_counter][0];
	GPIOB->BSRR = LED_display_buffer[cathode_counter][1];
	
	/* activate current scan cathode */
	GPIOA->BSRR = CATHODE_BITMASKS[cathode_counter][0];
	GPIOB->BSRR = CATHODE_BITMASKS[cathode_counter][1];
	
	cathode_counter++;
	if(cathode_counter >= NUM_LED_CATHODES)
		cathode_counter = 0;
}

/**
  * @brief  GPIO EXTI callback
  * @param  GPIO_Pin
  * @retval None
	*
	*	djjw ... Slide Switch interrupt callback. Captures timestamp and sets flag.
	*
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == SLIDE_SWITCH_PIN)
  {
    /* Capture timestamp, set switch event flag */
    switch_event_timestamp = HAL_GetTick();
    switch_event_pending = 1;
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

/**
  * @brief  Low power stop mode
  * @param  None
  * @retval None
	*
	*	djjw ... prepares system for and enters low power stop.
	*
  */
void enter_low_power(void)
{
	display_cathodes_off(); /* turn off all display cathodes */
		
	/* put GPIO in low-power state (analog is best choice to avoid leakage current) */
	GPIO_InitTypeDef GPIO_InitStruct = {0};	
	
	/* GPIOA */
	GPIO_InitStruct.Pin = RED_PIN | BLUE_PIN | GREEN_PIN | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_15;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	/* GPIOB */
	GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/* stop TIM1 timer and suspend SysTick */
	HAL_TIM_Base_Stop_IT(&htim1);
	HAL_SuspendTick();
	
	/* Enter STOP mode with interrupt wakeup */
	HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
	
	/* exit from stop ... reconfigure clocks and gpio, restart timer and SysTick */
	APP_SystemClockConfig();
	
	/* return GPIO Port A LED pins to outputs */
	GPIO_InitStruct.Pin = RED_PIN | BLUE_PIN | GREEN_PIN | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_15;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	/* return GPIO Port B LED pins to outputs */
	GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	HAL_ResumeTick();
	HAL_TIM_Base_Start_IT(&htim1);
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
