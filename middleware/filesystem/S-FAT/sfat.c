#include "sfat.h"

// File discriptor table
static FD_t       fd_table[SFAT_MAX_OPEN_FILES] = {0};

/**
 * Cache metadata của các file đang được mở.
 * 
 * Ownership rule:
 * - Slot i trong recently_opened_files[i] chỉ hợp lệ khi fd_table[i].used == true.
 * - Vòng đời metadata được quản lý hoàn toàn bởi fd_table.
 * 
 * Note:
 * - Mỗi slot ánh xạ 1–1 với fd_table[i].
 * - Khi file được đóng (fd_table[i].used = false), metadata tại slot i được coi là invalid.
 */
static DirEntry_t recently_opened_files[SFAT_MAX_OPEN_FILES] = {0};

// Thông số SFAT mặc định
SuperBlock_t superblock = {
    .total_sectors        = SFAT_TOTAL_SECTORS,
    .sectors_per_cluster  = SFAT_SECTORS_PER_CLUSTER,
    .bytes_per_sector     = SFAT_BYTES_PER_SECTOR,
    .available_sectors    = 4047,
    .total_direntries     = SFAT_MAX_DIRENTRIES,
    .available_direntries = 32,
    .fs_type              = 0xFA, // SFAT
    .reserved             = {0},
    .label                = "Simple-FAT"
};


/**
 * @brief Lấy giá trị trong FAT table.
 * @param fat_entry Vị trí fat (cluster index)
 * @retval Clusuter tiếp theo trong chuỗi của file.
 * 
 * - SFAT_END_CLUSTER: cluster cuối cùng
 * 
 * - SFAT_FREE_CLUSTER: cluster free
 */
static uint16_t sfat_fat_entry_get(uint16_t fat_entry) {
    uint8_t  fat_buf[SFAT_BYTES_PER_SECTOR];
    uint16_t *fat_ptr = (uint16_t *)fat_buf;

    uint16_t sector = GET_FAT_SECTOR(fat_entry); // sector chứa fat_entry
    uint16_t offset = GET_FAT_OFFSET(fat_entry); // Vị trí entry đó nằm trong sector hiện tại
    sfat_read_sector(SFAT_START_FAT + sector, fat_buf);

    return fat_ptr[offset];
}

/**
 * @brief Ghi giá trị vào FAT table tại vị trí cluster index
 * @param fat Cluster index cần ghi
 * @param value Giá trị mới (cluster tiếp theo, SFAT_END_CLUSTER hoặc SFAT_FREE_CLUSTER)
 */
static void sfat_fat_entry_set(uint16_t fat, uint16_t value) {
    uint16_t sector = GET_FAT_SECTOR(fat);
    uint16_t offset = GET_FAT_OFFSET(fat);

    // Đọc sector FAT từ flash
    uint8_t fat_buf[SFAT_BYTES_PER_SECTOR];
    uint16_t *fat_ptr = (uint16_t *)fat_buf;
    sfat_read_sector(SFAT_START_FAT + sector, fat_buf);
    
    // Ghi giá trị mới
    fat_ptr[offset] = value;
    sfat_write_sector(SFAT_START_FAT + sector, fat_buf);
}

/**
 * @brief Free các cluster không dùng nữa
 * @param start_cluster cluster bắt đầu (chỉ số fat_entry)
 * @retval Số lượng cluster được free
 */
static uint16_t free_cluster_chain(uint16_t start_cluster) {
    /* Kiểm tra tham số đầu vào */
    if(start_cluster >= SFAT_TOTAL_CLUSTERS)
        return 0;

    uint16_t cluster_cur  = start_cluster; 
    uint16_t cluster_next;
    uint8_t  freed = 0;

    while (cluster_cur != SFAT_END_CLUSTER) {
        cluster_next = sfat_fat_entry_get(cluster_cur);        // cluster next 
        sfat_fat_entry_set(cluster_cur, SFAT_FREE_CLUSTER);    // free cluster current
        freed++;

        cluster_cur = cluster_next;
        if (cluster_cur >= SFAT_TOTAL_CLUSTERS)
            break;
    }

    return freed;
}

void sfat_mount(void) {
    uint8_t buf[SFAT_BYTES_PER_SECTOR];
    sfat_read_sector(0, buf);
    memcpy(&superblock, buf, sizeof(SuperBlock_t));
}

void sfat_umount(void) {
    uint8_t buf[SFAT_BYTES_PER_SECTOR];
    memcpy(buf, &superblock, sizeof(SuperBlock_t));
    sfat_write_sector(0, buf);
}

void sfat_state() {
    uint32_t sectors_for_information = 49; // superblock + FAT + directory
    //  Tổng số sectors đã dùng = superblock.total_sectors - superblock.available_sectors;
    uint32_t sectors_for_data   = (superblock.total_sectors - superblock.available_sectors) - sectors_for_information;

    uint32_t used_entry   = superblock.total_direntries - superblock.available_direntries;

    // In ra thông tin superblock
    printf("              Total Sectors: %u\n", superblock.total_sectors);
    printf("        Sectors per Cluster: %u\n", superblock.sectors_per_cluster);
    printf("           Bytes per Sector: %u\n", superblock.bytes_per_sector);
    printf("          Available Sectors: %u (Used %u sectors for data) \n", superblock.available_sectors, sectors_for_data);
    printf("    Total Directory Entries: %u\n", superblock.total_direntries);
    printf("Available Directory Entries: %u (Used %u Entries) \n", superblock.available_direntries, used_entry);
    printf("            Filesystem Type: 0x%X\n", superblock.fs_type);
    printf("                      Label: %s\n\n", superblock.label);
}


// ------ API cho thao tác file trong SFAT Filesystem ------
int8_t sfat_create(const char *name) {
    uint8_t dir_buffer[SFAT_BYTES_PER_SECTOR];

    sfat_read_sector(SFAT_START_DIR, dir_buffer);
     DirEntry_t *entries = (DirEntry_t *)dir_buffer;

    for(uint8_t entry = 0; entry < SFAT_MAX_DIRENTRIES; entry++) {
        if(entries[entry].name[0] == 0 || entries[entry].name[0] == (uint8_t)'?') {
            memset(&entries[entry], 0, sizeof(DirEntry_t));
            strncpy((char *)entries[entry].name, name, 10); 
            entries[entry].fat_entry = SFAT_FREE_CLUSTER;

            sfat_write_sector(SFAT_START_DIR, dir_buffer);
            superblock.available_direntries--;

            return SFAT_OK;
        }
    }

    return SFAT_ERR_NODATA; // Không còn chỗ tạo file

    /**
     * Problem
     * - Nếu cứ tạo mỗi file như này rồi ghi xuống flash thì sẽ rát tốn hiệu suất. Bởi vì ta đâu có biết trong tương lai
     * có tạo 2 file nào liên tiếp trên cùng một sector hay không ?
     */

    /**
     * Cần lưu ý:
     *  Hiện tại do đang mô phỏng, nên direntries_ptr sẽ được lưu lại do chạy liên tục trong main().
     *  Tuy nhiên trong thực tế, sau khi sfat_umount() thì con trỏ này sẽ bị mất.
     *  => Do đó cần có cơ chế để xác định direntries_ptr khi sfat_mount() được gọi.
     */
}

/**
 * Thuật toán để xóa một file trong hệ thống.
 * 1. Tra cứu tên file trong thư mục -> Nếu không tìm thấy trả về lỗi và dừng lại.
 * 2. Đặt kí tự đâu tiên của tên file thành "?".
 * 3. Đặt tất cả các FAT entry của file thành 0.
 * 4. Cập nhập các thông tin trong superblock.
 */
int8_t sfat_delete(const char *name) {
    uint8_t dir_buf[SFAT_BYTES_PER_SECTOR];
    DirEntry_t *found = NULL;

    //* 1. Tìm directory entry */
    sfat_read_sector(SFAT_START_DIR, dir_buf);
    DirEntry_t *entries = (DirEntry_t *)dir_buf;

    for (uint8_t entry = 0; entry < SFAT_MAX_DIRENTRIES; entry++) {
         if (strncmp((char *)entries[entry].name, name, 10) == 0) { // Compare names to find the file
            found = &entries[entry];
            goto FOUND;
        }
    }

FOUND:

    /* 2. Free FAT chain */
    int16_t  fat = found->fat_entry; // cluster start
    uint16_t freed;

    if(found->size != 0)
        freed = free_cluster_chain(fat);

    /* 3. Mark directory entry deleted */
    found->name[0] = '?';
    found->fat_entry = SFAT_FREE_CLUSTER;

    sfat_write_sector(SFAT_START_DIR, dir_buf);

    /* 4. Update superblock */
    superblock.available_direntries++;
    superblock.available_sectors += freed;

    return SFAT_OK;
} 

/**
 * - Tìm thông tin của file đã mở lần nào gần đây chưa trong recently_opened_files[].
 * - Nếu có, sử dụng thông tin đó để tạo file descriptor mới trong fd_table[].
 * - Nếu không, tìm file trong directory area. Và lưu thông tin vào recently_opened_files[] để dùng cho 
 * lần mở sau.
 */

int8_t sfat_open(const char *filename, uint8_t mode) 
{
    int8_t  metadata_pos = 0; // Vị trí metadata 
    /* Case: Tìm trong cache (recently_opened_files) */
    metadata_pos = get_metadata_from_cache(recently_opened_files, filename);

    /* Case: Nếu trong cache không có, tìm metadata của file trong directory*/
    if(metadata_pos == -1)
    {
        metadata_pos = get_metadata_from_dir(fd_table, recently_opened_files, filename);
    }

    if(metadata_pos < 0) {
        return SFAT_ERR_NOENT; // The file does not exist
    }

    /* Tìm vị trí trong file descriptor table còn trống */
    int8_t position_fd = find_free_fd(fd_table);

    if(position_fd == -1) {
        return SFAT_ERR_NOMEM; // không còn slot fd trống
    } else {
        fd_table[position_fd].dir_entry = &recently_opened_files[metadata_pos];
        fd_table[position_fd].offset    = 0;
        fd_table[position_fd].mode      = mode;
        fd_table[position_fd].used      = true;
    }

    return position_fd;
} 


/**
 * @brief Tìm và cấp phát một chuỗi cluster free
 * 
 * - Duyệt từng sector trong FAT để tìm các cluster rỗng (SFAT_FREE_CLUSTER).
 * 
 * @param start_fat_entry Fat entry bắt đầu chuỗi cluster free
 * @param cluster_needed  Số cluster cần cấp phát
 * @retval Số cluster thực tế đã cấp phát (có thể < cluster_needed nếu không đủ).
 */
uint8_t alloc_cluster(uint16_t *start_fat_entry, uint8_t cluster_needed) 
{
	uint8_t  allocated    = 0;                   // Số lượng các cluster free đã dược cấp phát
    if(cluster_needed > MAX_ALLOC) cluster_needed = MAX_ALLOC;
	uint16_t available_clusters[cluster_needed]; // Mảng chứa các fat entry chưa được cấp phát (nghĩa là cluster tương ứng chưa được dùng)
    
    uint8_t  fat_buf[SFAT_BYTES_PER_SECTOR];  
    uint16_t *fat_entries = (uint16_t *)fat_buf;


    // Duyệt Fat table trong từng sector của toàn bộ Fat
    for (uint16_t sector = 0; sector < SFAT_FAT_SECTORS && allocated < cluster_needed; sector++) 
    {
        sfat_read_sector(SFAT_START_FAT + sector, fat_buf);

	    for(uint16_t fat_entry_cur = 0; fat_entry_cur < SFAT_FAT_ENTRIES_PER_SECTOR && allocated < cluster_needed; 
            fat_entry_cur++) 
        {
		    if(fat_entries[fat_entry_cur] == SFAT_FREE_CLUSTER) 
            {	
		    	available_clusters[allocated] = sector * SFAT_FAT_ENTRIES_PER_SECTOR + fat_entry_cur;
                allocated++; 
		    }			 
	    }
    }

    if(allocated == 0) return 0; // Các file đã sử dụng hết cluster trong filesystem

    /* Liên kết các cluster và write to flash */
    int16_t current_sector = -1;    // current_sector empty
    for(uint8_t i = 0; i < allocated; i++) {
        uint16_t target_sector = GET_FAT_SECTOR(available_clusters[i]); // Tìm sector chứa fat entry  
        uint16_t offset        = GET_FAT_OFFSET(available_clusters[i]); // Vị trí của fat entry trong sector đó

        // Nếu chuyển sang sector mới, hãy ghi sector cũ và đọc sector mới
        if (target_sector != current_sector) {
            if (current_sector != -1) {
                sfat_write_sector(SFAT_START_FAT + current_sector, fat_buf);
            }
            sfat_read_sector(SFAT_START_FAT + target_sector, fat_buf);
            current_sector = target_sector;
        }

        if (i == allocated - 1) {
            fat_entries[offset] = SFAT_END_CLUSTER; // fat entry của cluster cuối dc gán SFAT_END_CLUSTER
        } else {
            fat_entries[offset] = available_clusters[i + 1];
        }
    }

    // Ghi sector cuối cùng còn đang giữ trong buffer
    if (current_sector != -1) {
        sfat_write_sector(SFAT_START_FAT + current_sector, fat_buf);
    }

    if(start_fat_entry != 0) *start_fat_entry = available_clusters[0];

    return allocated;
}

// /**
//  * @brief Đếm số cluster đang được sử dụng bởi file
//  * @param file Con trỏ tới DirEntry_t của file
//  * @retval Số cluster hiện tại mà file đang sử dụng
//  */
uint8_t clusters_of_file(DirEntry_t *file) {
    uint8_t current_clusters = CALC_CLUSTERS(file->size);
    return current_clusters;
}

/*
 * @brief Tính số cluster cần thiết để ghi dữ liệu vào file (Mode = SFAT_MODE_W)
 * @param file Con trỏ tới DirEntry_t của file
 * @param size_data Kích thước dữ liệu cần ghi (tính bằng byte)
 * @retval uint8_t - Số cluster cần dùng để ghi toàn bộ dữ liệu
 */

 // ⚠️ cẩn cải thiện hơn nữa

static uint8_t mode_w(DirEntry_t *file, uint32_t size_data) {
    uint16_t cluster_needed       = CALC_CLUSTERS(size_data);
    uint8_t  clusters_Of_File     = clusters_of_file(file);     
       
    if(clusters_Of_File < cluster_needed) {
		uint16_t new_clusters_head    = 0;  // cluster đầu tiên trong chuỗi cluster mới được cấp phát
        uint8_t  needed_more = cluster_needed - clusters_Of_File;
        uint8_t  free_found  = alloc_cluster(&new_clusters_head, needed_more); 
		
		if(free_found == needed_more) 
        {
            uint16_t cur = file->fat_entry;

            uint8_t   fat_buf[SFAT_BYTES_PER_SECTOR];
            uint16_t *fat_ptr = (uint16_t *)fat_buf;
            uint16_t  last_cluster = 0;
            int8_t    current_sector = -1;
            
            while(cur != SFAT_END_CLUSTER) {
                last_cluster = cur;
                uint8_t target_sector = GET_FAT_SECTOR(cur);
                uint8_t offset        = GET_FAT_OFFSET(cur);
                if (target_sector != current_sector) {
                    current_sector = target_sector;
                    sfat_read_sector(SFAT_START_FAT + target_sector, fat_buf);
                }
				cur = fat_ptr[offset]; 
            }

            /* Liên kết chuỗi cluster mới vào cuối file hiện tại */
            // Đảm bảo sector của last_cluster chứa trong fat_buf
            uint16_t last_sector = last_cluster / SFAT_FAT_ENTRIES_PER_SECTOR;
            if(last_sector != current_sector)
            {
                sfat_read_sector(SFAT_START_FAT + last_sector, fat_buf);
            }


            fat_ptr[last_cluster % SFAT_FAT_ENTRIES_PER_SECTOR] = new_clusters_head;
            sfat_write_sector(SFAT_START_FAT + last_sector, fat_buf); // Important: Write back to flash

            /* Update superblock */
            superblock.available_sectors -= free_found;
            return cluster_needed;
        }

        printf("\nKhong du cluster free!\n");
        return 0; // Không đủ free cluster
    }
    else if(clusters_Of_File > cluster_needed) 
    {
        uint8_t fat_buf[SFAT_BYTES_PER_SECTOR];
        uint16_t *fat_ptr = (uint16_t *)fat_buf;

        uint16_t cur = file->fat_entry;
        int16_t  current_sector = -1;
        for(uint8_t i = 1; i < cluster_needed; i++) 
        {
            uint16_t target_sector = GET_FAT_SECTOR(cur);
            uint16_t offset        = GET_FAT_OFFSET(cur);
            if (target_sector != current_sector) {
                current_sector = target_sector;
                sfat_read_sector(SFAT_START_FAT + target_sector, fat_buf);
            }

            cur = fat_ptr[offset];
        }

        if(current_sector == -1) {
            current_sector = GET_FAT_SECTOR(cur);
            sfat_read_sector(SFAT_START_FAT + current_sector, fat_buf);
        }

        uint16_t offset = GET_FAT_OFFSET(cur);
        uint16_t cluster_to_free = fat_ptr[offset]; // Lưu lại cluster đầu tiên của chuỗi cần xóa
        if (cluster_to_free != SFAT_END_CLUSTER) 
        {
            fat_ptr[offset] = SFAT_END_CLUSTER;
            sfat_write_sector(SFAT_START_FAT + current_sector, fat_buf);

            // Giải phóng chuỗi bắt đầu từ node tiếp theo
            free_cluster_chain(cluster_to_free);
        }

        /* Update superblock */
        superblock.available_sectors += (clusters_Of_File - cluster_needed);
        return cluster_needed;
    } 
    else { // clusters_Of_File = cluster_need
        // Do nothing -> Giữ nguyên mọi thông tin về file
        return cluster_needed;
    }
}

/**
 * @brief Ghi dữ liệu mới vào cuối file hiện tại
 * @param fat_entry cluster đầu tiên của file
 * @param size_data kích thước dữ liệu cần ghi
 * @retval Số lượng cluster cần phải ghi
 */
static uint8_t mode_a(DirEntry_t* file, uint32_t size_data) {
    uint8_t clustersOfFile   = clusters_of_file(file); 
    uint8_t required_cluster = CALC_CLUSTERS(file->size + size_data);
    
    if(required_cluster <= clustersOfFile) {
        return required_cluster; // Đã đủ dùng, không cần thiết cấp phát nữa
    }
    
    uint8_t  missing_cluster    = required_cluster - clustersOfFile; // Số cluster còn thiếu cần cấp phát
    uint16_t new_clusters_head  = 0;
    uint8_t  numberOfNewCluster = alloc_cluster(&new_clusters_head, missing_cluster);
    if(numberOfNewCluster != missing_cluster) {
        return 0; // không đủ cluster
    }

    uint8_t   fat_buf[SFAT_BYTES_PER_SECTOR]; 
    uint16_t *fat_ptr = (uint16_t *)fat_buf;
    uint16_t  cur = file->fat_entry;
    int8_t    current_sector = -1;
    uint16_t  last = 0;
    while(cur != SFAT_END_CLUSTER) {
        last = cur;
        uint16_t target_sector = GET_FAT_SECTOR(cur);
        uint16_t offset        = GET_FAT_OFFSET(cur);

        // Đọc sector mới khi cần
        if (target_sector != current_sector) {
            current_sector = target_sector;
            sfat_read_sector(SFAT_START_FAT + target_sector, fat_buf);
        }

        cur = fat_ptr[offset];
    }

    // Đảm bảo sector của last_cluster chứa trong fat_buf
    uint16_t last_sector = GET_FAT_SECTOR(last); // Sector chứa last 
    if(last_sector != current_sector)
    {
        sfat_read_sector(SFAT_START_FAT + last_sector, fat_buf);
    }
    fat_ptr[GET_FAT_OFFSET(last)] = new_clusters_head;

    // Write back to flash
    sfat_write_sector(SFAT_START_FAT + last_sector, fat_buf);

    // Update superblock
    superblock.available_sectors -= numberOfNewCluster;

    return missing_cluster;
}

// /**
//  * @brief Có thể write vào offset của file
//  */
// void mode_rp(void) {
//     // TODO
// }

/**
 * @brief 
 * 
 * Hàm này chỉ thực hiện việc copy dữ liệu từ buffer vào các data block
 * theo thứ tự cluster trong bảng FAT, bắt đầu từ cluster chỉ định.
 * 
 * @param start_cluster Cluster bắt đầu để ghi dữ liệu (chỉ số trong fat)
 * @param offset_cluster Vị trí offset bên trong cluster đầu tiên
 * @param buf Buffer chứa dữ liệu cần ghi
 * @param size_buf Số bytes cần ghi
 * @retval Số bytes đã ghi thành công.
 * @note Ghi dữ liệu vào các cluster đã xác định sẵn và không được phép chỉnh sửa hay xóa cluster nào!
 */
static size_t write_clusters(uint16_t start_cluster, uint16_t offset_cluster, const char *buf, size_t size_buf) {
    /**
     * Flash recording method
     *  READ PAGE -> MODIFY PAGE -> WRITE PAGE
     */

    uint16_t chunk = 0;     // Số lượng dữ liệu cần ghi trên mỗi cluster
    uint16_t written = 0;   // Bytes dữ liệu đã write
    uint16_t cluster_to_write = start_cluster;
    uint8_t  page_buf[SFAT_BYTES_PER_CLUSTER]; 

    while(size_buf > 0) {
        /* 1. Tính số lượng byte ghi trong page hiện tại */
        chunk = SFAT_BYTES_PER_CLUSTER - offset_cluster;
        if(chunk > size_buf) chunk = size_buf;

        /* 2. Đọc dữ liệu của page hiện tại */
        /** Tiết kiệm thời gian đọc
         * TH offset_cluster khác 0 -> Ta cần phải đọc page lên tính vị trí offset để modify dữ liệu chỉnh xác.
         * TH offset_cluster = 0 -> Toàn bộ page đều phải modify. Không cần read.
         */
        if(chunk != SFAT_BYTES_PER_CLUSTER) 
            sfat_read_sector(SFAT_START_DATA + cluster_to_write, page_buf); 

        /* 3. Modify dữ liệu chứa trong page_buf */
        memcpy(&page_buf[offset_cluster], buf + written, chunk);

        /* 4. Write page */
        sfat_write_sector(SFAT_START_DATA + cluster_to_write, page_buf);

        // Update state
        written          += chunk;
        size_buf         -= chunk;
        offset_cluster    = 0;
        
        // Di chuyển sang cluster tiếp theo
        if(size_buf > 0) {
            cluster_to_write = sfat_fat_entry_get(cluster_to_write); 
            if(cluster_to_write == SFAT_END_CLUSTER) break;
        }
    }

    return written;
}

int8_t sfat_write(int fd, const void *buf, uint32_t size) {
    /* 1. Kiểm tra đầu vào */
    if(fd >= SFAT_MAX_OPEN_FILES || fd_table[fd].used == false) {
        return SFAT_ERR_BAD_FD; 
    }

    if(buf == NULL || size == 0) {
        return SFAT_ERR_NULL_PTR; 
    }

    /* 2. Lấy thông tin của file khi mở */   
    DirEntry_t* file       = fd_table[fd].dir_entry; 
    uint32_t    offset     = fd_table[fd].offset;
    uint8_t     mode       = fd_table[fd].mode;
    uint16_t    start_cluster = 0;

    // Cấp phát cluster
    if(file->size == 0) { // Chưa có cluster được cấp phát, mới tạo file.
        uint16_t cluster_needed = SFAT_CLUSTERS_FOR_SIZE(size); // Số lượng cluster cần thiết để lưu trữ dữ liệu
        
        /* Kiểm tra số lượng cluster còn lại cho cấp phát */
        if(cluster_needed > superblock.available_sectors) { 
            return SFAT_ERR_NODATA; // Mã lỗi báo không đủ sector
        }

        uint16_t start_fat_entry = 0;
        uint8_t  free_found = alloc_cluster(&start_fat_entry, cluster_needed);

        if(free_found == cluster_needed) {
            file->fat_entry = start_fat_entry;
        } else {
            return SFAT_ERR_NODATA; // Free cluster không đủ cho file.
        }

        /* Cập nhập lại thông tin trong superblock */
        superblock.available_sectors -= free_found; 
        
        /* Chuẩn bị tham số đầu vào cho quá trình ghi */
        offset        = 0;
        start_cluster = file->fat_entry;
    }
    else { // Đã có cluster được cấp phát, file đã được sử dụng trước đó.
        switch (mode)
        {
        case SFAT_MODE_W:
            mode_w(file, size);
            offset        = 0;
            start_cluster = file->fat_entry;
            break;
        case SFAT_MODE_A:
            mode_a(file, size);
            offset        = GET_FAT_OFFSET(file->size);
            start_cluster = GET_FAT_SECTOR(file->size);
            break;
        case SFAT_MODE_RP:
            break;

        default:
            break;
        }
    }

    // Ghi dữ liệu vào các data block
    uint32_t bytes_written = write_clusters(start_cluster, offset, buf, size);
    file->size += bytes_written; // Cập nhập lại kích thước của file
    
    return SFAT_OK;
}

uint32_t sfat_read(uint8_t fd, void *buf, uint32_t size) {
    if (fd >= SFAT_MAX_OPEN_FILES || !fd_table[fd].used) // Lỗi không tìm thấy file đang open
        return SFAT_ERR_NOT_OPEN;

    if(fd_table[fd].mode != SFAT_MODE_R)   // Lỗi file open nhưng không ở mode "r"
        return SFAT_ERR_BAD_MODE; 
        
    if(buf == NULL || size == 0)          // Lỗi buffer và kích thước
        return SFAT_ERR_NULL_PTR; 

    DirEntry_t* file   = fd_table[fd].dir_entry; 
    uint32_t    offset = fd_table[fd].offset;   // Khi open file -> offset = 0

    // Đã read() đến EOF trong lần trước.
    if (offset >= file->size)
        return SFAT_ERR_EOF;

    // Kiểm tra số lượng data yêu cầu có vượt quá data còn trong file hay không ?
    uint32_t remain_data = file->size - offset;
    if (size > remain_data)
        size = remain_data;

    uint32_t bytes_left = size; 
    uint32_t total_read = 0;

    /* Tìm cluster bắt đầu */
    uint16_t cluster              = file->fat_entry;

    // Duyệt bảng FAT để đến đúng cluster chứa offset
    uint32_t clusters_to_skip = offset / SFAT_BYTES_PER_CLUSTER;  // Số cluster cần duyệt để đến đúng vị trí offset
    for (uint32_t i = 0; i < clusters_to_skip; i++) {
        cluster = sfat_fat_entry_get(cluster);
        if (cluster == SFAT_END_CLUSTER || cluster == SFAT_FREE_CLUSTER) 
            return 0; // Lỗi cấu trúc file
    }

    // Offset trong cluster đầu tiên cần đọc
    uint16_t data_offset_in_cluster = offset % SFAT_BYTES_PER_SECTOR; // offset cảu dữ liệu trong cluster đọc đầu tiên

    uint8_t data_buf[SFAT_BYTES_PER_SECTOR];
    uint8_t fat_buf[SFAT_BYTES_PER_SECTOR];
    uint16_t *fat_ptr = (uint16_t *)fat_buf;

    int16_t current_fat_sector = -1;

    while(bytes_left > 0 && cluster != SFAT_END_CLUSTER) {
        // 2. Đọc dữ liệu từ vùng DATA
        sfat_read_sector(SFAT_START_DATA + cluster, data_buf);

        uint32_t bytes_in_cluster = SFAT_BYTES_PER_SECTOR - data_offset_in_cluster; // số byte còn lại chưa đọc trong cluster hiện tại.   
        uint32_t chunk = (bytes_left < bytes_in_cluster) ? bytes_left : bytes_in_cluster; // số bytes thực sự read() trong lần này

        memcpy((uint8_t *)buf + total_read, data_buf + data_offset_in_cluster, chunk);

        total_read   += chunk;      // Tổng số byte đã đọc
        bytes_left   -= chunk;
        data_offset_in_cluster = 0; // Từ cluster thứ 2 trở đi luôn đọc từ byte 0

        // 3. Lấy cluster tiếp theo từ bảng FAT
        if (bytes_left > 0) {
            uint16_t fat_sec = GET_FAT_SECTOR(cluster);
            if (fat_sec != current_fat_sector) {
                sfat_read_sector(SFAT_START_FAT + fat_sec, fat_buf);
                current_fat_sector = fat_sec;
            }
            cluster = fat_ptr[GET_FAT_OFFSET(cluster)];
        }
    }

    fd_table[fd].offset += total_read;
    return total_read;
}

void sfat_close(uint8_t fd) {
    fd_table[fd].used = false;
}

int sfat_stat(const char *name) {
    /* 1. Tìm file */
    uint8_t dir_buf[SFAT_BYTES_PER_SECTOR];
    sfat_read_sector(SFAT_START_DIR, dir_buf);
    DirEntry_t *direntries_ptr = (DirEntry_t *)dir_buf;
    
    DirEntry_t *file = NULL;
    for(uint16_t i = 0; i < SFAT_MAX_DIRENTRIES; i++) {
        file = &direntries_ptr[i];
        if(strncmp((const char *)file->name, name, 10) == 0) {
            break;
        }
    }

    /* 2. Hiển thị thông tin của file */
    if(file != NULL) {
        uint8_t number_cluster_used = (file->size + SFAT_BYTES_PER_CLUSTER - 1) / SFAT_BYTES_PER_CLUSTER;

        uint8_t mode = SFAT_NO_MODE; // Nếu không mở thì không có mode
        char    str_mode[4];
        for(int i = 0; i < SFAT_MAX_OPEN_FILES; i++) {
            if(fd_table[i].used && fd_table[i].dir_entry == file) {
                mode = fd_table[i].mode;
                break;
            }
        }

        switch (mode)
        {
        case SFAT_NO_MODE:  strncpy(str_mode, "N/A", 3); str_mode[3] = '\0'; break;
        case SFAT_MODE_W:   strncpy(str_mode, "w", 1);   str_mode[1] = '\0'; break;
        case SFAT_MODE_A:   strncpy(str_mode, "a", 1);   str_mode[1] = '\0'; break;
        case SFAT_MODE_RP:  strncpy(str_mode, "r+", 2);  str_mode[2] = '\0'; break;

        default:
            break;
        }

        printf("\nname = \"%s\",  first cluster = %u, clusters = %u, size = %u bytes, mode = \"%s\"\n",
                file->name,
                file->fat_entry,
                number_cluster_used,
                file->size,
                str_mode);
    }
    return 0;
}

