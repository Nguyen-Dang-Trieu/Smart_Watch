#ifndef _MAX30102_H
#define _MAX30102_H

#include "main.h"

/*  ---- Standard Library ---- */
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

// ---- ---- ---- ---- Address of Registers ---- ---- ---- ----
/* ---- Status Register ---- */
#define INT_STAT1_REG            0x00
#define INT_STAT2_REG            0x01
#define INT_ENABLE1_REG          0x02
#define INT_ENABLE2_REG          0x03

/* ---- FIFO Register ---- */
#define FIFO_WRITE_PTR_REG       0x04
#define FIFO_OVERFLOW_COUNT_REG  0x05
#define FIFO_READ_PTR_REG        0x06
#define FIFO_DATA_REG            0x07

/* ---- Configuration Registers ---- */
#define FIFO_CONFIG_REG          0x08
#define MODE_CONFIG_REG          0x09       
#define SP02_CONFIG_REG          0x0A
#define LED1_PULSE_AMP_REG       0x0C
#define LED2_PULSE_AMP_REG       0x0D
#define LED_PROX_AMP_REG         0x10
#define MULTI_LED_CONFIG1_REG    0x11
#define MULTI_LED_CONFIG2_REG    0x12   

/* ---- Die Temperature Registers ---- */
#define DIE_TEMP_INT_REG         0x1F
#define DIE_TEMP_FRAC_REG        0x20
#define DIE_TEMP_CONFIG_REG      0x21

/* ---- Proximity Function Registers ---- */    
#define PROX_INT_THRESH_REG      0x30

/* ---- Part ID Registers ---- */
#define REVISION_ID_REG          0xFE   
#define PART_ID_REG              0xFF    

#define ID_MAX30102              0x15 // Expected Part ID for MAX30102

/* ---- Setting I2C ---- */
#define MAX30102_ADDRESS				 0x57
#define I2C_REQUEST_WRITE				 0x00
#define I2C_REQUEST_READ				 0x01

#define USE_TIMEOUT							 0

#if (USE_TIMEOUT == 1)
	#define I2C_SEND_TIMEOUT_TXE_MS		5
	#define I2C_SEND_TIMEOUT_SB_MS		5
	#define I2C_SEND_TIMEOUT_ADDR_MS	5
	
	uint32_t Timeout = 0; /* Variable used for Timeout management */
#endif /* USE_TIMEOUT	*/

// ---- ---- ---- ---- Bits of Registers ---- ---- ---- ----

/* ---- Interrupt configuration ---- */
/* Interrtup Status 1 */
// Bit 7
#define INT_A_FULL_MASK					0x7F
#define INT_A_FULL_ENABLE				0x80
#define INT_A_FULL_DISABLE			0x00
// Bit 6
#define INT_DATA_RDY_MASK				0xBF
#define INT_DATA_RDY_ENABLE			0x40
#define INT_DATA_RDY_DISABLE		0x00
// Bit 5
#define INT_ALC_OVF_MASK				0xDF
#define INT_ALC_OVF_ENABLE			0x20
#define INT_ALC_OVF_DISABLE			0x00
// Bit 4
#define INT_PROX_INT_MASK				0xEF
#define INT_PROX_INT_ENABLE			0x10
#define INT_PROX_INT_DISABLE		0x00

/* Interrtup Status 2 */
// Bit 1
#define INT_DIE_TEMP_RDY_MASK			0xFD
#define INT_DIE_TEMP_RDY_ENABLE 	0x02
#define INT_DIE_TEMP_RDY_DISABLE	0x00

/* ---- FIFO Configuration ---- */
// Bit 7 -> Bit 5
#define SAMPLE_AVG_MASK					0x1F
#define SAMPLE_AVG_1						0x00
#define SAMPLE_AVG_2 						0x20
#define SAMPLE_AVG_4 						0x40
#define SAMPLE_AVG_8						0x60
#define SAMPLE_AVG_16						0x80
#define SAMPLE_AVG_32 					0xA0
// Bit 4
#define FIFO_ROLLOVER_MASK			0xEF
#define FIFO_ROLLOVER_ENABLE 		0x10
#define FIFO_ROLLOVER_DISABLE 	0x00
// Bit 3 -> Bit 0
#define FIFO_A_FULL_MASK				0xF0
#define FIFO_A_FULL_0						0xF0 					// When the interrupt occurs, the FIFO is full. 
// .... Còn nhieu lua chon khac

/* ---- Mode Configuration ---- */
// Bit 7
#define SHUTDOWN_MASK						0x7F
#define SHUTDOWN								0x80
#define WAKEUP									0x00
// Bit 6
#define RESET_MASK							0xBF
#define RESET										0x40
// Bit 0 -> Bit 2
#define MODE_MASK								0xF8
#define MODE_HEART_RATE					0x02
#define MODE_SPO2								0x03
#define MODE_MUTIL_LED					0x07

/* ---- SpO2 Configuration ---- */
// Bit 6 -> Bit 5
#define ADC_RANGE_MASK					0x9F
#define ADC_RANGE_2048					0x00
#define ADC_RANGE_4096					0x20
#define ADC_RANGE_8192					0x40
#define ADC_RANGE_16384					0x60
// Bit 4 -> Bit 2
#define SAMPLE_RATE_MASK				0xE3
#define SAMPLE_RATE_50					0x00
#define SAMPLE_RATE_100					0x04
#define SAMPLE_RATE_200					0x08
#define SAMPLE_RATE_400					0x0C
#define SAMPLE_RATE_800					0x10
#define SAMPLE_RATE_1000				0x14
#define SAMPLE_RATE_1600				0x18
#define SAMPLE_RATE_3200				0x1C
// Bit 1 -> Bit 0
#define PULSE_WIDTH_MASK				0xFC
#define PULSE_WIDTH_69					0x00
#define PULSE_WIDTH_118					0x01
#define PULSE_WIDTH_215					0x02
#define PULSE_WIDTH_411					0x03

/* ---- Multi-LED Mode Control Registers ---- */
// Multi-LED Mode Config_1 (0x11)
#define SLOT_1_MASK							0xF8
#define SLOT_2_MASK							0x8F
// Multi-LED Mode Config_2 (0x12)
#define SLOT_3_MASK							0xF8
#define SLOT_4_MASK							0x8F

#define SLOT_NONE								0x00
#define SLOT_RED_LED						0x01
#define SLOT_IR_LED							0x02
#define SLOT_RED_PILOT					0x05
#define SLOT_IR_PILOT						0x06

/* ---- ---- ---- ---- ---- ---- MAX30102 Struct ---- ---- ---- ---- ---- ---- */

#define STORAGE_SIZE	5

// Dung ky thuat "Circular Buffer" de luu tru du lieu
typedef struct MAX30102 {
	uint8_t activeLeds; 	/* So luong led hoat dong */ 
	// char mode[]; 
	
	uint32_t Red[STORAGE_SIZE];
	uint32_t IR[STORAGE_SIZE];
	uint8_t  head;
	uint8_t  tail;
} sensor;

typedef struct DataSamplesFromFIFO
{
	uint8_t* data;
	uint8_t index;
}	FIFO;


/* ---- ---- ---- ---- ---- ---- MAX30102 Driver API ---- ---- ---- ---- ---- ---- */
/**
  * @brief  MAX30102 Initialization
  * @param  None			
  * @retval bool 	
	* 				 + True  if the Max30102 sensor is detected and initialized successfully
	*					 + False if hardware detection fails or sensor is not found
  */
bool MAX30102_Init(void);

/**
  * @brief  Modify specific bits in a register (set or clear bits)
  * @param  Register 	Address of the register to modify.
	* @param  Mask 			Bit mask to select the bit to modify (bits set to 0 in mask will be cleared, bits set to 1 will be preserved)
	* @param  Bit 			Bit to set or clear
  * @retval None
  */
void bitMask(uint8_t Register, uint8_t Mask, uint8_t Bit);

/**
  * @brief  Check if there is new data
  * @param  maxTimeToCheck 	Maximum time to wait (in milliseconds)
	* @param 	record 					Pointer to the struct that holds MAX30102 data, active leds, and mode
  * @retval bool 						
	*						+True   New data is available
	*						+False 	No data available (timeout)
  */
bool safeCheck(uint8_t maxTimeToCheck, sensor* record);

uint32_t MAX30102_getRed(sensor* MAX30102);			// Read the data of Red led
uint32_t MAX30102_getIR(sensor* MAX30102);			// Read the data of IR led

/* ---- ---- Interrupt  ---- ---- */
/**
  * @brief  Read the Interrupt Status 1,2 register
  * @param  None
  * @retval uint8_t		Current value of the INT1, INT2 status register					
  */
uint8_t MAX30102_getINT1(void);
uint8_t MAX30102_getINT2(void);

#define ENABLE 		true
#define DISABLE 	false

void MAX30102_setA_FULL(bool status);					//  FIFO Almost Full Flag
void MAX30102_setPPG_RDY(bool status); 				//	New FIFO Data Ready
void MAX30102_setALC_OVF(bool status); 				//	Ambient Light Cancellation Overflow
void MAX30102_setPROX_INT(bool status);				//  Proximity Threshold Triggered
void MAX30102_setDIE_TEMP_RDY(bool status);  	// Internal Temperature Ready Flag

/* ---- ---- FIFO  ---- ---- */
/* ----  FIFO Configuration ---- */
/**
  * @brief  Configure the number of samples to average (SMP_AVE) before storing in FIFO.
  * @param  numberOfSample	Number of consecutive samples to average: 1, 2, 4, 8, 16, or 32.	
  * @retval None
  */
void setFIFOAverage(uint8_t numberOfSample);
/**
  * @brief  Enable or disable FIFO rollover when it becomes full.
	*					If rollover is enabled, new samples overwrite the oldest ones when FIFO is full.
	*					If rollover is disabled, new samples are discarded when FIFO is full.
  * @param  FIFO_ROLLOVER_EN_Bit	
	*						True 	- Enable rollover
	*						False - Disable rollover
  * @retval None
  */
void controlFIFORollover(bool FIFO_ROLLOVER_EN_Bit);	
void setFIFOAlmostFull(uint8_t samples);

/* ----  FIFO Operating ---- */
/**
  * @brief  Read sample from FIFO buffer and stored them in struct MAX30102
  * @param  record		 Pointer to the struct that holds MAX30102 data, active leds, and mode			
  * @retval uint8_t    Numbers of sample read
  */
uint8_t getData(sensor* record);


/**
  * @brief  Get the value of FIFO_WR_PTR register
  * @param  None			
  * @retval uint8_t    The 8-bit data of the write pointer
  */
uint8_t getWritePointer(void);

/**
  * @brief  Get the value of FIFO_RD_PTR register
  * @param  None			
  * @retval uint8_t 		The 8-bit data of the read pointer 	
  */
uint8_t getReadPointer(void);
/**
  * @brief  Reset FIFO read and write pointers to zero.
	*					This effectively clears all stored samples in FIFO
  * @param  None			
  * @retval None 	
  */
void clearFIFO(void);



/* ---- ---- Mode Configuration ---- ---- */
/**
  * @brief  Reset all registers to default state.
  * @param  None
  * @retval None
  */
void Reset(void);



void setLEDMode(uint8_t Mode);

/* ---- ---- SpO2 Configuration ---- ---- */
void setADCRange(uint8_t Range);
void setSampleRate(uint8_t sampleRate);
void setPulseWidth(uint8_t pulseWidth);

/* ---- ---- LED Pulse Amplitude ---- ---- */
void setPulseAmplitudeRed(uint8_t value);
void setPulseAmplitudeIR(uint8_t value);
void setPulseAmplitudeProximity(uint8_t value);

/* ---- ---- Multi-LED Mode Control Registers ---- ---- */
/**
  * @brief  Configure a LED slot for the Multi-LED mode of the MAX30102/MAX30105.
  *         Each slot represents a time slot in the LED pulse sequence.
  *         The device supports up to 4 slots (SLOT1 -> SLOT4). 
  *         Each slot can be assigned to a specific LED (Red, IR, Green) or a Pilot LED.
  *         Note: Slots must be enabled in sequential order (e.g., SLOT1 must be set before SLOT2).
  *
  * @param  slot   Slot number to configure: 1, 2, 3, or 4.
  * @param  led    LED type to assign to the slot.
  * @retval None
  */
void selectSlot(uint8_t slot, uint8_t led);

/* ---- ---- Detecting ID/Revision ---- ---- */
/**
  * @brief  Read ID to identify MAX30102 (ID = 0x15).
  * @param  None			
  * @retval uint8_t 	8-bit ID of MAX30102
  */
uint8_t MAX30102_readPartID(void);

/* ---- ---- Low Level I2C Communication ---- ---- */
/**
  * @brief  Function used to read a byte.
  * @param  Slave			Slave address.
	* @param 	Register	Register address.
  * @retval uint8_t 	The 8-bit data read from the register
  */
uint8_t MAX30102_readByte(uint8_t Slave, uint8_t Register);	 

/**
  * @brief  Function used to read  bytes
  * @param  Slave						I2C slave address of MAX30102
	* @param 	Register				Register address
	* @param 	BufferI2C				Store datas returned from I2C	
	* @param 	sizeBufferI2C		Number of bytes to read
  * @retval None
  */
void MAX30102_requestBytes(uint8_t Slave, uint8_t Register, uint8_t* BufferI2C, uint8_t sizeBufferI2C);

/**
  * @brief  Read a byte from FIFO buffer
	*					The FiFo buffer must be previously filled by MAX30102_requestBytes()
  * @param  bufferPointer		Pointer to FIFO buffer structure containing I2C data
	* @param 	bufferSize			Size of buffer
  * @retval uint8_t					The 8-bit data from FIFO
  */
uint8_t MAX30102_read(FIFO* bufferPointer, uint8_t bufferSize);

/**
  * @brief  Function used to write a byte.
  * @param  Slave			Slave address.
	* @param 	Register	Register address.
  * @retval None
  */
void MAX30102_writeByte(uint8_t Slave, uint8_t Register, uint8_t Value); 



#endif // MAX30102_H