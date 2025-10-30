#ifndef TTP226_H
#define TTP226_H

#include <stdint.h>
#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_system.h"
#include "stm32f1xx_ll_utils.h"
#include "stm32f1xx_ll_gpio.h"

// Delay Library
#include "stm32f1xx_delay.h"
/*
 * DO - PA0, CLK - PA1, RST - PA2, DV - PA3.
 */

void TTP226_Init();
void TTP226_Config();
void TTP226_GPIO_Config();
void TTP226_EXTI_Config();
uint8_t TTP226_Read();

#endif // TTP226_h
