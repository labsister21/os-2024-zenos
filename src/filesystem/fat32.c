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

void init_directory_table(struct FAT32DirectoryTable *dir_table, char *name, uint32_t parent_dir_cluster){
    // struct FAT32DirectoryEntry entry;
    uint8_t i;
   memcpy(dir_table->table[0].name,name,8);


   dir_table->table[0].cluster_high = ((uint32_t)parent_dir_cluster >> 16) & 0xffff;
   dir_table->table[0].cluster_low = ((uint32_t)parent_dir_cluster & 0xffff);
   dir_table->table[0].filesize = 0; // this might cause an error
   dir_table->table[0].user_attribute = UATTR_NOT_EMPTY;
   dir_table->table[0].undelete = true;
    
   for (i = 1 ; i < 64 ; i++ ){
    dir_table->table[i].user_attribute = !UATTR_NOT_EMPTY;
   }
   write_clusters(dir_table->table,parent_dir_cluster,1);

}

bool is_empty_storage(void){
    struct BlockBuffer block;
    read_blocks(block.buf,BOOT_SECTOR,1);
    bool isEmpty = false;
    uint8_t i = 0;
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
    uint8_t i;
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

int8_t read_directory(struct FAT32DriverRequest request){
    read_clusters(&driverState.dir_table_buf,request.parent_cluster_number,1);
    bool isFound = false;
    uint8_t i = 0;
    while(i<(uint8_t)(CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry)) && !isFound){
        if(memcmp(driverState.dir_table_buf.table[i].name,request.name,8) == 0){
            if(driverState.dir_table_buf.table[i].filesize == 0 || driverState.dir_table_buf.table[i].attribute == ATTR_SUBDIRECTORY){
                isFound = true;
            }
            else{
                return 1;
            }
        }
        i++;
    }
    if(isFound){
        uint8_t cluster_num = driverState.dir_table_buf.table[i].cluster_low;
        read_clusters(request.buf + 0*CLUSTER_SIZE,cluster_num,1);
        return 0;
    }
    else{
        return 2;
    }
}

int8_t read(struct FAT32DriverRequest request){
    read_clusters(&driverState.dir_table_buf,request.parent_cluster_number,1);
    bool isFound = false;
    uint8_t i = 0;
    while(i<(uint8_t)(CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry)) && !isFound){
        if(memcmp(driverState.dir_table_buf.table[i].name,request.name,8) == 0 && memcmp(driverState.dir_table_buf.table[i].ext,request.ext,3) == 0){
            if(driverState.dir_table_buf.table[i].filesize == 0 || driverState.dir_table_buf.table[i].attribute == ATTR_SUBDIRECTORY){
                return 1;
            }
            else if(driverState.dir_table_buf.table[i].filesize > request.buffer_size){
                return -1;
            }
            else{
                isFound = true;
            }
        }
        i++;
    }
    if(isFound){
        uint8_t n_cluster= (uint8_t)(driverState.dir_table_buf.table[i].filesize / CLUSTER_SIZE);
        if(driverState.dir_table_buf.table[i].filesize % CLUSTER_SIZE !=0){
            n_cluster++;
        }
        uint8_t cluster_num = driverState.dir_table_buf.table[i].cluster_low;
        uint8_t j;
        for(j=0;j<n_cluster;j++){
            read_clusters(request.buf + j*CLUSTER_SIZE,cluster_num,1);
            cluster_num = driverState.fat_table.cluster_map[cluster_num];
        }
        return 0;
    }
    else{
        return 2;
    }
}

// int8_t write(struct FAT32DriverRequest request);

// int8_t delete(struct FAT32DriverRequest request);
