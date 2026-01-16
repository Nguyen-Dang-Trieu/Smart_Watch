#include "debug_sfat.h"

void sfat_dump_fat(void)
{
    // Dump 16 byte / dòng
    for (int row = 0; row < FAT_SIZE; row += 16)
    {
        // Offset
        printf("%04X: ", row);

        // Hex part
        for (int i = 0; i < 16; i++)
        {
            if (row + i < FAT_SIZE)
                printf("%02X ", FLASH[ADDRESS_FAT_AREA + row + i]);
            else
                printf("   "); // padding
        }

        // ASCII part: luôn '.'
        printf(" |");
        for (int i = 0; i < 16; i++)
            putchar('.');

        printf("|\n");
    }
}


void sfat_dump_fat_range(size_t start_cluster, size_t end_cluster)
{
    const size_t NUM_CLUSTERS = superblock.total_sectors / superblock.sectors_per_cluster;

    // Kiểm tra phạm vi hợp lệ
    if (start_cluster >= NUM_CLUSTERS)
        start_cluster = NUM_CLUSTERS - 1;
    if (end_cluster >= NUM_CLUSTERS)
        end_cluster = NUM_CLUSTERS - 1;
    if (start_cluster > end_cluster)
        return; // invalid range

    // Chuyển cluster thành offset byte
    size_t start_offset = start_cluster * 2;
    size_t end_offset   = end_cluster * 2 + 1; // +1 vì mỗi cluster 2 byte

    // Dump 16 byte / dòng
    for (size_t row = start_offset; row <= end_offset; row += 16)
    {
        printf("%04zX: ", row);  // offset

        // Hex part
        for (int i = 0; i < 16; i++)
        {
            size_t idx = row + i;
            if (idx <= end_offset)
                printf("%02X ", FLASH[ADDRESS_FAT_AREA + idx]);
            else
                printf("   "); // padding
        }

        // ASCII part: toàn '.'
        printf(" |");
        for (int i = 0; i < 16; i++)
            putchar('.');
        printf("|\n");
    }
}



void sfat_dump_directory_entry(int index) {
    uint8_t buf[SFAT_BYTES_PER_SECTOR];
    AT45DB_ReadPage(SFAT_START_DIR, buf, NULL);

    int offset = index * 16;  // mỗi entry 16 bytes

    if (offset >= SFAT_BYTES_PER_SECTOR) {
        printf("Invalid directory index %d\n", index);
        return;
    }

    // In offset
    printf("%04X  ", offset);

    // HEX
    for (int j = 0; j < 16; j++) {
        printf("%02X ", buf[offset + j]);
        if (j == 7) printf(" ");
    }

    // ASCII
    printf(" |");
    for (int j = 0; j < 16; j++) {
        uint8_t c = buf[offset + j];
        printf("%c", (c >= 32 && c <= 126) ? c : '.');
    }
    printf("|\n");
}

void sfat_dump_directory(void) {
    uint8_t buf[SFAT_BYTES_PER_SECTOR];
    AT45DB_ReadPage(SFAT_START_DIR, buf, NULL);

    printf("      ");
    for (uint8_t i = 0; i < 16; i++) {
        printf("%02X ", i);
        if (i == 7) printf(" ");
    }
    printf("\n");


    for (uint16_t i = 0; i < SFAT_BYTES_PER_SECTOR; i += 16) {

        /* offset */
        printf("%04X  ", i);

        /* HEX (bên trái) */
        for (uint8_t j = 0; j < 16; j++) {
            if (i + j < SFAT_BYTES_PER_SECTOR)
                printf("%02X ", buf[i + j]);
            else
                printf("   ");

            /* chia cột cho dễ nhìn */
            if (j == 7) printf(" ");
        }

        /* ASCII (bên phải) */
        printf(" |");
        for (uint8_t j = 0; j < 16 && (i + j) < SFAT_BYTES_PER_SECTOR; j++) {
            uint8_t c = buf[i + j];
            printf("%c", (c >= 32 && c <= 126) ? c : '.');
        }
        printf("|\n");
    }
}

void sfat_dump_sector(uint32_t sector_index)
{
    uint8_t data_buf[SFAT_BYTES_PER_SECTOR];
    FTL_Read(SFAT_START_DATA + sector_index, data_buf);
    const uint8_t *addr = data_buf;

    // Header
    printf("       ");
    for (int i = 0; i < 16; i++) {
        printf("%02X ", i);
    }
    printf("\n       -----------------------------------------------       \n");

    for (int row = 0; row < SFAT_BYTES_PER_SECTOR; row += 16)
    {
        // Offset trong sector
        printf("%04X:  ", row);

        // In hex (16 byte)
        for (int i = 0; i < 16; i++) {
            printf("%02X ", addr[row + i]);
        }

        printf(" |");

        // In ASCII
        for (int i = 0; i < 16; i++) {
            uint8_t c = addr[row + i];
            if (isprint(c))
                putchar(c);
            else
                putchar('.');
        }

        printf("|\n");
    }
}