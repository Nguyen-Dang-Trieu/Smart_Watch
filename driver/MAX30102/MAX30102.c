#include "MAX30102.h"

bool MAX30102_Init(void)
{
	// Check that a MAX30102 is connected
	if(MAX30102_readPartID() != ID_MAX30102)
	{
		// Error -- Part ID read from MAX30102 does not match expected part ID.
    // This may mean there is a physical connectivity problem (broken wire, unpowered, etc).
		return false;
	}

	return true;
}

void bitMask(uint8_t Register, uint8_t Mask, uint8_t Bit)
{
	// Read the current value of register
	uint8_t currentContentsReg = MAX30102_readByte(MAX30102_ADDRESS, Register);
	
	// Clear the bit to be modified, keep other bits unchanged
	currentContentsReg = currentContentsReg & Mask;
	
	// Set or Clear the target bit, write back to the register.
	MAX30102_writeByte(MAX30102_ADDRESS, Register, currentContentsReg | Bit);
}

bool safeCheck(uint8_t maxTimeToCheck, sensor* record)
{
	uint8_t Timeout = maxTimeToCheck;

  while (Timeout > 0)
  {
		if (LL_SYSTICK_IsActiveCounterFlag())  // SysTick ticked (1 ms)
    {
       Timeout--;
    }

    if (getData(record) != 0)  // Found new data
    {
       return true;
    }
  }
	return false;
}

uint32_t MAX30102_getRed(sensor* MAX30102)
{
	if(safeCheck(250, MAX30102))
	{
		return (MAX30102->Red[MAX30102->head]);
	}
	else
	{
		return (0);
	}
}

uint32_t MAX30102_getIR(sensor* MAX30102)
{
	if(safeCheck(250, MAX30102))
	{
		return (MAX30102->IR[MAX30102->head]);
	}
	else
	{
		return (0);
	}
}

/* ---- ---- Interrupt  ---- ---- */
uint8_t MAX30102_getINT1(void)
{
	return MAX30102_readByte(MAX30102_ADDRESS, INT_STAT1_REG);
}


uint8_t MAX30102_getINT2(void)
{
	return MAX30102_readByte(MAX30102_ADDRESS, INT_STAT2_REG);
}

void MAX30102_setA_FULL(bool status)
{
	if(status == ENABLE)
	{
		bitMask(INT_ENABLE1_REG, INT_A_FULL_MASK, INT_A_FULL_ENABLE);
	}
	else // status == DISABLE
	{
		bitMask(INT_ENABLE1_REG, INT_A_FULL_MASK, INT_A_FULL_DISABLE);
	}
}

void MAX30102_setPPG_RDY(bool status)
{
	if(status == ENABLE)
	{
		bitMask(INT_ENABLE1_REG, INT_DATA_RDY_MASK, INT_DATA_RDY_ENABLE);
	}
	else // status == DISABLE
	{
		bitMask(INT_ENABLE1_REG, INT_DATA_RDY_MASK, INT_DATA_RDY_DISABLE);
	}
}

void MAX30102_setALC_OVF(bool status)
{
	if(status == ENABLE)
	{
		bitMask(INT_ENABLE1_REG, INT_ALC_OVF_MASK, INT_ALC_OVF_ENABLE);
	}
	else // status == DISABLE
	{
		bitMask(INT_ENABLE1_REG, INT_ALC_OVF_MASK, INT_ALC_OVF_DISABLE);
	}
}	

void MAX30102_setPROX_INT(bool status)
{
	if(status == ENABLE)
	{
		bitMask(INT_ENABLE1_REG, INT_PROX_INT_MASK, INT_PROX_INT_ENABLE);
	}
	else // status == DISABLE
	{
		bitMask(INT_ENABLE1_REG, INT_PROX_INT_MASK, INT_PROX_INT_DISABLE);
	}
}	

void MAX30102_setDIE_TEMP_RDY(bool status)
{
	if(status == ENABLE)
	{
		bitMask(INT_ENABLE2_REG, INT_DIE_TEMP_RDY_MASK, INT_DIE_TEMP_RDY_ENABLE);
	}
	else // status == DISABLE
	{
		bitMask(INT_ENABLE2_REG, INT_DIE_TEMP_RDY_MASK, INT_DIE_TEMP_RDY_DISABLE);
	}
}


/* ---- ---- FIFO  ---- ---- */
/* ----  FIFO Configuration ---- */
void setFIFOAverage(uint8_t numberOfSample)
{
	bitMask(FIFO_CONFIG_REG, SAMPLE_AVG_MASK, numberOfSample);
}

void controlFIFORollover(bool FIFO_ROLLOVER_EN_Bit)	
{
	if(FIFO_ROLLOVER_EN_Bit == true)
	{
		bitMask(FIFO_CONFIG_REG, FIFO_ROLLOVER_MASK, FIFO_ROLLOVER_ENABLE);
	}
	else // FIFO_ROLLOVER_EN_Bit == false
	{
		bitMask(FIFO_CONFIG_REG, FIFO_ROLLOVER_MASK, FIFO_ROLLOVER_DISABLE);
	}
}

void setFIFOAlmostFull(uint8_t samples)
{
	bitMask(FIFO_CONFIG_REG, FIFO_A_FULL_MASK, FIFO_A_FULL_0);
}

/* ----  FIFO Operating ---- */
uint8_t getData(sensor* record)
{
	uint8_t readPointer  = getReadPointer();
	uint8_t writePointer = getWritePointer();
	
	int8_t numberOfSample = 0;
	if(readPointer != writePointer)
	{
		numberOfSample = writePointer - readPointer;
		if(numberOfSample < 0) numberOfSample += 32; //Wrap condition
		
		uint8_t bytesLeftToRead = numberOfSample * (record->activeLeds * 3);
		
		/* Bat dau qua trinh lay data tu FIFO */
		// Khoi tao FIFO
		FIFO buffer = {
			.index = 0
		};
		
		// Lay full du lieu trong FIFO va luu tru trong buffer.data
		MAX30102_requestBytes(MAX30102_ADDRESS, FIFO_DATA_REG, buffer.data, bytesLeftToRead);
		
		while(buffer.index < bytesLeftToRead)
		{
			record->head++;
			record->head %= STORAGE_SIZE; //Wrap condition of Circular buffer
			
			uint8_t  arrTemp[4];
			uint32_t Temp = 0;
			
			// Doc 3 bytes cua Red led
			arrTemp[3] = 0;
			arrTemp[2] = MAX30102_read(&buffer, bytesLeftToRead);
			arrTemp[1] = MAX30102_read(&buffer, bytesLeftToRead);
			arrTemp[0] = MAX30102_read(&buffer, bytesLeftToRead);
			
			// Convert array to uint32_t
			memcpy(&Temp, arrTemp, sizeof(Temp));
			Temp &= 0x3FFFF; // 3 bytes nhung chi su dung 18 bit thap
			
			record->Red[record->head] = Temp; // Luu tru gia tri doc duoc vao trong Circular Buffer
			
			if(record->activeLeds > 1)
			{
				// Doc 3 bytes cua IR led
				arrTemp[3] = 0;
				arrTemp[2] = MAX30102_read(&buffer, bytesLeftToRead);
				arrTemp[1] = MAX30102_read(&buffer, bytesLeftToRead);
				arrTemp[0] = MAX30102_read(&buffer, bytesLeftToRead);
				
				// Convert array to uint32_t
				memcpy(&Temp, arrTemp, sizeof(Temp));
				Temp &= 0x3FFFF; // 3 bytes nhung chi su dung 18 bit thap
			
				record->IR[record->head] = Temp; // Luu tru gia tri doc duoc vao trong Circular Buffer
			}
		}
	}
	
	return numberOfSample;
}


uint8_t getWritePointer(void)
{
	return MAX30102_readByte(MAX30102_ADDRESS, FIFO_WRITE_PTR_REG);
}

uint8_t getReadPointer(void)
{
	return MAX30102_readByte(MAX30102_ADDRESS, FIFO_READ_PTR_REG);
}

void clearFIFO(void)
{
	MAX30102_writeByte(MAX30102_ADDRESS, FIFO_WRITE_PTR_REG, 0x00);
	MAX30102_writeByte(MAX30102_ADDRESS, FIFO_OVERFLOW_COUNT_REG, 0x00);
	MAX30102_writeByte(MAX30102_ADDRESS, FIFO_READ_PTR_REG, 0x00);
}

/* ---- ---- Mode Configuration ---- ---- */
void Reset(void)
{
	// Dua cac thanh ghi ve trang thai mac dinh
	bitMask(MODE_CONFIG_REG, RESET_MASK, RESET);
	
	// Kiem tra bit RESET
	uint32_t timeoutReset = 100; // Maximum wait time = 100 ms
	uint8_t  bitReset = 0;
	
	while(timeoutReset > 0)
	{
		bitReset = MAX30102_readByte(MAX30102_ADDRESS, MODE_CONFIG_REG);
		if((bitReset & RESET) == 0) break;
		
		// Tranh bi nhot trong while()
		if (LL_SYSTICK_IsActiveCounterFlag()) 
    {
      timeoutReset--;
    }
	}
}

void setLEDMode(uint8_t Mode)
{
	bitMask(MODE_CONFIG_REG, MODE_MASK, Mode);
}

/* ---- ---- SpO2 Configuration ---- ---- */
void setADCRange(uint8_t Range)
{
	bitMask(SP02_CONFIG_REG, ADC_RANGE_MASK, Range);
}

void setSampleRate(uint8_t sampleRate)
{
	bitMask(SP02_CONFIG_REG, SAMPLE_RATE_MASK, sampleRate);
}

void setPulseWidth(uint8_t pulseWidth)
{
	bitMask(SP02_CONFIG_REG, PULSE_WIDTH_MASK, pulseWidth);
}

/* ---- ---- LED Pulse Amplitude ---- ---- */
void setPulseAmplitudeRed(uint8_t value)
{
	MAX30102_writeByte(MAX30102_ADDRESS, LED1_PULSE_AMP_REG, value);
}

void setPulseAmplitudeIR(uint8_t value)
{
	MAX30102_writeByte(MAX30102_ADDRESS, LED2_PULSE_AMP_REG, value);
}

void setPulseAmplitudeProximity(uint8_t value)
{
	MAX30102_writeByte(MAX30102_ADDRESS, LED_PROX_AMP_REG, value);
}

/* ---- ---- Multi-LED Mode Control Registers ---- ---- */
void selectSlot(uint8_t slot, uint8_t led)
{
	switch(slot)
	{
		case 1: bitMask(MULTI_LED_CONFIG1_REG, SLOT_1_MASK, led); 		 break;
		case 2: bitMask(MULTI_LED_CONFIG1_REG, SLOT_2_MASK, led << 4); break; 
		case 3: bitMask(MULTI_LED_CONFIG2_REG, SLOT_3_MASK, led);      break;
		case 4: bitMask(MULTI_LED_CONFIG2_REG, SLOT_4_MASK, led << 4); break;
		default: break;
	}
}

/* ---- ---- Detecting ID/Revision ---- ---- */
uint8_t MAX30102_readPartID(void)
{
	return MAX30102_readByte(MAX30102_ADDRESS, PART_ID_REG);
}

/* ---- ---- Low Level I2C Communication ---- ---- */
uint8_t MAX30102_readByte(uint8_t Slave, uint8_t Register)
{
	// (1) Gui START condition va dia chi device (dia chi + bit Write = 0)
	LL_I2C_AcknowledgeNextData(I2C1, LL_I2C_ACK);
	LL_I2C_GenerateStartCondition(I2C1);
	while(!LL_I2C_IsActiveFlag_SB(I2C1));
	
	LL_I2C_TransmitData8(I2C1, (Slave << 1)| I2C_REQUEST_WRITE);
	while(!LL_I2C_IsActiveFlag_ADDR(I2C1)){
		// Do nothing
	}
	LL_I2C_ClearFlag_ADDR(I2C1);
	
	// (2) Gui dia chi thanh ghi
	while (!LL_I2C_IsActiveFlag_TXE(I2C1)){
		// Do nothing
	}
  LL_I2C_TransmitData8(I2C1, Register);
  while (!LL_I2C_IsActiveFlag_TXE(I2C1));
	
	// (3) Gui Repeat Start conditon + dia chi device (dia chi + bit Read = 1)
	LL_I2C_AcknowledgeNextData(I2C1, LL_I2C_NACK);
	LL_I2C_GenerateStartCondition(I2C1);
	while (!LL_I2C_IsActiveFlag_SB(I2C1)){
		// Do nothing
	}
	
	LL_I2C_TransmitData8(I2C1, (Slave << 1)| I2C_REQUEST_READ);
	while(!LL_I2C_IsActiveFlag_ADDR(I2C1)){
		// Do nothing
	}
	LL_I2C_ClearFlag_ADDR(I2C1);
	
	// (4) Stop
	LL_I2C_GenerateStopCondition(I2C1);
	
	// (5) Cho va doc du lieu
	while (!LL_I2C_IsActiveFlag_RXNE(I2C1)){
		// Do nothing
	}
	return LL_I2C_ReceiveData8(I2C1);
}

void MAX30102_requestBytes(uint8_t Slave, uint8_t Register, uint8_t* BufferI2C, uint8_t sizeBufferI2C)
{
	// (1) Gui START condition va dia chi device (dia chi + bit Write = 0)
	LL_I2C_GenerateStartCondition(I2C1);
	while(!LL_I2C_IsActiveFlag_SB(I2C1));
	
	LL_I2C_TransmitData8(I2C1, (Slave << 1)| I2C_REQUEST_WRITE);
	while(!LL_I2C_IsActiveFlag_ADDR(I2C1)){
		// Do nothing
	}
	LL_I2C_ClearFlag_ADDR(I2C1);
	
	// (2) Gui dia chi thanh ghi
	while (!LL_I2C_IsActiveFlag_TXE(I2C1)){
		// Do nothing
	}
  LL_I2C_TransmitData8(I2C1, Register);
  while (!LL_I2C_IsActiveFlag_TXE(I2C1));
	
	// (3) Gui Repeat Start conditon + dia chi device (dia chi + bit Read = 1)
	LL_I2C_GenerateStartCondition(I2C1);
	while (!LL_I2C_IsActiveFlag_SB(I2C1)){
		// Do nothing
	}
	
	LL_I2C_TransmitData8(I2C1, (Slave << 1)| I2C_REQUEST_READ);
	while(!LL_I2C_IsActiveFlag_ADDR(I2C1)){
		// Do nothing
	}
	LL_I2C_ClearFlag_ADDR(I2C1);
	
	// (4) Cho va doc du lieu
	for(uint8_t i = 0; i < sizeBufferI2C; i++)
	{
		// Check byte cuoi cung
		if(i == sizeBufferI2C - 1)
		{
			LL_I2C_AcknowledgeNextData(I2C1, LL_I2C_NACK);
		}
		else
		{
			LL_I2C_AcknowledgeNextData(I2C1, LL_I2C_ACK);
		}
		
		// Wait RXNE flag set.
		while (!LL_I2C_IsActiveFlag_RXNE(I2C1)){
			// Do nothing
		}
		BufferI2C[i] = LL_I2C_ReceiveData8(I2C1);
	}
	
	// (5) Gui STOP condition
	LL_I2C_GenerateStopCondition(I2C1);
}

uint8_t MAX30102_read(FIFO* bufferPointer, uint8_t bufferSize)
{
	if(bufferPointer->index >= (bufferSize - 1))
	{
		bufferPointer->index = 0;
	}
	return bufferPointer->data[(bufferPointer->index)++];
}

void MAX30102_writeByte(uint8_t Slave, uint8_t Register, uint8_t Value)
{
	LL_I2C_AcknowledgeNextData(I2C1, LL_I2C_ACK);
	
	// (1) Tao xung dieu kien: START
	LL_I2C_GenerateStartCondition(I2C1);
	
	// (2) Cho bit SB trong thanh ghi I2C_SR1 duoc set.
#if (USE_TIMEOUT == 1)
  Timeout = I2C_SEND_TIMEOUT_SB_MS;
#endif /* USE_TIMEOUT */

  /* Loop until SB flag is raised  */
  while(!LL_I2C_IsActiveFlag_SB(I2C1))
  {
#if (USE_TIMEOUT == 1)
    /* Check Systick counter flag to decrement the time-out value */
    if (LL_SYSTICK_IsActiveCounterFlag()) 
    {
      if(Timeout-- == 0)
      {
				break; // Thoat khoi vong lap
      }
    }
#endif /* USE_TIMEOUT */
  }
	
	// (3) Gui dia chi cua slave + bit write: 0
	LL_I2C_TransmitData8(I2C1, (Slave << 1) | I2C_REQUEST_WRITE);
	
	// (4) Cho bit ADDR = 1
	/* Loop until ADDR flag is raised  */
  while(!LL_I2C_IsActiveFlag_ADDR(I2C1))
  {
#if (USE_TIMEOUT == 1)
    /* Check Systick counter flag to decrement the time-out value */
    if (LL_SYSTICK_IsActiveCounterFlag()) 
    {
      if(Timeout-- == 0)
      {
				break;
      }
    }
#endif /* USE_TIMEOUT */
  }
	
	// (5) Clear bit ADDR = 0 (neu khong clear thi ta se khong gui dc data) + du lieu
	LL_I2C_ClearFlag_ADDR(I2C1);
	
	/* Ghi 2 byte data: dia chi thanh ghi + du lieu */
	uint8_t bytesForWriting = 2;
	uint8_t data[2] = { Register, Value };
	uint8_t *data_p = data;
	
	while(bytesForWriting > 0)
	{
		/* Khi TXE = 1 => thanh ghi DR rong, san sang gui byte tiep theo */
		if (LL_I2C_IsActiveFlag_TXE(I2C1))
    {
        LL_I2C_TransmitData8(I2C1, (*data_p++));
        bytesForWriting--;
    }
		
#if (USE_TIMEOUT == 1)
      Timeout = I2C_SEND_TIMEOUT_TXE_MS;
#endif /* USE_TIMEOUT */
    

#if (USE_TIMEOUT == 1)
    /* Check Systick counter flag to decrement the time-out value */
    if (LL_SYSTICK_IsActiveCounterFlag()) 
    {
      if(Timeout-- == 0)
      {
				break;
      }
    }
#endif /* USE_TIMEOUT */
	}
	
	// (6) Ket thuc: dieu kien STOP
	LL_I2C_GenerateStopCondition(I2C1);
}




