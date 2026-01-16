# InCore
## STM32 ↔ ESP32 Communication
Similar to inter-core communication in a multi-core CPU, STM32 and ESP32 exchange data through a well-defined communication layer.
To standardize packet formatting and ensure reliable data transfer, a custom communication library named InCore was developed. This library is inspired by [SerialTransfer](https://github.com/PowerBroker2/SerialTransfer).

Key features of the InCore library:
- **Packet structure**:
~~~
[Start Byte] [Packet ID] [Payload Length] [Payload Bytes ...] [CRC8] [Stop Byte]

Start Byte      : 0x7E
Packet ID       : 1-byte packet identifier
Payload Length  : Length of COBS-encoded payload
Payload Bytes   : Actual data (up to 254 bytes)
CRC8            : 8-bit checksum for error detection
Stop Byte       : 0x81
~~~
- Error detection using CRC8.
- COBS (Consistent Overhead Byte Stuffing) encoding to eliminate 0x00 bytes in the payload.
