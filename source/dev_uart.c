/*
 * debug_uart.cpp
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
 */
#include <stdio.h>
#include "dev_uart.h"
#include "stm32f30x_tools.h"

/* These are used to accelerate the handle of the IRQ instead
 * of using a list to search for the correct device. It's a
 * dirty _shortcut_
 */
static struct dev_uart * dev_uart1 = NULL;
static struct dev_uart * dev_uart2 = NULL;

void dev_uart_add(struct dev_uart * uart)
{
	if (!uart || !uart->port || !uart->uart_buff.rx_buffer_size || !uart->uart_buff.tx_buffer_size) return;

	/* Create buffers */
	uart->uart_buff.rx_buffer = (uint8_t*)malloc(uart->uart_buff.rx_buffer_size);
	uart->uart_buff.tx_buffer = (uint8_t*)malloc(uart->uart_buff.tx_buffer_size);

	/* reset TX */
	uart->uart_buff.tx_int_en = 0;
	uart->uart_buff.tx_length = 0;
	uart->uart_buff.tx_ptr_in = 0;
	uart->uart_buff.tx_ptr_out = 0;
	uart->uart_buff.tx_ready = 0;
	/* reset RX */
	uart->uart_buff.rx_ready = 0;
	uart->uart_buff.rx_ready_tmr = 0;
	uart->uart_buff.rx_ptr_in = 0;

	if (uart->port == USART1) {
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	}
	else if (uart->port == USART2) {
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	}

	if (uart->pins.port == GPIOA) {
		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	}
	else if (uart->pins.port == GPIOB) {
		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
	}
	GPIO_PinAFConfig(uart->pins.port, gpiopin_to_gpiosource(uart->pins.tx_pin), GPIO_AF_7);
	GPIO_PinAFConfig(uart->pins.port, gpiopin_to_gpiosource(uart->pins.rx_pin), GPIO_AF_7);

	GPIO_InitTypeDef GPIO_InitStructure;
	/* TX */
	GPIO_InitStructure.GPIO_Pin = uart->pins.tx_pin | uart->pins.rx_pin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(uart->pins.port, &GPIO_InitStructure);

//	RCC_USARTCLKConfig(RCC_USART2CLK_PCLK);

	USART_DeInit(uart->port);
	/* USART configuration */
	USART_Init(uart->port, &uart->config);

	/*
	 Jump to the USART1_IRQHandler() function
	 if the USART1 receive interrupt occurs
	 */
	USART_ITConfig(uart->port, USART_IT_RXNE, ENABLE); // enable the USART receive interrupt

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	if (uart->port == USART1) {
		uart->nvic.NVIC_IRQChannel = USART1_IRQn;	// we want to configure the USART1 interrupts
		uart->nvic.NVIC_IRQChannelSubPriority = 5;	// this sets the sub-priority inside the group
	}
	else if (uart->port == USART2) {
		uart->nvic.NVIC_IRQChannel = USART2_IRQn;	// we want to configure the USART1 interrupts
		uart->nvic.NVIC_IRQChannelPreemptionPriority = 0;	// this sets the sub-priority inside the group
		uart->nvic.NVIC_IRQChannelSubPriority = 0;
	}
	uart->nvic.NVIC_IRQChannelCmd = ENABLE;	// the USART1 interrupts are globally enabled
	NVIC_Init(&uart->nvic);	// the properties are passed to the NVIC_Init function which takes care of the low level stuff

	/* Enable the USART */
	USART_Cmd(uart->port, ENABLE);

	if (uart->port == USART1)
		dev_uart1 = uart;
	else if (uart->port == USART2)
		dev_uart2 = uart;
}

void dev_uart_set_baud_rate(struct dev_uart * uart, uint32_t baudrate)
{
	uart->config.USART_BaudRate = baudrate;

	/* USART configuration */
	USART_Init(uart->port, &uart->config);
}

void dev_uart_remove(struct dev_uart * uart)
{
	if (uart) {
		/* Remove buffers */
		if (uart->uart_buff.rx_buffer) {
			memset(uart->uart_buff.rx_buffer, 0, uart->uart_buff.rx_buffer_size);
			free(uart->uart_buff.rx_buffer);
		}
		if (uart->uart_buff.tx_buffer) {
			memset(uart->uart_buff.tx_buffer, 0, uart->uart_buff.tx_buffer_size);
			free(uart->uart_buff.tx_buffer);
		}
		uart->nvic.NVIC_IRQChannelCmd = DISABLE;
		USART_ITConfig(uart->port, USART_IT_RXNE, DISABLE);
		NVIC_Init(&uart->nvic);
		USART_Cmd(uart->port, DISABLE);
		USART_DeInit(uart->port);
		if (uart->port == USART1)
			dev_uart1 = NULL;
		else if (uart->port == USART2)
			dev_uart2 = NULL;
	}
}


/**
 * This is a (weak) function in syscalls.c and is used from printf
 * to print data to the UART1
 */
int dev_uart_send(struct dev_uart * uart, int ch)
{
	if ((uart->uart_buff.tx_ptr_in + 1)%uart->uart_buff.tx_buffer_size == uart->uart_buff.tx_ptr_out) {
		return -1;
	}

	uart->uart_buff.tx_length++;
	uart->uart_buff.tx_buffer[uart->uart_buff.tx_ptr_in] = ch;
	uart->uart_buff.tx_ptr_in = (uart->uart_buff.tx_ptr_in + 1)%uart->uart_buff.tx_buffer_size;

	/* If INT is disabled then enable it */
	if (!uart->uart_buff.tx_int_en) {
		uart->uart_buff.tx_int_en = 1;
		USART_ITConfig(uart->port, USART_IT_TXE, ENABLE); 	// enable the USART1 receive interrupt
		USART_ITConfig(uart->port, USART_IT_RXNE, DISABLE);
	}

	return ch;
}


size_t dev_uart_send_buffer(struct dev_uart * uart, uint8_t * buffer, size_t buffer_len)
{
	size_t i = 0;
	for (i=0; i<buffer_len; i++) {
		if (dev_uart_send(uart, buffer[i]) < 0) break;
	}
	return i;
}

void USART1_EXTI25_IRQHandler(void)
{
	if (dev_uart1) dev_uart_irq(dev_uart1);
}


void USART2_EXTI26_IRQHandler(void)
{
	if (dev_uart2) dev_uart_irq(dev_uart2);
}

void dev_uart_update(struct dev_uart * uart)
{
	if (uart->uart_buff.rx_ready) {
		if ((uart->uart_buff.rx_ready_tmr++) >= uart->timeout_ms) {
			uart->uart_buff.rx_ready = 0;
			uart->uart_buff.rx_ready_tmr = 0;
			uart->available = 1;
			if (uart->fp_dev_uart_cb) {
				uart->fp_dev_uart_cb(uart->uart_buff.rx_buffer, uart->uart_buff.rx_ptr_in, 0);
			}
			/* reset RX */
			uart->uart_buff.rx_ptr_in = 0;
		}
	} //:~ rx_ready
}

void dev_uart_irq(struct dev_uart * uart)
{
	if (USART_GetITStatus(uart->port, USART_IT_RXNE) != RESET) {
		/* Read one byte from the receive data register */
		if (uart->uart_buff.rx_ptr_in == uart->uart_buff.rx_buffer_size) {
			uart->port->RDR;	//discard data
			return;
		}
		uart->uart_buff.rx_buffer[uart->uart_buff.rx_ptr_in++] = uart->port->RDR;

		/* Disable the USARTy Receive interrupt */
		/* flag the byte reception */
		uart->uart_buff.rx_ready = 1;
		/* reset receive expire timer */
		uart->uart_buff.rx_ready_tmr = 0;
//		uart->port->SR &= ~USART_FLAG_RXNE;	          // clear interrupt
	}

	if (USART_GetITStatus(uart->port, USART_IT_TXE) != RESET) {
		if (uart->uart_buff.tx_ptr_out != uart->uart_buff.tx_ptr_in) {
			uart->port->TDR = uart->uart_buff.tx_buffer[uart->uart_buff.tx_ptr_out];
			uart->uart_buff.tx_ptr_out = (uart->uart_buff.tx_ptr_out + 1)%uart->uart_buff.tx_buffer_size;
			uart->uart_buff.tx_length--;
		}
		else {
			/* Disable the USARTy Transmit interrupt */
			USART_ITConfig(uart->port, USART_IT_TXE, DISABLE);
			USART_ITConfig(uart->port, USART_IT_RXNE, ENABLE);
			uart->uart_buff.tx_int_en = 0;
			/* Rest uart buffer */
			uart->uart_buff.tx_ptr_in = 0;
			uart->uart_buff.tx_ptr_out = 0;
			uart->uart_buff.tx_length = 0;
		}
//		uart->port->SR &= ~USART_FLAG_TXE;	          // clear interrupt
	}
}


/**
 * This is a (weak) function in syscalls.c and is used from printf
 * to print data to the UART1
 */
int __io_putchar(int ch)
{
	if (dev_uart1 && dev_uart1->debug)
		dev_uart_send(dev_uart1, ch);

	if (dev_uart2 && dev_uart2->debug)
		dev_uart_send(dev_uart2, ch);
	return ch;
}