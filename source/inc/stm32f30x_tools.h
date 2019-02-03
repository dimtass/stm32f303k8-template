#ifndef STM32F30X_SYSCLK_H_
#define STM32F30X_SYSCLK_H_

/**
 * stm32f30x_tools.h
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
 */

#include "stm32f30x.h"

static inline uint8_t gpiopin_to_gpiosource(uint16_t gpiopin)
{
    switch(gpiopin) {
        case GPIO_Pin_0:
            return GPIO_PinSource0;
        case GPIO_Pin_1:
            return GPIO_PinSource1;
        case GPIO_Pin_2:
            return GPIO_PinSource2;
        case GPIO_Pin_3:
            return GPIO_PinSource3;
        case GPIO_Pin_4:
            return GPIO_PinSource4;
        case GPIO_Pin_5:
            return GPIO_PinSource5;
        case GPIO_Pin_6:
            return GPIO_PinSource6;
        case GPIO_Pin_7:
            return GPIO_PinSource7;
        case GPIO_Pin_8:
            return GPIO_PinSource8;
        case GPIO_Pin_9:
            return GPIO_PinSource9;
        case GPIO_Pin_10:
            return GPIO_PinSource10;
        case GPIO_Pin_11:
            return GPIO_PinSource11;
        case GPIO_Pin_12:
            return GPIO_PinSource12;
        case GPIO_Pin_13:
            return GPIO_PinSource13;
        case GPIO_Pin_14:
            return GPIO_PinSource14;
        case GPIO_Pin_15:
            return GPIO_PinSource15;
    };
    return 0;
}
#endif