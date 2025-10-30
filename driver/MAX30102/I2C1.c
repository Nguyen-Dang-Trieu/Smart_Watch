#include "I2C1.h"

void I2C1_Init(void)
{
	LL_RCC_ClocksTypeDef rcc_clocks;

  /* (1) Enables GPIO clock  **********************/

  /* Enable the peripheral clock of GPIOB */
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOB);

  /* (2) Enable the I2C1 peripheral clock *************************************/

  /* Enable the peripheral clock for I2C1 */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C1);

  /* Configure SCL Pin as : Alternate function, High Speed, Open drain, Pull up */
  LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_6, LL_GPIO_MODE_ALTERNATE);  							// Chuc nang ngoai vi
  LL_GPIO_SetPinSpeed(GPIOB, LL_GPIO_PIN_6, LL_GPIO_SPEED_FREQ_HIGH);
  LL_GPIO_SetPinOutputType(GPIOB, LL_GPIO_PIN_6, LL_GPIO_OUTPUT_OPENDRAIN);
  LL_GPIO_SetPinPull(GPIOB, LL_GPIO_PIN_6, LL_GPIO_PULL_UP);

  /* Configure SDA Pin as : Alternate function, High Speed, Open drain, Pull up */
  LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_7, LL_GPIO_MODE_ALTERNATE);				// Chuc nang ngoai vi
  LL_GPIO_SetPinSpeed(GPIOB, LL_GPIO_PIN_7, LL_GPIO_SPEED_FREQ_HIGH);
  LL_GPIO_SetPinOutputType(GPIOB, LL_GPIO_PIN_7, LL_GPIO_OUTPUT_OPENDRAIN);
  LL_GPIO_SetPinPull(GPIOB, LL_GPIO_PIN_7, LL_GPIO_PULL_UP);

  /* (3) Configure I2C1 functional parameters ********************************/
  
  /* Disable I2C1 prior modifying configuration registers */
  LL_I2C_Disable(I2C1);
  
  /* Retrieve Clock frequencies */
  LL_RCC_GetSystemClocksFreq(&rcc_clocks);

  /* Configure the SCL Clock Speed */
  LL_I2C_ConfigSpeed(I2C1, rcc_clocks.PCLK1_Frequency, I2C_SPEEDCLOCK, I2C_DUTYCYCLE);
}

void I2C1_Activate(void)
{
  /* (1) Enable I2C1 */
  LL_I2C_Enable(I2C1);
}
