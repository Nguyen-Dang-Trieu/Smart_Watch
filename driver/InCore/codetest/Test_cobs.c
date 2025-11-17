#include <stdio.h>
#include <stdint.h>
#include "crc8.h"
#include "cobs.h"

int main() {
    // Example data to encode
    uint8_t data[] = {0x11, 0x22, 0x00, 0x33, 0x44};
    int data_len = sizeof(data) / sizeof(data[0]);  
    printf("datalen: %d\n", COBS_MAX_LEN(data_len));

    uint8_t encoded[COBS_MAX_LEN(data_len)];
    uint8_t decoded[data_len];
    
    printf("Original Data: ");
    for (int i = 0; i < data_len; i++) {
        printf("%02X ", data[i]);
    }
    printf("\n");

    // COBS Encode
    int encoded_len = COBS_Encode(data, data_len, encoded);
    printf("Encoded Data: ");
    for (int i = 0; i < encoded_len; i++) {
        printf("%02X ", encoded[i]);
    }
    printf("\n");

    // COBS Decode
    int decoded_len = COBS_Decode(encoded, encoded_len, decoded);
    printf("Decoded Data: ");
    for (int i = 0; i < decoded_len; i++) {
        printf("%02X ", decoded[i]);
    }
    printf("\n");

    return 0;
}