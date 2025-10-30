#ifndef DS18B20_H
#define DS18B20_H

#include "OneWire.h"
#include <stdbool.h>
#include <stdint.h>

// Error Codes
#define DEVICE_DISCONNECTED_C   -127.0f
#define DEVICE_DISCONNECTED_F   -196.6f
#define DEVICE_DISCONNECTED_RAW -7040

// OneWire commands
#define STARTCONVERT    0x44  // Tells device to take a temperature reading
#define COPYSCRATCH     0x48  // Copy scratchpad to EEPROM
#define READSCRATCH     0xBE  // Read from scratchpad
#define WRITESCRATCH    0x4E  // Write to scratchpad
#define RECALLSCRATCH   0xB8  // Recall from EEPROM to scratchpad
#define READPOWERSUPPLY 0xB4  // Determine if device needs parasite power
#define ALARMSEARCH     0xEC  // Query bus for devices with an alarm condition

// Index của array Scratchpad[] 
#define TEMP_LSB        0
#define TEMP_MSB        1
#define HIGH_ALARM_TEMP 2
#define LOW_ALARM_TEMP  3
#define CONFIGURATION   4
#define INTERNAL_BYTE   5
#define COUNT_REMAIN    6
#define COUNT_PER_C     7
#define SCRATCHPAD_CRC  8

// Device resolution
#define TEMP_9_BIT      0x1F
#define TEMP_10_BIT     0x3F
#define TEMP_11_BIT     0x5F
#define TEMP_12_BIT     0x7F


#define constrain(amount, low, high) ((amount)<(low)?(low) : (amount)>(high)?(high) : (amount)) 

typedef struct DS18B20_Device
{   
    uint32_t Pin;
    uint8_t bitResolution;                  
    bool    autoSaveScratchPad;
    bool    waitForConversion;
    bool    checkForConversion; 
    uint8_t deviceAddress[8];               // ROM CODE - Chưa sử dụng
    uint8_t ScratchPad[9];                  // Memory
} DS18B20;

void DS18B20_init();

// Device Information
bool DS18B20_isConnected(uint8_t* scratchPad);

// Scratchpad Operations
bool DS18B20_readScratchPad(uint8_t* scratchPad);
void DS18B20_writeScratchPad(const uint8_t* scratchPad);
bool DS18B20_readPowerSupply();

// Resolution Control
uint8_t DS18B20_getResolution();
bool    DS18B20_setResolution(uint8_t newResolution);

// Conversion Status
bool     DS18B20_isConversionComplete(void);
uint16_t DS18B20_millisToWaitForConversion(uint8_t bitResolution);


// EEPROM Operations
bool DS18B20_saveScratchPad();
bool DS18B20_recallScratchPad();

// Temperature Operations
bool  DS18B20_requestTemperatures(void);
float DS18B20_getTempC();
float DS18B20_getTempF();
float DS18B20_getTemp();

// Conversion Completion Methods
void DS18B20_blockTillConversionComplete(uint8_t bitResolution, bool checkForConversion);

// Internal Methods
int32_t DS18B20_calculateTemperature(uint8_t* scratchPad);
bool    DS18B20_isAllZeros(const uint8_t* const scratchPad, const size_t length);

#endif // DS18B20_H
