/*
 * leds.c
 *
 *  Created:	23-Oct-2026
 *  Author:		David Waskevich
 *
 *  Description: LED matrix definition (Flash)
 *
 *	Update 23-Oct-2026:
 *			- created file
 *  
 */

#include "leds.h"
#include "main.h"

/* Handle for the TIM16 Timer ... will be used for display muxing */
TIM_HandleTypeDef htim16 = {0};

volatile uint32_t LED_display_buffer[7][2]; /* display buffer holds GPIOA and GPIOB data for 7 scan lines */

const LED_port_position_t LED_port_position[MAX_LEDS] =
{
/*{scan, setPortA, setPortB }		note: rstPortA and rstPortB are just 16-bit left-shifted setPortA/B values */
	{0, 0x00000000, 0x00000100}, 	// 0  - arc_top_1,
	{0, 0x00000000, 0x00000080}, 	// 1  - arc_top_2,
	{0, 0x00000000, 0x00000040}, 	// 2  - arc_top_3,
	{0, 0x00000000, 0x00000020}, 	// 3  - arc_top_4,
	{0, 0x00000000, 0x00000008}, 	// 4  - arc_top_5,
	{0, 0x00008000, 0x00000000}, 	// 5  - lightning_bolt,
	{6, 0x00000800, 0x00000000}, 	// 6  - one_top_upper,
	{5, 0x00000800, 0x00000000}, 	// 7  - one_top_lower,
	{6, 0x00000000, 0x00000100}, 	// 8  - digit_top_left_a,  digit1_top_a = digit_top_left_a,
	{6, 0x00000000, 0x00000080}, 	// 9  - digit_top_left_b,  digit1_top_b = digit_top_left_b,
	{6, 0x00000000, 0x00000040}, 	// 10 - digit_top_left_c,  digit1_top_c = digit_top_left_c,
	{6, 0x00000000, 0x00000020}, 	// 11 - digit_top_left_d,  digit1_top_d = digit_top_left_d,
	{6, 0x00000000, 0x00000008}, 	// 12 - digit_top_left_e,  digit1_top_e = digit_top_left_e,
	{6, 0x00008000, 0x00000000}, 	// 13 - digit_top_left_f,  digit1_top_f = digit_top_left_f,
	{6, 0x00001000, 0x00000000}, 	// 14 - digit_top_left_g,  digit1_top_g = digit_top_left_g,
	{5, 0x00000000, 0x00000100}, 	// 15 - digit_top_right_a, digit0_top_a = digit_top_right_a,
	{5, 0x00000000, 0x00000080}, 	// 16 - digit_top_right_b, digit0_top_b = digit_top_right_b,
	{5, 0x00000000, 0x00000040}, 	// 17 - digit_top_right_c, digit0_top_c = digit_top_right_c,
	{5, 0x00000000, 0x00000020}, 	// 18 - digit_top_right_d, digit0_top_d = digit_top_right_d,
	{5, 0x00000000, 0x00000008}, 	// 19 - digit_top_right_e, digit0_top_e = digit_top_right_e,
	{5, 0x00008000, 0x00000000}, 	// 20 - digit_top_right_f, digit0_top_f = digit_top_right_f,
	{5, 0x00001000, 0x00000000}, 	// 21 - digit_top_right_g, digit0_top_g = digit_top_right_g,
	{0, 0x00001000, 0x00000000}, 	// 22 - bar_top_left,
	{0, 0x00000800, 0x00000000}, 	// 23 - bar_top_right,
	{3, 0x00000000, 0x00000100}, 	// 24 - cycle_nw,
	{3, 0x00000000, 0x00000080}, 	// 25 - cycle_ne,
	{3, 0x00000000, 0x00000008}, 	// 26 - rocket,
	{3, 0x00000000, 0x00000040}, 	// 27 - cycle_sw,
	{3, 0x00000000, 0x00000020}, 	// 28 - cycle_se,
	{1, 0x00001000, 0x00000000}, 	// 29 - bar_bot_left,
	{1, 0x00000800, 0x00000000}, 	// 30 - bar_bot_right,
	{4, 0x00000800, 0x00000000}, 	// 31 - one_bot_upper,
	{2, 0x00000800, 0x00000000}, 	// 32 - one_bot_lower,
	{4, 0x00000000, 0x00000100}, 	// 33 - digit_bot_left_a,  digit1_bot_a = digit_bot_left_a,
	{4, 0x00000000, 0x00000080}, 	// 34 - digit_bot_left_b,  digit1_bot_b = digit_bot_left_b,
	{4, 0x00000000, 0x00000040}, 	// 35 - digit_bot_left_c,  digit1_bot_c = digit_bot_left_c, ????????
	{4, 0x00000000, 0x00000020}, 	// 36 - digit_bot_left_d,  digit1_bot_d = digit_bot_left_d, ????????
	{4, 0x00000000, 0x00000008}, 	// 37 - digit_bot_left_e,  digit1_bot_e = digit_bot_left_e, ????????
	{4, 0x00008000, 0x00000000}, 	// 38 - digit_bot_left_f,  digit1_bot_f = digit_bot_left_f,
	{4, 0x00001000, 0x00000000}, 	// 39 - digit_bot_left_g,  digit1_bot_g = digit_bot_left_g, dim
	{2, 0x00000000, 0x00000100}, 	// 40 - digit_bot_right_a, digit0_bot_a = digit_bot_right_a,
	{2, 0x00000000, 0x00000080}, 	// 41 - digit_bot_right_b, digit0_bot_b = digit_bot_right_b,
	{2, 0x00000000, 0x00000040}, 	// 42 - digit_bot_right_c, digit0_bot_c = digit_bot_right_c,
	{2, 0x00000000, 0x00000020}, 	// 43 - digit_bot_right_d, digit0_bot_d = digit_bot_right_d,
	{2, 0x00000000, 0x00000008}, 	// 44 - digit_bot_right_e, digit0_bot_e = digit_bot_right_e,
	{2, 0x00008000, 0x00000000}, 	// 45 - digit_bot_right_f, digit0_bot_f = digit_bot_right_f,
	{2, 0x00001000, 0x00000000}, 	// 46 - digit_bot_right_g, digit0_bot_g = digit_bot_right_g,
	{1, 0x00008000, 0x00000000}, 	// 47 - tear_drop,
	{1, 0x00000000, 0x00000100}, 	// 48 - arc_bot_1,
	{1, 0x00000000, 0x00000080}, 	// 49 - arc_bot_2,
	{1, 0x00000000, 0x00000040}, 	// 50 - arc_bot_3,
	{1, 0x00000000, 0x00000020}, 	// 51 - arc_bot_4,
	{1, 0x00000000, 0x00000008}, 	// 52 - arc_bot_5,	
};

/* define LED groups */
const uint8_t led_group_array[NUM_GROUPS][8] = {
	/* # elements in group, LED identifiers ..., padding */
	{2, one_top_upper, one_top_lower, 0, 0, 0, 0, 0},
	{7, digit_top_left_a, digit_top_left_b, digit_top_left_c, digit_top_left_d, digit_top_left_e, digit_top_left_f, digit_top_left_g},
	{7, digit_top_right_a, digit_top_right_b, digit_top_right_c, digit_top_right_d, digit_top_right_e, digit_top_right_f, digit_top_right_g},
	{2, one_bot_upper, one_bot_lower, 0, 0, 0, 0, 0},
	{7, digit_bot_left_a, digit_bot_left_b, digit_bot_left_c, digit_bot_left_d, digit_bot_left_e, digit_bot_left_f, digit_bot_left_g},
	{7, digit_bot_right_a, digit_bot_right_b, digit_bot_right_c, digit_bot_right_d, digit_bot_right_e, digit_bot_right_f, digit_bot_right_g},
	{5, arc_top_1, arc_top_2, arc_top_3, arc_top_4, arc_top_5},
	{5, arc_bot_1, arc_bot_2, arc_bot_3, arc_bot_4, arc_bot_5},
	{4, cycle_ne, cycle_se, cycle_sw, cycle_nw, 0},
	{2, bar_top_left, bar_top_right, 0, 0, 0},
	{2, bar_bot_left, bar_bot_right, 0, 0, 0},
	{1, arc_top_3, 0, 0, 0, 0, 0, 0},
	{2, arc_top_2, arc_top_4, 0, 0, 0, 0, 0},
	{2, arc_top_1, arc_top_5, 0, 0, 0, 0, 0},
	{1, lightning_bolt, 0, 0, 0, 0, 0, 0},
	{2, digit_top_left_a, digit_top_right_a, 0, 0, 0, 0, 0},
	{5, one_top_upper, digit_top_left_b, digit_top_left_f, digit_top_right_b, digit_top_right_f, 0, 0},
	{2, digit_top_left_g, digit_top_right_g, 0, 0, 0, 0, 0},
	{5, one_top_lower, digit_top_left_c, digit_top_left_e, digit_top_right_c, digit_top_right_e, 0, 0},
	{2, digit_top_left_d, digit_top_right_d, 0, 0, 0, 0, 0},
	{2, bar_top_left, bar_top_right, 0, 0, 0, 0, 0},
	{2, cycle_nw, cycle_ne, 0, 0, 0, 0, 0},
	{1, rocket, 0, 0, 0, 0, 0, 0},
	{2, cycle_sw, cycle_se, 0, 0, 0, 0, 0},
	{2, bar_bot_left, bar_bot_right, 0, 0, 0, 0, 0},
	{2, digit_bot_left_a, digit_bot_right_a, 0, 0, 0, 0, 0},
	{5, one_bot_upper, digit_bot_left_b, digit_bot_left_f, digit_bot_right_b, digit_bot_right_f, 0, 0},
	{2, digit_bot_left_g, digit_bot_right_g, 0, 0, 0, 0, 0},
	{5, one_bot_lower, digit_bot_left_c, digit_bot_left_e, digit_bot_right_c, digit_bot_right_e, 0, 0},
	{2, digit_bot_left_d, digit_bot_right_d, 0, 0, 0, 0, 0},
	{1, tear_drop, 0, 0, 0, 0, 0, 0},
	{2, arc_bot_1, arc_bot_5, 0, 0, 0, 0, 0},
	{2, arc_bot_2, arc_bot_4, 0, 0, 0, 0, 0},
	{1, arc_bot_3, 0, 0, 0, 0, 0, 0}
};

/* define 7-segment display groups (i.e. digits) */
const uint8_t seven_segment_displays[NUM_DISPLAYS][3] = {
	{digit_top_right, digit_top_left, one_top},
	{digit_bot_right,	digit_bot_left,	one_bot}
};


void leds_init(void)
{
	/* configure GPIO */
	
	/* enable GPIO port clock(s) */
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	
	/* declare an init structure */
	GPIO_InitTypeDef GPIO_InitStruct = {0};	
	
	/* initialize GPIO Port A LED pins as outputs */
	GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_15;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	/* initialize GPIO Port B LED pins as outputs */
	GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	/* configure Timer 16 */
	/* djjw ... LED display mux timer (600 Hz refresh) - 24MHz/40(prescaler) = 600,000, period of 1000 = 600Hz */

	htim16.Instance               = TIM16;                            /* Select TIM16 */
  htim16.Init.Period            = 1000 - 1;                         /* Auto-reload value */
  htim16.Init.Prescaler         = 40 - 1;                           /* Prescaler of 40 - 1 */
  htim16.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;           /* No clock division */
  htim16.Init.CounterMode       = TIM_COUNTERMODE_UP;               /* Up counting */
  htim16.Init.RepetitionCounter = 1 - 1;                            /* No repetition counting */
  htim16.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;   /* Auto-reload register not buffered */
	
  /* Initialize TIM16 */
  if (HAL_TIM_Base_Init(&htim16) != HAL_OK)
  {
    APP_ErrorHandler();
  }
	
	/* Start the TIM16 Base generation in interrupt mode */
  if (HAL_TIM_Base_Start_IT(&htim16) != HAL_OK)
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
	*	copied from PUYA SDK example
	*
	*	Notes on HAL interrupt plumbing:
	*		- hardware interrupt vector table is mapped in startup_py32f030x8.s
	*		- handler must be placed in py32f0xx_it.c
	*			-> reference example in puya's sdk TIMER1 TIM1_Update_IT example:
	*				 PY32F0xx_Firmware-master\PY32F0xx_Firmware-master\Projects\PY32F030-STK\Example\TIMER1\TIM1_Update_IT\Src
	*		- py32f0xx_hal_tim.c has a weak definition of the callback:
	*			-> __weak void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
	*		- HAL_TIM_PeriodElapsedCallback definition here overrides weak definition
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
	
	/* Constant mapping of cathode activation masks (upper 16-bits of BSRR for Reset) */
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
