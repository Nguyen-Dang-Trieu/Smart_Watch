
void test_shrink() {
    printf("\n=== Test: Shrink ===\n");

    sfat_create("t1.txt");
    uint8_t fd = sfat_open("t1.txt", SFAT_MODE_W);

    char buf1[600];
    memset(buf1, '1', 600);

    printf("\n=== Before ===\n");
    sfat_write(fd, buf1, 600);
    sfat_dump_fat_range(0,5);
    sfat_dump_sector(0);
    sfat_dump_sector(1);

    char buf2[512];
    memset(buf2, '2', 512);
    printf("\n=== After ===\n");
    sfat_write(fd, buf2, 512);
    sfat_dump_fat_range(0,5);
    sfat_dump_sector(0);
    sfat_dump_sector(1);

    sfat_state();
}

void test_read() {
    sfat_create("read.txt");
    uint8_t fd = sfat_open("read.txt", SFAT_MODE_W);

    char buf1[1024];
    memset(buf1, '1', 512);
    memset(buf1 + 512, '2', 512);
    sfat_write(fd, buf1, 1024);

    //sfat_dump_fat_range(0,4);
    //sfat_dump_sector(0);
    //sfat_dump_sector(1);

    sfat_close(fd);

    fd = sfat_open("read.txt", SFAT_MODE_R);
    char buf2[101];
    buf2[100] = '\0';
    int8_t size = 0; // kích thước mỗi lần đọc
    for(int i = 0; i < 13; i++) {
        memset(buf2, '.', 100);
        size = sfat_read(fd, buf2, 100);
        printf("Size: %d - %s\n", size, buf2);
    }
    
}

void test_mode_a() {
    printf("\n --- --- --- TEST WRITE MODE A --- --- --- \n\n");

    sfat_create("test.txt");
    sfat_create("test2.txt");
    uint8_t fd1 = sfat_open("test.txt", SFAT_MODE_W);
    uint8_t fd2 = sfat_open("test2.txt", SFAT_MODE_W);

    char buf1[200];
    memset(buf1, '1', 200);
    sfat_write(fd1, buf1, 200);
    sfat_close(fd1);

    char buf2[200];
    memset(buf2, '2', 200);
    sfat_write(fd2, buf2, 200);
    sfat_close(fd2);

    fd1 = sfat_open("test.txt", SFAT_MODE_A);
    char buf3[500];
    memset(buf3, '3', 500);
    sfat_write(fd1, buf3, 500);
    sfat_close(fd1);

    fd1 = sfat_open("test.txt", SFAT_MODE_R);
    char buf4[301];
    buf4[301] = '\0';
    uint32_t bytes_to_read = sfat_read(fd1, buf4, 300);
    printf("So luong byte da doc duoc: %d, \n%s\n", bytes_to_read, buf4);
    
    sfat_state();
    sfat_dump_directory();
}

void test_write_mode_w_if(void) {

    printf("\n --- --- --- TEST WRITE MODE W: branch = if --- --- --- \n\n");

    sfat_create("test.txt");

    // Mo lan 1
    uint8_t fd = sfat_open("test.txt", SFAT_MODE_W);

    char buf1[300];
    memset(buf1, '1', 300);
    sfat_write(fd, buf1, 300);
    sfat_close(fd);

    sfat_dump_fat_range(0,5);
    sfat_dump_sector(0);

    // Mo lan 2
    fd = sfat_open("test.txt", SFAT_MODE_W);
    char buf2[1025];
    memset(buf2, '2', 1025);
    sfat_write(fd, buf2, 1025);
    sfat_close(fd);

    sfat_state();
    sfat_dump_fat_range(0,5);
    sfat_dump_sector(0);
    sfat_dump_sector(1);
    sfat_dump_sector(2);
} 

void test_write_mode_w_elif(void) {
    sfat_create("test.txt");

    // Mo lan 1
    uint8_t fd = sfat_open("test.txt", SFAT_MODE_W);

    char buf1[700];
    memset(buf1, '1', 700);
    sfat_write(fd, buf1, 700);
    sfat_close(fd);

    sfat_state();
    sfat_dump_fat_range(0,5);

    // Mo lan 2
    fd = sfat_open("test.txt", SFAT_MODE_W);
    char buf2[300];
    memset(buf2, '2', 300);
    sfat_write(fd, buf2, 300);
    sfat_close(fd);

    sfat_state();
    sfat_dump_fat_range(0,5);
    sfat_dump_sector(0);
    sfat_dump_sector(1);
}

void test_write_mode_w_else(void) {
    sfat_create("test.txt");

    // Mo lan 1
    uint8_t fd = sfat_open("test.txt", SFAT_MODE_W);

    char buf1[300];
    memset(buf1, '1', 300);
    sfat_write(fd, buf1, 300);
    sfat_close(fd);

    sfat_state();
    sfat_dump_fat_range(0,5);

    // Mo lan 2
    fd = sfat_open("test.txt", SFAT_MODE_W);
    char buf2[300];
    memset(buf2, '2', 300);
    sfat_write(fd, buf2, 300);
    sfat_close(fd);

    sfat_state();
    sfat_dump_fat_range(0,5);
    sfat_dump_sector(0);
    sfat_dump_sector(1);
}

void test_delete(void) {
    printf("\n=== Test: Delete ===\n");

    sfat_create("t1.txt");
    sfat_create("t2.txt");
    sfat_create("t3.txt");
    sfat_delete("t2.txt");
    sfat_create("t4.txt");
    sfat_create("t5.txt");
    sfat_delete("t3.txt");
    sfat_delete("t1.txt");
    sfat_dump_directory();
}
