#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "../header/stdlib/string.h"
#include "../header/filesystem/fat32.h"

const uint8_t fs_signature[BLOCK_SIZE] = {
    'z','e','n','O','S',' ','h','a','s',' ','b','o','o','t','e','d'
};

struct FAT32DriverState driverState;

uint32_t cluster_to_lba(uint32_t cluster){
    return cluster*CLUSTER_SIZE;
};

// void init_directory_table(struct FAT32DirectoryTable *dir_table, char *name, uint32_t parent_dir_cluster);

bool is_empty_storage(void){
    struct BlockBuffer block;
    read_blocks(block.buf,BOOT_SECTOR,1);
    bool isEmpty = false;
    int i = 0;
    while(i<BLOCK_SIZE and !isEmpty){
        if(block.buf[i] != fs_signature[i]){
            isEmpty = true;
        i++;
        }
    }
    return isEmpty;
};

void create_fat32(void){
    write_blocks(fs_signature,BOOT_SECTOR,1);
    driverState.fat_table.cluster_map[0] = CLUSTER_0_VALUE;
    driverState.fat_table.cluster_map[1] = CLUSTER_1_VALUE;
    driverState.fat_table.cluster_map[2] = FAT32_FAT_END_OF_FILE; // Root
    int i;
    for (i=3;i<CLUSTER_MAP_SIZE;i++){
        driverState.fat_table.cluster_map[i] = FAT32_FAT_EMPTY_ENTRY;
    };
    write_clusters(&driverState.fat_table,1,1);
};

void initialize_filesystem_fat32(void){
    if (is_empty_storage()){
        create_fat32();
    }
    else{
        read_clusters(&driverState.fat_table,1,1);
    }
};

void write_clusters(const void *ptr, uint32_t cluster_number, uint8_t cluster_count){
    write_blocks(ptr,cluster_to_lba(cluster_number),CLUSTER_BLOCK_COUNT*cluster_count);
};

void read_clusters(void *ptr, uint32_t cluster_number, uint8_t cluster_count){
    read_blocks(ptr,cluster_to_lba(cluster_number),CLUSTER_BLOCK_COUNT*cluster_count);
};

// int8_t read_directory(struct FAT32DriverRequest request);

// int8_t read(struct FAT32DriverRequest request);

// int8_t write(struct FAT32DriverRequest request);

// int8_t delete(struct FAT32DriverRequest request);
