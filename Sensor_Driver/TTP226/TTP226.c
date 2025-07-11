#include "TTP226.h"

void TTP226_Init(void)
{
    TTP226_Config();
}
void TTP226_Config()
{
    TTP226_GPIO_Config();
    TTP226_EXTI_Config();
}

void TTP226_Config(void)
{
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
  
    /* GPIO Ports Clock Enable */
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOA);
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_AFIO);    // Kích hoạt xung nhịp cho AFIO

    /*     GPIO Configiratopm     */  
    
    /* Reset OUTPUT Pin*/ 
    LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_1);
    LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_2);
    LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_3);    
    
    /* PA0 (DO) | PA3 (DV) - Input pull-up */
    GPIO_InitStruct.Pin     = LL_GPIO_PIN_0 | LL_GPIO_PIN_3;
    GPIO_InitStruct.Mode    = LL_GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull    = LL_GPIO_PULL_UP;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* PA1 (CLK) | PA2 (RST) - Output push-pull */
    GPIO_InitStruct.Pin        = LL_GPIO_PIN_1 | LL_GPIO_PIN_2;
    GPIO_InitStruct.Mode       = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Speed      = LL_GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Pull       = LL_GPIO_PULL_NO;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}

void TTP226_EXTI_Config()
{
    // Liên kết EXIT Line 0 với Pin PA0
    LL_GPIO_AF_SetEXTISource(LL_GPIO_AF_EXTI_PORTA, LL_GPIO_AF_EXTI_LINE0);

    // Cấu hình EXIT Line 0
    LL_EXTI_InitTypeDef EXTI_InitStruct = {0};
    EXTI_InitStruct.Line_0_31      = LL_EXTI_LINE_0;
    EXTI_InitStruct.LineCommand    = ENABLE;
    EXTI_InitStruct.Mode           = LL_EXTI_MODE_IT; // Interrupt mode
    EXTI_InitStruct.Trigger        = LL_EXTI_TRIGGER_FALLING;
    LL_EXTI_Init(&exti_init);

    // Bật NVIC cho EXTI0
    NVIC_SetPriority(EXTI0_IRQn, 0);
    NVIC_EnableIRQ(TTP226_ReadData_EXTI0_IRQHandler);
}

uint8_t TTP226_ReadData_EXTI0_IRQHandler(void)
{
  
}

