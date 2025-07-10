#ifndef TTP226_H
#define TTP226_H

#include <stdint.h>
#include "stm32f1xx_ll_rcc.h"
#include "stm32f1xx_ll_gpio.h"
/*
 * DO - PA0, CLK - PA1, RST - PA2, DV - PA3.
 */

void TTP226_Init();
void TTP226_Config(uint32_t DO, uint32_t CLK, uint32_t RST, uint32_t DV);
uint8_t TTP226_Read();

#endif // TTP226_h
