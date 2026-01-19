#ifndef H_STM32F1xx_DELAY_H
#define H_STM32F1xx_DELAY_H

#include <stdint.h>
#include "stm32f1xx.h"
#include "stm32f1xx_ll_tim.h"
#include "stm32f1xx_ll_bus.h"

void LL_TimerDelay_Init();
void LL_delay_us(uint16_t value_us);
void LL_delay_ms(uint16_t value_ms);

#endif // H_STM32F1xx_DELAY_H