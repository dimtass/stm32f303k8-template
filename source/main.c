/**
 * Copyright 2018 Dimitris Tassopoulos <dimtass@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 * This is just a template cmake project for the STM32 Nucleo-32 board (MB1180)
*/

#include "stm32f30x.h"
#include "stm32f30x_it.h"
#include "stm32f30x_sysclk.h"
#include "platform_config.h"
#include "dev_timer.h"
#include "dev_uart.h"
#include "dev_led.h"

// #define BENCHMARK_MODE

/* Globals */
struct tp_glb glb;
uint16_t led_cntr = 0;

/* Prototypes */
void setup_dgb_pin();
void print_clocks(RCC_ClocksTypeDef *clocks);
void uart_rx_parser(uint8_t *buffer, size_t bufferlen, uint8_t sender);

/* Peripherals */
/* Init the tiemr list */
static LIST_HEAD(dev_timer_list);
/* Add a LED */
DECLARE_MODULE_LED(led_module, 250);
DECLARE_DEV_LED(led_status, GPIOB, GPIO_Pin_3, &led_module);
/* Add a UART */
DECLARE_UART_DEV(dbg_uart, USART2, GPIOA, GPIO_Pin_2, GPIO_Pin_15, 115200, 1024, 10, 1, uart_rx_parser);


static inline void main_loop(void)
{
	/* 1 ms timer */
	if (glb.tmr_1ms) {
		glb.tmr_1ms = 0;
		/* update the timer object */
		dev_timer_polling(&dev_timer_list);
	}
}

int main(void)
{
	/* Initialize clock, enable safe clocks */
	sysclk_init(SYSCLK_SOURCE_INTERNAL, SYSCLK_72MHz, 1);

	RCC_ClocksTypeDef clocks;
	RCC_GetClocksFreq(&clocks);
	/* Get system frequency */
	SystemCoreClock = clocks.SYSCLK_Frequency;
	if (SysTick_Config(SystemCoreClock / 1000)) {
		/* Capture error */
		while (1);
	}

#ifdef BENCHMARK_MODE
	setup_dgb_pin();
#endif

	/* setup uart port */
	dev_uart_add(&dbg_uart);
	/* set callback for uart rx */
	dbg_uart.fp_dev_uart_cb = uart_rx_parser;
	dev_timer_add((void*) &dbg_uart, 5, (void*) &dev_uart_update, &dev_timer_list);

	/* Initialize led module */
	dev_led_module_init(&led_module);
	/* Attach led module to a timer */
	dev_timer_add((void*) &led_module, led_module.tick_ms, (void*) &dev_led_update, &dev_timer_list);
	/* Add a new led to the led module */
	dev_led_init(&led_status);
	dev_led_set_pattern(&led_status, 0b00001111);

	set_trace_level(
			0
			| TRACE_LEVEL_DEFAULT
			,1);

	printf("Program started\n");
	printf("Freq: %lu\n", clocks.SYSCLK_Frequency);
	print_clocks(&clocks);

	while(1) {
#ifdef BENCHMARK_MODE
		GPIOB->ODR ^= GPIO_Pin_4;
#else
		main_loop();
#endif
	}
}

void uart_rx_parser(uint8_t *buffer, size_t bufferlen, uint8_t sender)
{
	buffer[bufferlen-1] = 0;
	TRACE(("Rx: %s\n", buffer));
	if (!strncmp((char*)buffer, "TEST", 4)) {
		TRACE(("\t>TEST\n"));
	}
}




void setup_dgb_pin()
{
	GPIO_InitTypeDef led;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
	led.GPIO_Pin = GPIO_Pin_4;
	led.GPIO_Mode = GPIO_Mode_OUT;
	led.GPIO_Speed = GPIO_Speed_50MHz;
	led.GPIO_OType = GPIO_OType_PP;
	led.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB, &led);

	GPIOB->ODR &= ~GPIO_Pin_4;
}

void print_clocks(RCC_ClocksTypeDef *clocks)
{
	TRACE((
			"Clocks:\n"
			"  HCLK: %lu\n"
			"  SYSCLK: %lu\n"
			"  PCLK1: %lu\n"
			"  PCLK2: %lu\n"
			"  HRTIM1CLK: %lu\n"
			"  ADC12CLK: %lu\n"
			"  ADC34CLK: %lu\n"
			"  USART1CLK: %lu\n"
			"  USART2CLK: %lu\n"
			"  USART3CLK: %lu\n"
			"  UART4CLK: %lu\n"
			"  UART5CLK: %lu\n"
			"  I2C1CLK: %lu\n"
			"  I2C2CLK: %lu\n"
			"  I2C3CLK: %lu\n"
			"  TIM1CLK: %lu\n"
			"  TIM2CLK: %lu\n"
			"  TIM3CLK: %lu\n"
			"  TIM8CLK: %lu\n"
			"  TIM15CLK: %lu\n"
			"  TIM16CLK: %lu\n"
			"  TIM17CLK: %lu\n"
			"  TIM20CLK: %lu\n"
			,
			clocks->HCLK_Frequency,
			clocks->SYSCLK_Frequency,
			clocks->PCLK1_Frequency,
			clocks->PCLK2_Frequency,
			clocks->HRTIM1CLK_Frequency,
			clocks->ADC12CLK_Frequency,
			clocks->ADC34CLK_Frequency,
			clocks->USART1CLK_Frequency,
			clocks->USART2CLK_Frequency,
			clocks->USART3CLK_Frequency,
			clocks->UART4CLK_Frequency,
			clocks->UART5CLK_Frequency,
			clocks->I2C1CLK_Frequency,
			clocks->I2C2CLK_Frequency,
			clocks->I2C3CLK_Frequency,
			clocks->TIM1CLK_Frequency,
			clocks->TIM2CLK_Frequency,
			clocks->TIM3CLK_Frequency,
			clocks->TIM8CLK_Frequency,
			clocks->TIM15CLK_Frequency,
			clocks->TIM16CLK_Frequency,
			clocks->TIM17CLK_Frequency,
			clocks->TIM20CLK_Frequency
			));
}