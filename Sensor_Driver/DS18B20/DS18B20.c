#include "DS18B20.h"

void DS18B20_Init()
{
    OneWrite_Init();
}

// Device Information
/**
 * @brief Kiểm tra xem cảm biến DS18B20 có đang kết nối và phản hồi dữ liệu hợp lệ không
 * @param scratchPad Bộ nhớ 9 byte chứa dữ liệu từ DS18B20
 * @return true nếu
 *         - Đọc được scratchpad thành công
 *         - Dữ liệu không toàn số 0
 *         - CRC đúng (scratchPad[8] == CRC(scratchPad[0..7]))
 */
bool DS18B20_isConnected(uint8_t* scratchPad)
{
    bool temp_data = DS18B20_readScratchPad(scratchPad);
    return temp_data && !DS18B20_isAllZeros(scratchPad, 9) && OneWire_crc8(scratchPad, 8) == scratchPad[SCRATCHPAD_CRC]);
}


/* ---- ---- ---- Scratchpad Operations ---- ---- ---- */
bool DS18B20_readScratchPad(uint8_t *scratchPad)
{
    uint8_t presence = OneWire_Reset();
    if (!presence)
        return false; // Kiểm tra sự hiện diện của thiết bị.

    OneWire_Skip();

    OneWire_Write(READSCRATCH);
    for (int i = 0; i < 9; i++)
    {
        scratchPad[i] = OneWire_Read();
    }

    /* Viết thêm code CRC8 ở đây nếu cần độ tin cậy cao */
    // Code CRC8
    // -----------

    presence = OneWire_Reset();
    return (presence == 1); // Toán tử quan hệ trong c
}

void DS18B20_writeScratchPad(const uint8_t *scratchPad, bool autoSaveScratchPad)
{
    OneWire_Reset();

    OneWire_Skip();

    OneWire_Write(WRITESCRATCH);

    OneWire_Write(scratchPad[HIGH_ALARM_TEMP]); // TH register
    OneWire_Write(scratchPad[LOW_ALARM_TEMP]);  // TL register
    OneWire_Write(scratchPad[CONFIGURATION]);   // Configuration register

    // Lưu giá trị các thanh ghi TH, TL, Config từ ScratchPad vào trong EEPROM
    if (autoSaveScratchPad)
    {
        saveScratchPad();
    }
    else
    {
        OneWire_Reset();
    }
}

bool DS18B20_readPowerSupply()
{
    bool parasiteMode = false; // nguồn kí sinh

    OneWire_Reset();

    OneWire_Skip(); // Chỉ 1 device , mở rộng dùng Match ROM (0x55) nếu nhiều device

    OneWire_Write(READPOWERSUPPLY);
    // Kiểm tra Parasite Power Mode - bit = 0 / External Power - bit = 1
    if (OneWire_ReadBit() == 0)
        parasiteMode = true;

    OneWire_Reset();

    return parasiteMode;
}

/* ---- ---- ---- ---- Resolution Control ---- ---- ---- ---- */
uint8_t DS18B20_getResolution()
{
    /uint8_t scratchPad[9];
    if(DS18B20_isConnected(scratchPad))
    {
        switch (scratchPad[CONFIGURATION]) {
            case TEMP_12_BIT: return 12;
            case TEMP_11_BIT: return 11;
            case TEMP_10_BIT: return 10;
            case TEMP_9_BIT:  return 9;
        }
    }
    return 0;
}

bool DS18B20_setResolution(uint8_t newResolution)
{
    bool success = false;
    uint8_t bitResolution = constrain(newResolution, 9, 12);

    uint8_t newValue = 0;
    uint8_t scratchPad[9];

    if(DS18B20_isConnected(scratchPad))
    {
        switch (newResolution) {
            case 12: newValue = TEMP_12_BIT; break;
            case 11: newValue = TEMP_11_BIT; break;
            case 10: newValue = TEMP_10_BIT; break;
            case 9:  newValue = TEMP_9_BIT;  break;
        }

        if (scratchPad[CONFIGURATION] != newValue) {
            scratchPad[CONFIGURATION] = newValue;
            DS18B20_writeScratchPad(scratchPad);
        }
        success = true;
    }

    return success;
     
}

/* ---- ---- ---- ---- End Resolution Control ---- ---- ---- ---- */



/* ---- ---- ---- Conversion Status ---- ---- ---- */
bool DS18B20_isConversionComplete(void)
{
    uint8_t bit = OneWire_ReadBit();

    return (bit == 1); // DS18B20 gửi bit = 1 khi xong quá trình chuyển đổi T.
}

uint16_t DS18B20_millisToWaitForConversion(uint8_t bitResolution)
{
    switch (bitResolution)
    {
    case 9:  return 94;
    case 10: return 188;
    case 11: return 375;
    default: return 750;
    }
}

/* ---- ---- ---- EEPROM Operations ---- ---- ---- */
bool DS18B20_saveScratchPad()
{
    if (OneWire_Reset() == 0)
        return false;

    OneWire_Skip();

    OneWire_Write(COPYSCRATCH);

    /**
     * Specification: NV Write Cycle Time is typically 2ms, max 10ms
     * Waiting 20ms to allow for sensors that take longer in practice
     */
    LL_delay_ms(20);

    return (OneWire_Reset() == 1);
}

bool DS18B20_recallScratchPad()
{
    if (OneWire_Reset() == 0)
        return false;

    OneWire_Skip();

    OneWire_Write(RECALLSCRATCH);

    /**
     * Master nhận bit = 0  - đang recall
     * Master nhận bit = 1 - quá trình recall hoàn tất.
     */
    while (OneWire_ReadBit() == 0)
    {
        LL_delay_ms(1);

        // Thêm phần xử lí timeout
    };

    return (OneWire_Reset() == 1);
}

/* ---- ---- ---- Temperature Operations ---- ---- ---- */
bool DS18B20_requestTemperatures(uint8_t bitResolution)
{
    if (OneWire_Reset() == 0)
        return false;

    OneWire_Skip();

    OneWire_Write(STARTCONVERT);

    // 1. Phương thức: Non-Blocking
    // if(!waitForConversion) return true;
    

    // 2. Phương thức: Blocking
    DS18B20_blockTillConversionComplete(bitResolution); 

    return true;
}

float DS18B20_getTempC()
{
    float Temp_C = DS18B20_getTemp();

    if(Temp_C < DEVICE_DISCONNECTED_C)
        return DEVICE_DISCONNECTED_C;
    else
        return Temp_C;
}

float DS18B20_getTempF()
{
    float Temp_F = DS18B20_getTempC() * 1.8f + 32.0f;
    if(Temp_F == DEVICE_DISCONNECTED_F)
        return DEVICE_DISCONNECTED_F;
    else 
        return Temp_F;
}

float DS18B20_getTemp()
{
    uint8_t scratchPad[9];

    if(DS18B20_isConnected(scratchPad)) {
        return DS18B20_calculateTemperature(scratchPad);
    }
    return DEVICE_DISCONNECTED_RAW;
}



/* ---- ---- ---- Conversion Completion Methods ---- ---- ---- */
void DS18B20_blockTillConversionComplete(uint8_t bitResolution, bool checkForConversion)
{
    if(checkForConversion)
    {
        while(!DS18B20_isConversionComplete() /* && Timeout sẽ xét sau */){
            // Do nothing - không làm gì cả
        }
    }
    else {
        unsigned long delayInMillis = DS18B20_millisToWaitForConversion(bitResolution);

        LL_delay_ms(delayInMillis);
    }
}
/* ---- ---- ---- End ---- ---- ---- */

float DS18B20_calculateTemperature(const uint8_t *scratchPad)
{
    int16_t raw = (scratchPad[TEMP_MSB] << 8) | scratchPad[TEMP_LSB];

    // DS18B20 dùng đơn vị là 1/16 độ C => nghĩa là cứ 1 đơn vị ~ 0.0625 độ C
    // Vd: Gsiá trị của 2 thanh ghi là 10 -> 0.625 độ C
    return raw * 0.0625f;
}

bool DS18B20_isAllZeros(const uint8_t *const scratchPad, const size_t length)
{
    for (size_t i = 0; i < length; i++)
    {
        if (scratchPad[i] != 0)
            return false;
    }
    return true;
}
