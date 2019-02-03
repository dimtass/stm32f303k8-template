/*
 * led_pattern.c
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
#include "dev_led.h"

#define LED_EXISTS(LED, ITTERATOR) ( (LED->port == ITTERATOR->port) && (LED->pin == ITTERATOR->pin) )

void dev_led_module_init(struct dev_led_module * dev)
{
	INIT_LIST_HEAD(&dev->led_list);
}


static inline struct dev_led * dev_led_find_led(struct dev_led_module * dev, struct dev_led * led)
{
	if (!dev || !led) return NULL;
	if (!list_empty(&dev->led_list)) {
		struct dev_led * led_it = NULL;
		list_for_each_entry(led_it, &dev->led_list, list) {
			if LED_EXISTS(led, led_it) {
				/* found */
				return(led_it);
			}
		}
	}
	return NULL;
}


void dev_led_init(struct dev_led * led)
{
	struct dev_led_module * dev = led->owner;
	/* do not allow duplicates */
	if (dev_led_find_led(dev, led)) {
		return;
	}

	/* init dev head list */
	INIT_LIST_HEAD(&led->list);
	/* Add to led_list */
	list_add(&led->list, &dev->led_list);

	if (led->port == GPIOA) {
		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	}
	else if (led->port == GPIOB) {
		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
	}
	else if (led->port == GPIOF) {
		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOF, ENABLE);
	}

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;
	/* Configure LED */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Pin = led->pin;
	GPIO_Init(led->port, &GPIO_InitStructure);
	led->port->ODR &= ~led->pin;	//set to 0

	dev_led_set_pattern(led, led->pattern);
	return;
}


void dev_led_remove(struct dev_led * led)
{
	struct dev_led_module * dev = led->owner;
	struct dev_led * found_led = dev_led_find_led(dev, led);
	if (found_led) {
		list_del(&found_led->list);
		found_led->port->BRR = found_led->pin;	//set to 0
	}
}


void dev_led_set_pattern(struct dev_led * led, int pattern)
{
	struct dev_led_module * dev = led->owner;
	struct dev_led * found_led = dev_led_find_led(dev, led);
	if (found_led)
		found_led->pattern = (uint8_t) pattern;
}


void dev_led_update(struct dev_led_module * dev)
{
	if (!list_empty(&dev->led_list)) {
		struct dev_led * led_it;
		list_for_each_entry(led_it, &dev->led_list, list) {
			if (led_it->pattern & (1 << dev->led_pattern_index) )
				led_it->port->ODR |= led_it->pin;
			else
				led_it->port->ODR &= ~led_it->pin;
			if ((++dev->led_pattern_index) == 8)
				dev->led_pattern_index = 0;
		}
	}
}

