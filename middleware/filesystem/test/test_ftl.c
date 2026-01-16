static uint8_t buf[PAGE_SIZE];

void fill_buffer(uint8_t v) {
    memset(buf, v, PAGE_SIZE);
}

void start_tests(void) {
    AT45DB_Init();
    FTL_Init();

    printf("=== TEST 1: ghi logical pages lien tiep ===\n");

    fill_buffer((uint8_t)'1');
    FTL_Write(0, buf);   // LP 0

    fill_buffer(0xA2);
    FTL_Write(1, buf);   // LP 1

    fill_buffer(0xA3);
    FTL_Write(2, buf);   // LP 2

    FTL_DumpMapping(0, 4);
    FTL_DumpBlockInfo(0, 2);

    printf("\n=== TEST 2: update logical page 1 ===\n");

    fill_buffer(0xB1);
    FTL_Write(1, buf);   // update LP 1 (out-of-place)

    FTL_DumpMapping(0, 4);
    FTL_DumpBlockInfo(0, 2);

    printf("\n=== TEST 3: ghi full block ===\n");

    for (int i = 3; i < 8; i++) {
        fill_buffer(0xC0 + i);
        FTL_Write(i, buf);
    }

    FTL_DumpMapping(0, 8);
    FTL_DumpBlockInfo(0, 2);

    printf("\n=== TEST 4: ghi them -> sang block moi ===\n");

    fill_buffer(0xD0);
    FTL_Write(8, buf);

    FTL_DumpMapping(0, 10);
    FTL_DumpBlockInfo(0, 3);

    printf("\n=== TEST 5: Dump data logical page ===\n");
    FTL_DumpLogicalPage(0);

    printf("\n==> DONE!\n");
}