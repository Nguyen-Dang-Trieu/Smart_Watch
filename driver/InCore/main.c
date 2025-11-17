#include <stdio.h>
#include "packet.h"

int main() {
    // Packet
    Packet_t txPacket = {0};
    Packet_t rxPacket = {0};

    // Dữ liệu mẫu cần gửi
    int a = 4; // 0x00 00 00 04
    int lenBufferRaw = writeToBuffer(&a, txPacket.buffer, sizeof(a));

    printBuffer(txPacket.buffer, lenBufferRaw);
    
    int lenBufferCOBS = Packet_Construct(&txPacket, lenBufferRaw, 0);
    printf("Packet length after COBS: %d\n", lenBufferCOBS);

    printBuffer(txPacket.buffer, lenBufferCOBS); 

    printf("Full Packet:\n");
    printBuffer(txPacket.preamble, PREAMBLE_SIZE);
    printBuffer(txPacket.buffer, lenBufferCOBS);
    printBuffer(txPacket.postamble, POSTAMBLE_SIZE);



    /* ---- Nhận dữ liệu từ txPacket ---- */ 
    printf("\n--- Receiving Packet ---\n");
    uint8_t *packetBytes = (uint8_t *)&txPacket; // ép kiểu txPacket thành mảng byte
    uint8_t totalPacketSize = PREAMBLE_SIZE + lenBufferCOBS + POSTAMBLE_SIZE;
    printf("Total Packet Size: %d\n", totalPacketSize);
    uint8_t currentByte;


    // Test thử packetBytes
    for(int i = 0; i < totalPacketSize; i++) {
        if(i < PREAMBLE_SIZE) {
            currentByte = txPacket.preamble[i];
        } else if(i < PREAMBLE_SIZE + lenBufferCOBS) {
            currentByte = txPacket.buffer[i - PREAMBLE_SIZE];
        } else {
            currentByte = txPacket.postamble[i - PREAMBLE_SIZE - lenBufferCOBS];
        }
        Packet_Parse(&rxPacket, currentByte);
    }

    // In ra buffer nhận được
    printf("preamble:\n");
    printBuffer(rxPacket.preamble, PREAMBLE_SIZE);  
    printf("buffer:\n");
    printBuffer(rxPacket.buffer, rxPacket.preamble[2]);
    printf("postamble:\n");
    printBuffer(rxPacket.postamble, POSTAMBLE_SIZE);

    // Dữ liệu nhận được
    int receivedValue = 0;
    COBS_Decode(rxPacket.buffer, rxPacket.preamble[2], rxPacket.buffer);
    readFromBuffer(&receivedValue, rxPacket.buffer, sizeof(receivedValue));
    printf("Received Value: %d\n", receivedValue);

    return 0;
}