#include "stm32f1xx_delay.h"

// Tham khảo trang web để có thể tạo delay :https://deepbluembedded.com/stm32-delay-microsecond-millisecond-utility-dwt-delay-timer-delay/ 

// Đang ở chế độ busy wait.

void LL_TimerDelay_Init()
{
  // Bật clock cho TIMER 4
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM4);

  // Reset TIMER
  LL_TIM_DeInit(TIM4);

  // Cấu hình Prescaler để tạo 1 MHz. Nếu Clock Timer = 72 MHz / 72 = 1 MHz
  // => Prescaler = 71
  LL_TIM_SetPrescaler(TIM4, 71);

  // Cấu hình ARR
  LL_TIM_SetAutoReload(TIM4, 0xFFFF);

  // Counter Up
  LL_TIM_SetCounterMode(TIM4, LL_TIM_COUNTERMODE_UP);

  // Enable Timer
  LL_TIM_EnableCounter(TIM4);
  
  
}

void LL_delay_us(uint16_t value_us)
{
  LL_TIM_SetCounter(TIM4, 0);
  while (LL_TIM_GetCounter(TIM4) < value_us);
}

void LL_delay_ms(uint16_t value_ms)
{
  while (value_ms--)
  {
    LL_delay_us(1000);  // mỗi lần 1000 µs = 1 ms
  }
}
