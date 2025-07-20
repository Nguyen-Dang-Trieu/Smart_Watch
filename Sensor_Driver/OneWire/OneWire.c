#include "OneWire.h"

/**
 * @brief Initialize the One-Wire bus.
 * 
 * This function enables the clock for GPIOA and configures PA4 as an input with no pull-up and pull-down
 * 
 */
void OneWire_Init(void)
{
	// Enable clock cho GPIOA
	LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOA);

	// Cấu hình Pin Mode
	LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_4);
	LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_4, LL_GPIO_MODE_INPUT);
	LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_4, LL_GPIO_PULL_NO);
}


/**
 * @brief Perform the One-Wire reset sequence.
 *
 * This function sends a reset pulse and waits up to 250 µs for the bus
 * to return HIGH. If the bus remains low, it may indicate a short circuit.
 *
 * @return uint8_t Returns 1 if a presence pulse is detected, 0 otherwise.
 */
uint8_t OneWire_Reset(void)
{
	uint8_t r;
	uint8_t retries = 125;

	LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_4, LL_GPIO_MODE_INPUT); // receive data

	// -------------------------
	// Xử lí trường hợp: Bus đang bị Device nào đó giữ mức LOW
	// Master se cho doi trong 250 us de cho bus on dinh o muc HIGH.
	// wait until the wire is high... just in case

	do
	{
		if (--retries == 0)
			return 0;
		LL_delay_us(2);
	} while (!LL_GPIO_IsInputPinSet(GPIOA, LL_GPIO_PIN_4));

	// -------------------------

	LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_4);
	LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_4, LL_GPIO_MODE_OUTPUT);

	LL_delay_us(480);

	LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_4, LL_GPIO_MODE_INPUT);
	LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_4, LL_GPIO_PULL_NO);		// allow it to float

	LL_delay_us(70);

	r = !LL_GPIO_IsInputPinSet(GPIOA, LL_GPIO_PIN_4);
	LL_delay_us(410);
	return r;
}

/**
 * @brief Write a single bit to the One-Wire bus.
 * 
 * This function performs a bit write operation on the One-Wire bus according to the protocol timing.
 * The timing for writing a '1' or a '0' is carefully controlled using delays.
 * 
 * @param bit The bit to write: 0 or 1.
 */
void OneWire_WriteBit(uint8_t bit)
{
	if (bit & 1) // Write bit "1"
	{
		LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_4); // Master output LOW
		LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_4, LL_GPIO_MODE_OUTPUT); 
		LL_delay_us(10);
		LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_4);	 // Master output HIGH
		LL_delay_us(55);
	}
	else	 	// Write bit "0"
	{
		LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_4); // Master output LOW
		LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_4, LL_GPIO_MODE_OUTPUT);
		LL_delay_us(65);
		LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_4);   // Master output HIGH
		LL_delay_us(5);
	}
}


/**
 * @brief Read a single bit from the One-Wire bus
 * 
 * This function initiates a read time slot by pulling the bus low briefly, then switching the pin to input 
 * mode to sample the bit from the slave device (DS18B20).
 * 
 * @return uint8_t Returns 1 if the bus is HIGH (bit = 1), 0 if LOW (bit = 0).
 */
uint8_t OneWire_ReadBit(void)
{
	uint8_t result;

	// The master initiates "read time slots"
	LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_4, LL_GPIO_MODE_OUTPUT);
	LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_4);

	LL_delay_us(3);

	// The master recieves datas.
	LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_4, LL_GPIO_MODE_INPUT);
	LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_4, LL_GPIO_PULL_NO);		// let pin float, pull up will raise

	LL_delay_us(10);

	r = LL_GPIO_IsInputPinSet(GPIOA, LL_GPIO_PIN_4);

	LL_delay_us(53);

	return result;
}


/**
 * @brief Write a byte to the One-Wire bus (LSB first).
 * 
 * This function sends a full byte to the One-Wire bus by writing each bit individually using the 
 * OneWire_WriteBit() function. Bits are sent in LSB-first order, as required by the One-Wire protocol
 * 
 * @param byte The 8-bit data value to transmit.
 */
void OneWire_Write(uint8_t byte)
{
	uint8_t bitMask;

	for (bitMask = 0x01; bitMask != 0 ; bitMask <<= 1)
	{
		OneWire_WriteBit((bitMask & byte) ? 1 : 0);
	}
}

/**
 * @brief Write multiple bytes to the One-Wire bus (LSB first per byte)
 * 
 * This function transmits a sequence of bytes over the One-Wire bus. Each byte is sent LSB-first.
 * 
 * @param buf 	Pointer to the buffer containing the data to send.
 * @param count Number of bytes to transmit.
 */
void OneWire_WriteBytes(const uint8_t *buf, uint16_t count)
{
	for (uint16_t i = 0; i < count; i++)
		OneWire_Write(buf[i]);
}

/**
 * @brief Read a byte from the One-Wire bus (LSB first).
 * 
 * This function reads 8 bits from the One-Wire bus, one bit at a time, 
 * starting from the least significant bit (LSB) to the most significant bit (MSB).
 * 
 * @return uint8_t The byte read from the bus.
 */
uint8_t OneWire_Read()
{
	uint8_t bitMask;
	uint8_t result = 0x00;

	for (bitMask = 0x01; bitMask != 0; bitMask <<= 1)
	{
		if (OneWire_ReadBit())
			result |= bitMask;
	}
	return result;
}

/**
 * @brief Read multiple bytes from the One-Wire bus (LSB first per byte).
 * 
 * This function reads a sequence of bytes from the One-Wire bus.
 * 
 * @param buf   Pointer to the buffer where the received bytes will be stored.
 * @param count Number of bytes to read from the bus.
 */
void OneWire_ReadBytes(uint8_t *buf, uint16_t count)
{
	for (uint16_t i = 0; i < count; i++)
		buf[i] = OneWire_Read();
}


/**
 * @brief Send the Skip ROM [0xCC] command to the One-Wire bus.
 * 
 * This function issues the Skip ROM command, which allows the master to 
 * address all devices on the bus simultaneously. It is typically used when
 * there is only one device connected, or when all devices should respond 
 * to the next command (e.g. Read Scratchpad).
 * 
 * @note This function should be called after a successful OneWire_Reset().
 */
void OneWire_Skip()
{
	OneWire_Write(0xCC); // Skip ROM
}


#if ONEWIRE_CRC

#if ONEWIRE_CRC8_TABLE
// Dow-CRC using polynomial X^8 + X^5 + X^4 + X^0
// See http://lentz.com.au/blog/calculating-crc-with-a-tiny-32-entry-lookup-table
static const uint8_t dscrc2x16_table[] = {
    0x00, 0x5E, 0xBC, 0xE2, 0x61, 0x3F, 0xDD, 0x83,
    0xC2, 0x9C, 0x7E, 0x20, 0xA3, 0xFD, 0x1F, 0x41,
    0x00, 0x9D, 0x23, 0xBE, 0x46, 0xDB, 0x65, 0xF8,
    0x8C, 0x11, 0xAF, 0x32, 0xCA, 0x57, 0xE9, 0x74
};

uint8_t OneWire_crc8(const uint8_t *addr, uint8_t len)
{
    uint8_t crc = 0;

    while (len--)
    {
        crc = *addr++ ^ crc; 
        crc = dscrc2x16_table[(crc & 0x0F)] ^
              dscrc2x16_table[16 + ((crc >> 4) & 0x0F)];
    }

    return crc;
}
#else

uint8_t OneWire_crc8(const uint8_t *addr, uint8_t len)
{
    uint8_t crc = 0;

    while (len--)
    {
        uint8_t inbyte = *addr++;
        for (uint8_t i = 8; i; i--)
        {
            uint8_t mix = (crc ^ inbyte) & 0x01;
            crc >>= 1;
            if (mix)
                crc ^= 0x8C;
            inbyte >>= 1;
        }
    }
    return crc;
}
#endif // ONEWIRE_CRC8_TABLE

#endif // ONEWIRE_CRC

#endif

