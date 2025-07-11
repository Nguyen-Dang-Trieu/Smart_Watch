#include "TTP226.h"

void TTP226_Init(void)
{
    TTP226_Config();
    
    LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_1);
}
void TTP226_Config()
{
    TTP226_GPIO_Config();
    TTP226_EXTI_Config();
}

void TTP226_GPIO_Config(void)
{
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
  
    /* GPIO Ports Clock Enable */
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOA);
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_AFIO);    // Kích hoạt xung nhịp cho AFIO

    /* === GPIO Configiratopm === */  
    
    /* Reset OUTPUT Pin*/ 
    LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_1);
    LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_2);
  
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
    // Liên kết EXIT Line 3 với Pin PA3
    LL_GPIO_AF_SetEXTISource(LL_GPIO_AF_EXTI_PORTA, LL_GPIO_AF_EXTI_LINE3);

    // Cấu hình EXIT Line 3
    LL_EXTI_InitTypeDef EXTI_InitStruct = {0};
    EXTI_InitStruct.Line_0_31      = LL_EXTI_LINE_3;
    EXTI_InitStruct.LineCommand    = ENABLE;                  // Active EXTI
    EXTI_InitStruct.Mode           = LL_EXTI_MODE_IT;         // Interrupt mode
    EXTI_InitStruct.Trigger        = LL_EXTI_TRIGGER_FALLING;
    LL_EXTI_Init(&EXTI_InitStruct);

    // Bật NVIC cho EXTI3
    NVIC_SetPriority(EXTI3_IRQn, 0);
    NVIC_EnableIRQ(EXTI3_IRQn);
}

uint8_t TTP226_ReadData(void)
{
    uint8_t Data = 0;

    /* Reset TTP226 */
    LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_2);
    LL_mDelay(62); // 62 us
    LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_2);
    LL_mDelay(62); // 62 us

    /* Read data from TTP226 (D0 -> D7) */
    for(uint8_t i = 0; i < 8; i++)
    {
        LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_1);
        LL_mDelay(62); // 62 us

        Data >>= 1;
        if(LL_GPIO_Read(GPIOA, LL_GPIO_PIN_0) == HIGH)
        {
            Data |= 0x80;    
        }

        LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_1);
        LL_mDelay(62); // 62 us
    }

    return Data;
}

void EXTI3_IRQHandler(void)
{
    // 1. Check Interrtup Flag
    if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_3) != RESET)
    {
        // 2. Clear Interrupt Flag
        LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_3);
        
        // 3. Interrup Handling
        TTP226_ReadData();
    }
}
