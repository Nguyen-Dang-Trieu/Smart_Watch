## DS18B20 API

|     Function Name            | Uses                                    |
| :--------------------------- | :--------------------------------------:|
| DS18B20_init()               | Khởi tạo thư viện                       | 
| DS18B20_isConnected()        | Kiểm tra kết nối và dữ liệu             | 
| DS18B20_requestTemperatures()| Yêu cầu cảm biến bắt đầu lấy nhiệt độ   |  
| DS18B20_getTempC()           | Đọc nhiệt độ (°C)                       |   
| DS18B20_getTempF()           | Đọc nhiệt độ (°F)                       | 
| DS18B20_setResolution()      |Điều chỉnh độ phân giải (9 - 12 bit)     | 

📌 **Ví dụ:**


~~~c
#include "DS18B20.h"

// B1. kHAI BÁO THIẾT BỊ
DS18B20 Device_1 = {
    .Pin                 = LL_GPIO_PIN_4;   // Port A: Hiện tại chỉ sử dụng pin 4 của port A
    .bitResolution       = 12;              // default
    .autoSaveScratchPad  = true;
    .waitForConversion   = true;
    .checkForConversion  = false;
};

// B2. KHỞI TẠO
DS18B20_init();

// B3. YÊU CẦU NHIỆT ĐỘ
DS18B20_requestTemperatures();

// B4. LẤY NHIỆT ĐỘ
DS18B20_getTempC(); // Độ C
DS18B20_getTempF(); // Độ F
~~~

