# ⌚ Smart Watch Project
## I. Overview
The **Smart Watch** project is a from-scratch embedded system, covering both hardware design and firmware development. The system is built on **FreeRTOS** and follows a distributed **dual-core architecture**, where processing is divided across two microcontrollers.

> 📌 **Note**    
> In the early development phase, off-the-shelf hardware modules are used to accelerate software development and system validation. Once the firmware architecture is stabilized, the hardware will be redesigned and optimized to meet actual smartwatch constraints.


## II. System Architecture
<p align="center">
  <img src="./images/System Architecture.png" width="500">
</p>

To gain deeper practical experience with both **STM32** and **ESP32**, instead of using a **single multi-core MCU**, the system is designed using **two independent microcontrollers**, forming a logically dual-core system:
- **STM32F103C8T6**: Responsible for sensor interfacing and interaction with external peripherals.
- **ESP32**: Handles user interface (UI), high-level data processing, and IoT connectivity.

<p align="center">
  <img src="./images/System Architecture_2.png" width="500">
</p>
The image above shows the scope of modules managed by each microcontroller.

The project contains many modules. For detailed information about any module, please refer to the corresponding folder in the `doc` directory.

> 📌 **Note**   
> The project is still under active development. If detailed documentation for a module is not yet available, please be patient. The documentation is continuously updated to help you better understand the project.


## III. Hardware Components
| No.     |        Component       |  Role |
| :-----: | :--------------------: | :--------------------: |
|    1    | STM32F103C8T6          | Sensor data acquisition and peripheral control  |
|    2    | ESP32                  | Data processing, UI rendering, and IoT connectivity |
|    3    | TTP226                 | Touch input interface  | 
|    4    | DS18B20                | Temperature sensing |
|    5    | MAX30102               | Heart rate and SpO₂ measurement|
|    6    | MPU9250                | 9-DOF IMU |
|    7    | BMP280                 | Barometric pressure and altitude measurement | 

## IV. Build and Run
STM32 (Logical Core 1)
- Open the project in Keil C under the `core1_stm` directory.
- Include the required driver modules.
- Build and flash the firmware to the STM32.

ESP32 (Logical Core 2)
- Open the project in VS Code with ESP-IDF under the `core2_esp` directory.
- Include the required driver modules.
- Build and flash the firmware to the ESP32.

## V. 🔥 Future Development
- Replace STM32 + ESP32 with STM32WB dual-core MCU as the main processor. 
- Replace FreeRTOS with a **custom OS tailored specifically for smartwatch functionality**. 
- Basic filesystem is functional but not fully optimized; needs improvement for safe shutdown and power-off handling.

## VI. 🔎 References 
- Thư viện tham khảo cho MPU9250: https://github.com/DonovanZhu/9DoF_MARG_Madgwick_Filter/blob/master/Teensy/MPU9250/MPU9250_Madwick_Filter/MPU9250.h
- https://github.com/microsoft/IoT-For-Beginners/tree/main
- https://github.com/ZSWatch/ZSWatch
- https://github.com/fbiego
- Màn hình tròn, cảm ứng: https://www.proe.vn/1-28inch-touch-lcd
- Filesystem SPIFFS stm32 + freeRTOS: cần thiết kế thread safe. https://github.com/pellepl/spiffs + AT45DB161D-SU IC Nhớ Flash 16Mbit 8-SOIC (có thể dùng loại E để thay thế)
