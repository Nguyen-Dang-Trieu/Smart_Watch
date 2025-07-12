#ifndef TIMER_DELAY_H
#define TIMER_DELAY_H

#include <stdint.h>

void LL_TimerDelay_Init();
void LL_delay_us(uint16_t value);
void LL_delay_ms(uint16_t value);

#endif // TIMER_DELAY_H
