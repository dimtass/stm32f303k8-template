/*
 * dev_uart.h
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
 * Add this line to the interrupt file (e.g. stm32f10x_it.c)
 *		extern struct debug_uart_dev uart_dev;
 *
 * Add this line to the IRQ function (e.g. USART1_IRQHandler):
 * 		dev_uart_irq(&uart_dev);
 *
 * Add this line to the __io_putchar(int ch) function (used for printf)
 * 		dev_uart_send(&uart_dev, ch);
 *
 * Initialize the device in the main.c
 * 		DECLARE_UART_BUFFER(uart_buffer, 64, 16);
 * 		DECLARE_DEBUG_UART_DEV(uart_dev, USART1, 115200, TRACE_LEVEL_DEFAULT, NULL, NULL);
 *		uart_dev.uart_buff = &uart_buffer;
 *		dev_uart_init(&uart_dev);
 */

#ifndef DEV_UART_H_
#define DEV_UART_H_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "stm32f30x.h"
#include "list.h"
#include "comm_buffer.h"

#define DECLARE_UART_DEV(NAME, PORT, PPORT, PTX, PRX, BAUDRATE, BUFFER_SIZE, TIMEOUT_MS, DEBUG, CALLBACK) \
	struct dev_uart NAME = { \
		.port = PORT, \
		.pins = { \
			.port = PPORT, \
			.tx_pin = PTX, \
			.rx_pin = PRX, \
		}, \
		.config = { \
			.USART_BaudRate = BAUDRATE, \
			.USART_WordLength = USART_WordLength_8b, \
			.USART_StopBits = USART_StopBits_1, \
			.USART_Parity = USART_Parity_No, \
			.USART_Mode = USART_Mode_Rx | USART_Mode_Tx, \
			.USART_HardwareFlowControl = USART_HardwareFlowControl_None, \
		}, \
		.uart_buff = { \
			.rx_buffer_size = BUFFER_SIZE, \
			.tx_buffer_size = BUFFER_SIZE, \
		}, \
		.timeout_ms = TIMEOUT_MS, \
		.debug = DEBUG, \
		.fp_dev_uart_cb = CALLBACK, \
	}

/*
 * USART1:
 * 		TX:	PA9, PB6
 * 		RX:	PA10, PB7
 * USART2:
 * 		TX: PA2, PB3, PA14
 * 		RX:	PA3, PA15, PB4
 */
struct dev_uart_pins {
	GPIO_TypeDef * 	port;
	uint16_t		tx_pin;
	uint16_t		rx_pin;
};

/**
 * @brief Callback function definition for reception
 * @param[in] buffer Pointer to the RX buffer
 * @param[in] bufferlen The length of the received data
 */
typedef void (*dev_uart_cb)(uint8_t *buffer, size_t bufferlen, uint8_t sender);

struct dev_uart {
	USART_TypeDef*		port;
	USART_InitTypeDef	config;
	struct dev_uart_pins pins;
	NVIC_InitTypeDef	nvic;
	uint8_t				debug;
	uint8_t				timeout_ms;
	uint8_t				available;
	volatile struct tp_comm_buffer uart_buff;
	/**
	* @brief Callback function definition for reception
	* @param[in] buffer Pointer to the RX buffer
	* @param[in] bufferlen The length of the received data
	*/
	dev_uart_cb fp_dev_uart_cb;
};

/**
 * @brief Initialize the debugging UART interface
 * @param[in] dev_uart A pointer to the UART device
 */
void dev_uart_add(struct dev_uart * uart);

/**
 * @brief Configure the STM32 uart port (including the port pins)
 * @param[in] dev_uart A pointer to the UART device
 */
void* dev_uart_probe(struct dev_uart * dev);

/**
 * @brief Remove the STM32 uart port (including the port pins)
 * @param[in] dev_uart A pointer to the UART device
 */
void dev_uart_remove(struct dev_uart * dev);

/**
 * @brief IRQ handler for the debug interface
 * @param[in] dev_uart A pointer to the UART device
 */
void dev_uart_irq(struct dev_uart * dev);

/**
 * @brief Send a single byte on the debug uart. This should be used with the
 * 		syscalls.c file to implement the printf() function
 * @param[in] dev_uart A pointer to the UART device
 * @param[in] ch The byte to send
 * @return int The byte sent
 */
int dev_uart_send_ch(struct dev_uart * dev, int ch);

size_t dev_uart_send_buffer(struct dev_uart * dev, uint8_t * buffer, size_t buffer_len);

int dev_uart_receive(struct dev_uart * dev);


void dev_uart_set_baud_rate(struct dev_uart * dev, uint32_t baudrate);


/**
 * @brief Poll the RX buffer for new data. If new data are found then
 * 		the fp_debug_uart_cb will be called.
 * @param[in] dev_uart A pointer to the UART device
 */
void dev_uart_update(struct dev_uart * uart);


#endif /* DEV_UART_H_ */
