#include "TTP226.h"

void TTP226_Init(void)
{
    TTP226_Config();
}
void TTP226_Config(void)
{
  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
  
  /* GPIO Ports Clock Enable */
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOA);

  /*     GPIO Configiratopm     */  
  /* PA0 (DO) | PA3 (DV) - Input pull-up */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_0 | LL_GPIO_PIN_3;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* PA1 (CLK) | PA2 (RST) - Output push-pull */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_1 | LL_GPIO_PIN_2;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Speed      = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Pull       = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* Reset OUTPUT Pin*/ 
  LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_0);
  LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_1);
  LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_2);
  LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_3);
}

uint8_t TTP226_Read(void)
{
  
}

