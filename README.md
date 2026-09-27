# Geek Bar Pulse Hardware/Firmware Hack
Hijack/repurpose of Geek Bar Pulse Puya PY32F030K28 Cortex-M0

Description: Reverse engineering and repurposing of Geek Bar Pulse assets (MCU, LED display, USB-C charging, Lithium Ion battery)

## Hardware:
- MCU:
	- Puya PY32f030EK28 Cortex-M0
	- 64k Flash, 8k SRAM, 32-QFN 5x5 package
	- https://www.py32.org/en/mcu/PY32F030xx.html
- 53-LED multiplexed display
- 16 perimeter RGB LEDs
- LP4068 800 mA single-chip linear charger for single-cell Li-ion battery
- USB-C charging port (5.1k pull-downs on CC1/CC2 for USB-C PD)
	- SWD interface (SWDIO, SWCLK) routed to CC1, CC2 for external flash programming



## IDE/SDK:
- Keil MDK-ARM v5.43a uVision 5
	- Puya microcontrollers natively supported in latest version
- Puya PY32F0xx_Firmware-master SDK
	- HAL/LL drivers, code examples, device templates, start-up files
		- https://github.com/OpenPuya/PY32F0xx_Firmware
- Debug probes:
	- Segger J-Link Plus + Segger J-Link Commander v9.78, J-Flash v9.78
		- Closely integrated with Keil uVision (works reliably for programming and debug)
		- J-Flash seemed to be most reliable tool for erasing chip
		- J-Link Commander (command line) also used to attempt capture of the core on a sealed unit
	- ST-Linkv2 (not tested or used)
	- RP2040 Pi Pico (debugprobe_on_pico.uf2) + OpenOCD
		- https://github.com/raspberrypi/debugprobe/releases
		- https://www.visualmicro.com/page/Creating-a-Raspberry-Pi-Debug-Probe-(CMSIS-DAP).aspx
	

## Wiring:
- SWD interface exposed on USB-C charging port (CC1/CC2)
- SWDIO - PA13, SWCLK - PA14
- NOTE -> Normal state of unit is in deep sleep (or stop), SWD interface is normally unavailable. Slide switch wakes unit and provides small connect window. Can also plug in charger to wake uinit and open another connect window/opportunity.
- See schematic for feature element mapping (i.e. RGB LEDs, display, etc)
	- https://github.com/schlae/VapeRE/tree/main/BD0027
- Perimeter RGB LEDs:
	- Red - PA5
	- Green - PA4
	- Blue - PA2
- Charge indicator - PF1
- Slide switch - PA7 (pulls low in one position, high in remaining two)
- LED matrix (53 display elements/LEDs)
	- Refer to leds.h for mapping

## Peripheral setup:
- LED matrix timer - TIM1 (auto-reload mode, 1.6 ms interrupt)
- Application time-out timer - TIM2 (oneshot mode, 10s interrupt)
- Low power timer - LPTIM (? interrupt)
- UART - USART1 ... re-purpose SWD via AF registers:
	- Rx = PA13 (AF8), Tx = PA14 (AF1)


 
