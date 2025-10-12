#ifndef _I2C1_H
#define _I2C1_H

#include "main.h"


/**
  * @brief I2C devices settings
  */
/* I2C SPEEDCLOCK define to value: Stardard Mode @100kHz */
#define I2C_SPEEDCLOCK           100000
#define I2C_DUTYCYCLE            LL_I2C_DUTYCYCLE_2

/**
  * @brief  This function configures I2C1 in Master mode.
  * @note   This function is used to :
  *         -1- Enables GPIO clock.
  *         -2- Enable the I2C1 peripheral clock and configures the I2C1 pins.
  *         -3- Configure I2C1 functional parameters.
  * @note   Peripheral configuration is minimal configuration from reset values.
  * @param  None
  * @retval None
  */
void I2C1_Init(void);


/**
  * @brief  This function Activate I2C1 peripheral (Master)
  * @note   This function is used to :
  *         -1- Enable I2C1.
  * @param  None
  * @retval None
  */
void I2C1_Activate(void);


#endif // _I2C1_H