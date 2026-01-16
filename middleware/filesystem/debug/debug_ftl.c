#include "debug_ftl.h"

/**
 * @brief In ra bảng mapping từ logical page sang physical page
 * @param from start logical page
 * @param to end logical page 
 * 
 * @note logical page = from -> (to - 1)
 */
void FTL_DumpMapping(uint16_t from, uint16_t to) {
    printf("L2P mapping:\n");
    for (uint16_t i = from; i < to; i++) {
        if (l2p[i] != INVALID_PAGE) {
            printf("  LP %3d -> PP %4d (Block %d, Page %d)\n",
                   i,
                   l2p[i],
                   PAGE_TO_BLOCK(l2p[i]),
                   PAGE_OFFSET_IN_BLOCK(l2p[i]));
        }
    }
}

/**
 * @brief In ra thông tin các block
 * @param from start block
 * @param to end block
 * 
 * @note block = from -> (to - 1)
 * 
 * - Hiện tại block data là từ 4 -> sau
 */
void FTL_DumpBlockInfo(uint16_t from, uint16_t to) {
    printf("\n* Block info:\n");
    for (uint16_t b = from; b < to; b++) {

        printf("  Block %3d: free= %d invalid= %d erase= %d\n",
               b,
               ftl.BlockAllocationTable[b].free_pages,
               ftl.BlockAllocationTable[b].invalid_pages,
               ftl.BlockAllocationTable[b].erase_count);
    }
}

void FTL_DumpLogicalPage(uint16_t logical_page) {
    if (logical_page >= TOTAL_LOGICAL_PAGES) {
        printf("Invalid logical page %d\n", logical_page);
        return;
    }

    uint16_t pp = l2p[logical_page];
    if (pp == INVALID_PAGE) {
        printf("Logical page %d is not mapped\n", logical_page);
        return;
    }

    printf("LP   %d -> pp   %d (Block %d, Page %d)\n",
           logical_page, pp, PAGE_TO_BLOCK(pp), PAGE_OFFSET_IN_BLOCK(pp));

    uint8_t *data = &FLASH[pp * PAGE_SIZE];

    // In dòng thứ tự byte (0..15)
    printf("\n");
    printf("     ");
    for (int col = 0; col < 16; col++) {
        printf(" %02X", col);
    }
    printf("\n");

    // Dump theo 16 byte 1 dòng
    for (int row = 0; row < PAGE_SIZE; row += 16) {
        printf("%04X  ", row); // offset trong page

        // Hex
        for (int col = 0; col < 16; col++) {
            if (row + col < PAGE_SIZE)
                printf("%02X ", data[row + col]);
            else
                printf("   ");
        }

        // ASCII
        printf(" |");
        for (int col = 0; col < 16 && (row + col < PAGE_SIZE); col++) {
            uint8_t c = data[row + col];
            if (c >= 32 && c <= 126)  // printable ASCII
                printf("%c", c);
            else
                printf(".");
        }
        printf("|\n");
    }
}

/**
 * @brief Dump toàn bộ bytes trong 1 page 
 * @param physical_page Vị trí page vật lí
 */
void dump_flash_page( uint16_t physical_page )
{
    if (physical_page >= AT45_TOTAL_PAGES) {
        printf("Invalid page %u\n", physical_page);
        return;
    }

    uint32_t base = physical_page * AT45_PAGE_SIZE;

    printf("      ");
    for (uint8_t i = 0; i < 16; i++) {
        printf("%02X ", i);
    }
    printf("\n");

    for (uint16_t i = 0; i < AT45_PAGE_SIZE; i += 16) {

        /* Offset */
        printf("%04X  ", i);

        /* HEX */
        for (uint8_t j = 0; j < 16; j++) {
            printf("%02X ", FLASH[base + i + j]);
        }

        printf(" | ");

        /* ASCII */
        for (uint8_t j = 0; j < 16; j++) {
            uint8_t b = FLASH[base + i + j];
            if (b >= 0x20 && b <= 0x7E)
                printf("%c", b);
            else
                printf(".");
        }

        printf("\n");
    }

}

/**
 * @brief Dùng để thông tin của một physical page dang chứa metadata (ở đây là range và mapping)
 */
void dump_mapping_page(uint32_t phy_page)
{
    mapping_page_t page;
    uint8_t buf_page[512];

    memset(buf_page, 0, sizeof(buf_page));

    /* 1. Read raw page from flash */
    AT45DB_ReadPage(phy_page, buf_page, NULL);
    memcpy(&page, buf_page, sizeof(mapping_page_t));

    /* 2. Calculate logical page range */
    uint32_t lpn_start = page.range_index * PAGES_PER_RANGE;
    uint32_t lpn_end   = lpn_start + PAGES_PER_RANGE - 1;

    /* 3. Print header info */
    printf("\n* Dump mapping page\n");
    printf("Physical page : %lu\n", (unsigned long)phy_page);
    printf("Magic         : 0x%04X\n", page.magic);
    printf("Range index   : %u (lp %lu -> %lu)\n",
           page.range_index,
           (unsigned long)lpn_start,
           (unsigned long)lpn_end);
    printf("Entries count : %u\n", PAGES_PER_RANGE);
    printf("----------------------\n");

    /* 4. Print mapping table */
    for (uint16_t i = 0; i < PAGES_PER_RANGE; i++)
    {
        printf("lp %03lu -> pp %u\n",
               (unsigned long)(lpn_start + i),
               page.entries[i]);
    }
}


void FTL_PrintCache(void) 
{
    cachePrint();
}