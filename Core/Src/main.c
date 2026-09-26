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
	*							 
  *              -> Normal state of unit is in deep sleep (or stop), SWD is
  *                 unavailable. Slide switch wakes unit
  *              /WR   - PB10 (BluePill P3 header, pin 15)
  *              /RD   - PB11 ... not used (BluePill P3 header, pin 16)
  *              /TEST - PC15 (BluePill P3 header, pin 4)
  *
  * Peripheral setup:
  *              UART - USART1 ... Tx = PA9, Rx = PA10
  *              Escape sequence timeout timer - TIM1 (oneshot mode, 20 ms interrupt)
  *              Readback timer - TIM2 (oneshot mode, 20/50 ms interrupt)
  *              Switch matrix timer - TIM3 (555 us interrupt)
  *              Hardware-based delay - TIM4 (.5 us count)
  *              I2C1 - future use (CapSense buttons)
  *              Program/Debug - SWD ... PA13 (SWDIO)/PA14 (SWDCK)
  *
  * Serial terminal (TeraTerm, etc) navigation:
  *              Up/Down arrow keys - scroll back/forward to previous/next line
  *              Right/Left arrow keys - replay line, fast/normal playback speed
  *              PageUp/PageDown keys - scroll back/forward PAGE_JUMP_SIZE lines
  *              Home key - return to the most recent line
  *              End key - go directly to the oldest line/record
  *              Delete key - enter pause/single-step mode (subsequent presses single-steps readback)
  *              Insert key - dual-purpose for now ... print the deepest FIFO level so far
  *              	and resume readback (i.e. restart readback/TIM2 timer)
  *              Escape key - abandon (escape) a readback by quickly recalling the line
  *
  * 4x4 matrix button navigation:
  *             		Col0	Col1	Col2	Col3
  *             Row0	PageUp	Home	N/A		SingleStep
  *             Row1	PageDn	End		N/A		Resume
  *             Row2	N/A		N/A		N/A		ScrollUp
  *             Row3	ReadBkF	Escape	ReadBk	ScrollDn
  *
  * Usage:       #include <iee_flip_03600_20_040.h>
  *              NOTE - arbitrarily chose BELL (ctrl-G) character to reset display.  *
  *
  * Update 18-Aug-2024:
  *		- imported source/header files from PSoC version
  *
  * Update 19-Aug-2024:
  *		- completed low-level hardware drivers for STM32
  *
  * Update 20-Aug-2024:
  *		- replaced HAL_Delay with cycle count
  *
  * Update 22-Aug-2024:
  *		- added 4x4 switch matrix
  *		- imported debounce library from https://github.com/tcleg/Button_Debouncer/tree/master
  *			-> note ... debounce library is an implementation of Jack Ganssle (https://www.ganssle.com/debouncing-pt2.htm)
  *		- replaced cycle count delay with TIM4 delay function
  *
  * Update 23-Aug-2024:
  *		- modified behavior of single-step readback
  *			-> if held, single-step reads back characters at half the readback speed
  *			-> when released, readback timer is stopped and reloaded with previous readback speed value
  *			-> press_and_release behaves the same (reads back one character for each button press)
  *
  * Update 24-Aug-2024:
  *		- fine tuned TIM4 delay function
  *			-> TIM4 is on 72MHz bus
  *			-> prescaler set to 18 (well, 18 - 1) ... 4MHz counter frequency
  *			-> modified delay_us() function to account for 4MHz clock
  *		- deactivated all column drivers in default switch condition
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
#define RED_PIN				GPIO_PIN_5
#define BLUE_PIN			GPIO_PIN_2
#define GREEN_PIN			GPIO_PIN_4

#define SLIDE_SWITCH_PORT		GPIOA
#define SLIDE_SWITCH_PIN		GPIO_PIN_7

#define CHARGE_PORT		GPIOF
#define CHARGE_PIN		GPIO_PIN_1

#define NUM_LED_CATHODES	(7)

#define ANIMATION_DELAY		(250)

/* Private variables ---------------------------------------------------------*/
/* Handle for the TIM1 Timer ... will be used for display muxing */
TIM_HandleTypeDef		htim1 = {0};

/* LED display 7-anode array (note - anodes span GPIO Port A and B) */
volatile uint32_t LED_display_buffer[7][2] = {0};

/* Private user code ---------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void APP_SystemClockConfig(void);
static void GPIO_Init(void);
static void TIM1_Init(void);

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
	
	HAL_Delay(1000); /* safety time delay in case SWD port bricked by firmware ... remove later */
	
	/* djjw - initialize pins */
	GPIO_Init();
	
	/* djjw - initialize TIM1 timer */
	TIM1_Init();
	
	/* Start the TIM1 Base generation in interrupt mode */
  if (HAL_TIM_Base_Start_IT(&htim1) != HAL_OK)
  {
    APP_ErrorHandler();
  }

  while (1)
  {		
		/* read charging indicator ... illuninate BLUE LEDs if charging is active */
		HAL_GPIO_WritePin(GPIOA, BLUE_PIN, HAL_GPIO_ReadPin(CHARGE_PORT, CHARGE_PIN) ? 0 : 1);
		
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
		
		/* testing led array */
		for(uint8_t i = 0; i < max_leds; i++)
		{
			LED_display_buffer[LED_port_position[i].scan_line][0] = LED_port_position[i].segment_GPIOA;
			LED_display_buffer[LED_port_position[i].scan_line][1] = LED_port_position[i].segment_GPIOB;
			HAL_Delay(100);
			LED_display_buffer[LED_port_position[i].scan_line][0] = 0x98000000;
			LED_display_buffer[LED_port_position[i].scan_line][1] = 0x01e80000;
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
	*	djjw ... mapping Geek Bar Pulse PCB layout (https://github.com/schlae/VapeRE/tree/main/BD0027)
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
	
	GPIO_InitStruct.Pin = RED_PIN | BLUE_PIN | GREEN_PIN | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_15;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin = SLIDE_SWITCH_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	
	HAL_GPIO_Init(SLIDE_SWITCH_PORT, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin = GPIO_PIN_1;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	
	HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
	
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
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{	
	/* port masks use BSRR lower 16-bits (set mask) to drive cathodes inactive (high) */
	#define PORTA_CATHODE_MASK	0x00000700
	#define PORTB_CATHODE_MASK	0x00000017
	
	/* port masks use BSRR upper 16-bits (reset mask) to drive anodes inactive (low) */
	#define PORTA_ANODE_MASK	0x98000000
	#define PORTB_ANODE_MASK	0x01e80000
	
	(void) htim; /* suppress compiler warning */
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
