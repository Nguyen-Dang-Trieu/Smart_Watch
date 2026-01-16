#include "cobs.h"

int COBS_Encode(const uint8_t *source, int len, uint8_t *destination)
{
    const uint8_t *start = destination;
    const uint8_t *end   = source + len; // end trỏ tới byte sau "byte cuối cùng" của destination

    /* Where to insert the leading count */
    uint8_t byteCode;
    uint8_t *storeByteCode; // trỏ tới vị trí lưu trữ byte code

    // Gán trước rồi mới tăng con trỏ destination.
    storeByteCode = destination++;  // Nơi lưu byte code của block hiện tại
    byteCode = 1;                   // Vì bản thân byte code cũng tính vào độ dài block

    while (source < end)
    {
        if (byteCode != 0xFF)
        {
            uint8_t byteDataCurrent = *source++; // *ptr lấy byte dữ liệu hiện tại và tăng con trỏ ptr
            if (byteDataCurrent != 0)
            {
                *destination++ = byteDataCurrent;
                byteCode++;
                continue;
            }
        }
        
        // Xử lí khi gặp byte 0 hoặc đạt đến giới hạn block 254 byte
        *storeByteCode = byteCode;
        storeByteCode = destination++;
        byteCode = 1;
    }
    *storeByteCode++ = byteCode;

    *destination++ = 0; // add delimiter

    return (int)(destination - start);
}

int COBS_Decode(const uint8_t *source, int len, uint8_t *destination)
{
    const uint8_t *start = destination;
    const uint8_t *end   = source + len;
    uint8_t code = 0xFF;
    uint16_t copy = 0;
    int ret;

    for (; source < end; copy--)
    {
        if (copy != 0)
        {
            *destination++ = *source++;
        }
        else
        {
            // || ((end-ptr) == 1) handles the edge case where a 0 is 255 bytes from the end
            // Some implementations handle this differently, but this should work with all of them
            if (code != 0xFF || ((end - source) == 1))
            {
                *destination++ = 0;
            }

            copy = code = *source++;

            if (code == 0)
            {
                break; /* Source length too long */
            }
        }
    }

    return (destination - start) - 1; // Exclude the delimiter from the length
}
