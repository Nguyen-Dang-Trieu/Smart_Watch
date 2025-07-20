#ifndef ONE_WIRE_H
#define ONE_WIRE_H

#include <stdint.h>

#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_system.h"
#include "stm32f1xx_ll_utils.h"
#include "stm32f1xx_ll_gpio.h"

#include "stm32f1xx_delay.h"

#ifndef ONEWIRE_CRC
#define ONEWIRE_CRC 1
#endif

#ifndef ONEWIRE_CRC8_TABLE
#define ONEWIRE_CRC8_TABLE 1
#endif

void OneWrite_Init(void);
uint8_t OneWire_Reset(void);

// Issue a 1-Wire rom skip command, to address all on bus.
void OneWire_Skip(void);

void OneWire_WriteBit(uint8_t bit);                                          // Write a bit
void OneWire_Write(uint8_t byte);                                            // Write a byte
void OneWire_WriteBytes(const uint8_t *buf, uint16_t count, bool power = 0); // Write bytes

uint8_t OneWire_ReadBit(void);                        // Read a bit
uint8_t OneWire_Read(void);                           // Read a byte
void OneWire_ReadBytes(uint8_t *buf, uint16_t count); // Read bytes

#if ONEWIRE_CRC
uint8_t OneWire_crc8(const uint8_t *addr, uint8_t len);
#endif  // ONEWIRE_CRC

#endif // ONE_WIRE_H
