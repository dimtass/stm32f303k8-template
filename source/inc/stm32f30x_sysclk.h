/*
 * stm32f30x_sysclk.h
 *
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
 * 
 * Note:
 * Use this to set clocks or overclock the MCU
 */

#ifndef STM32F30X_SYSCLK_H_
#define STM32F30X_SYSCLK_H_

#include "stm32f30x.h"

enum en_sysclk_source {
	SYSCLK_SOURCE_INTERNAL,
	SYSCLK_SOURCE_EXTERNAL
};

enum en_sysclk {
	SYSCLK_16MHz = 0,
	SYSCLK_32MHz,
	SYSCLK_64MHz,
	SYSCLK_72MHz,
	SYSCLK_128MHz
};

static inline void sysclk_init(enum en_sysclk_source clksrc, enum en_sysclk freq, uint8_t safe_clks)
{
	RCC_DeInit();
	uint32_t pll_src = RCC_PLLSource_PREDIV1;

	/* Select source */
	switch(clksrc) {
	case SYSCLK_SOURCE_INTERNAL:
		RCC_HSEConfig(RCC_HSE_OFF);
		RCC_HSICmd(ENABLE);
		pll_src = RCC_PLLSource_HSI;
		break;
	case SYSCLK_SOURCE_EXTERNAL:
		RCC_HSEConfig(RCC_HSE_ON);
		RCC_HSICmd(DISABLE);
		break;
	default:
		return;
	};
	while (RCC_GetFlagStatus(RCC_FLAG_HSIRDY) == RESET);

	/* Enable Prefetch Buffer */
	FLASH_PrefetchBufferCmd(ENABLE);
	if (freq >= SYSCLK_64MHz) {
		FLASH_SetLatency(FLASH_Latency_2);
	}

	/* Set peripheral clocks */
	if (freq >= SYSCLK_64MHz)
		RCC_PCLK1Config(RCC_HCLK_Div2);
	else
		RCC_PCLK1Config(RCC_HCLK_Div1);

	if (safe_clks) {
		switch(freq) {
		case SYSCLK_72MHz:
			RCC_PCLK1Config(RCC_HCLK_Div2);
			break;
	 	case SYSCLK_128MHz:
			RCC_PCLK1Config(RCC_HCLK_Div4);
			RCC_PCLK2Config(RCC_HCLK_Div2);
			break;
		default:
			RCC_HCLKConfig(RCC_SYSCLK_Div1);
			RCC_PCLK2Config(RCC_HCLK_Div1);
			break;
		};
	}

	uint32_t pllmul;
	switch(freq) {
		case SYSCLK_16MHz:
			pllmul = RCC_PLLMul_2;
			break;
		case SYSCLK_32MHz:
			pllmul = RCC_PLLMul_4;
			break;
		case SYSCLK_64MHz:
			pllmul = RCC_PLLMul_8;
			break;
		case SYSCLK_72MHz:
			pllmul = RCC_PLLMul_9;
			break;
		case SYSCLK_128MHz:
			pllmul = RCC_PLLMul_16;
			break;
		}
	RCC_PLLConfig(pll_src, pllmul);

	RCC_PLLCmd(ENABLE);
	while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);

	RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
	/* Wait till PLL is used as system clock source */
	while(RCC_GetSYSCLKSource() != 0x08)
	{;}
}


#endif /* STM32F30X_SYSCLK_H_ */
