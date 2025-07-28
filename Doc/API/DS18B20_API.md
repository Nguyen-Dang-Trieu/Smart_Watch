~~~c
typedef struct DS18B20_Device
{   
    uint32_t Pin = LL_GPIO_PIN_4; // Port A: Hiện tại chỉ sử dụng pin 4 của port A

    uint8_t bitResolution      = 12;        // default
    bool    autoSaveScratchPad = true;
    bool    waitForConversion  = true;
    bool    checkForConversion = false; 

    uint8_t deviceAddress[8];               // ROM CODE - Chưa sử dụng
    uint8_t ScratchPad[9];                  // Memory
} DS18B20;
~~~
