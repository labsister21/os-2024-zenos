#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "../header/stdlib/string.h"
#include "../header/filesystem/fat32.h"
const uint8_t fs_signature[BLOCK_SIZE] = {
    'C', 'o', 'u', 'r', 's', 'e', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',  ' ',
    'D', 'e', 's', 'i', 'g', 'n', 'e', 'd', ' ', 'b', 'y', ' ', ' ', ' ', ' ',  ' ',
    'L', 'a', 'b', ' ', 'S', 'i', 's', 't', 'e', 'r', ' ', 'I', 'T', 'B', ' ',  ' ',
    'M', 'a', 'd', 'e', ' ', 'w', 'i', 't', 'h', ' ', '<', '3', ' ', ' ', ' ',  ' ',
    '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '2', '0', '2', '4', '\n',
    [BLOCK_SIZE-2] = 'O',
    [BLOCK_SIZE-1] = 'k',
};

static struct FAT32DriverState driverState;

uint32_t mergeClusterHighLow(uint16_t high, uint16_t low){
    return high << 16 | low;
}

uint32_t cluster_to_lba(uint32_t cluster){
    return cluster*CLUSTER_BLOCK_COUNT;
};

void init_directory_table(struct FAT32DirectoryTable *dir_table, char *name, uint32_t parent_dir_cluster){

    uint8_t i;
    char parentName[8] = {'.', '.','\0','\0','\0','\0','\0','\0'};
   memcpy(dir_table->table[0].name,name,8);
   dir_table->table[0].cluster_high = ((uint32_t)parent_dir_cluster >> 16) & 0xffff;
   dir_table->table[0].cluster_low = ((uint32_t)parent_dir_cluster & 0xffff);
   dir_table->table[0].filesize = 0; // this might cause an error, filesize = 0 -> folder
   dir_table->table[0].user_attribute = UATTR_NOT_EMPTY;
   dir_table->table[0].attribute = ATTR_SUBDIRECTORY;
   dir_table->table[0].undelete = true;
   memcpy(dir_table->table[1].name, parentName, 8);
   dir_table->table[1].cluster_high = ((uint32_t)parent_dir_cluster >> 16) & 0xffff;
   dir_table->table[1].cluster_low = ((uint32_t)parent_dir_cluster & 0xffff);
   dir_table->table[1].filesize = 0; // this might cause an error, filesize = 0 -> folder
   dir_table->table[1].user_attribute = UATTR_NOT_EMPTY;
   dir_table->table[1].attribute = ATTR_SUBDIRECTORY;
   for (i = 2 ; i < 64 ; i++ ){
    dir_table->table[i].user_attribute = !UATTR_NOT_EMPTY;
   }
   write_clusters(dir_table->table,parent_dir_cluster,1);

    
}

bool is_empty_storage(void){
    struct BlockBuffer block;
    read_blocks(&block,BOOT_SECTOR,1);
    // bool isEmpty = false;
    // uint16_t i = 0;
    // while((i<BLOCK_SIZE) && (!isEmpty)){
    //     if(block.buf[i] != fs_signature[i]){
    //         isEmpty = true;
    //     };
    //     i++;
    // }
    // return isEmpty;
    return memcmp(&block, fs_signature, BLOCK_SIZE);
};

void create_fat32(void){
    write_blocks(fs_signature,BOOT_SECTOR,1);
    driverState.fat_table.cluster_map[0] = CLUSTER_0_VALUE;
    driverState.fat_table.cluster_map[1] = CLUSTER_1_VALUE;
    driverState.fat_table.cluster_map[2] = FAT32_FAT_END_OF_FILE; // Root
    uint16_t i;
    for (i=3;i<CLUSTER_MAP_SIZE;i++){
        driverState.fat_table.cluster_map[i] = FAT32_FAT_EMPTY_ENTRY;
    };
    write_clusters(&driverState.fat_table, 1, 1);
    struct FAT32DirectoryTable rootDir = {0};
    init_directory_table(&rootDir, "root\0\0\0\0", 2);
    write_clusters(&rootDir,2,1);
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
    if(driverState.dir_table_buf.table[0].user_attribute == UATTR_NOT_EMPTY){
        bool isFound = false;
        bool isFile = false;
        uint8_t i = 0;
        while(i<(uint8_t)(CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry)) && !isFound){
            if(memcmp(driverState.dir_table_buf.table[i].name,request.name,8) == 0){
                if(driverState.dir_table_buf.table[i].attribute == ATTR_SUBDIRECTORY){
                    isFound = true;
                    isFile = false;
                }else{
                    isFile = true;
                }
            }
            i++;
        }
        if(isFound){
            i--;
            uint32_t cluster_num = mergeClusterHighLow(driverState.dir_table_buf.table[i].cluster_high,driverState.dir_table_buf.table[i].cluster_low);
            read_clusters(request.buf,cluster_num,1);
            return 0;
        }
        if(isFile){
            return 1;
        }
        else{
            return 2;
        }
    }else{
        return -1;
    }
}

int8_t read(struct FAT32DriverRequest request){
    read_clusters(&driverState.dir_table_buf,request.parent_cluster_number,1);
    if(driverState.dir_table_buf.table[0].user_attribute == UATTR_NOT_EMPTY){
        bool isFound = false;
        uint8_t i = 0;
        while(i<(uint8_t)(CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry)) && !isFound){
            if(memcmp(driverState.dir_table_buf.table[i].name,request.name,8) == 0 && memcmp(driverState.dir_table_buf.table[i].ext,request.ext,3) == 0){
                if(driverState.dir_table_buf.table[i].attribute == ATTR_SUBDIRECTORY){
                    return 1;
                }
                else if(driverState.dir_table_buf.table[i].filesize > request.buffer_size){
                    return driverState.dir_table_buf.table[i].filesize;
                }
                else{
                    isFound = true;
                }
            }
            i++;
        }
        if(isFound){
            i--;
            uint32_t cluster_num = mergeClusterHighLow(driverState.dir_table_buf.table[i].cluster_high ,driverState.dir_table_buf.table[i].cluster_low);
            uint32_t j = 0;
            do{
                read_clusters(request.buf + j*CLUSTER_SIZE,cluster_num,1);
                cluster_num = driverState.fat_table.cluster_map[cluster_num]; 
                j++;
            }while(cluster_num != FAT32_FAT_END_OF_FILE);
            return 0;
        }
        else{
            return 3;
        }
    }else{
        return -1;
    }
};

int8_t write(struct FAT32DriverRequest request){
    uint16_t i;
    int8_t dir_loc = -1;
    uint16_t clusters_needed = request.buffer_size/CLUSTER_SIZE;
    uint16_t cluster_located = 0;
    uint16_t k =  3; // start after reserved
    uint16_t empty_locations[clusters_needed];

    if (driverState.fat_table.cluster_map[request.parent_cluster_number] != FAT32_FAT_END_OF_FILE){
        return 2;
    }
    read_clusters(&driverState.dir_table_buf,request.parent_cluster_number,1);
    

    if (request.buffer_size % CLUSTER_SIZE != 0 ){
        clusters_needed += 1;
    }
    if (request.buffer_size == 0 && clusters_needed == 0){
        // for folder
        clusters_needed += 1;

    }

    
    for (i = 2 ; i < CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry) ; i++){
        if (driverState.dir_table_buf.table[i].user_attribute != UATTR_NOT_EMPTY && dir_loc == -1){
            // validate file or folder name
            dir_loc = i;
        }

        // else if (request.buffer_size >= 0){

        // }
        // else {
        //     // invalid buffer, buffer < 0
        //     return -1;
        // }
        if ( (memcmp(driverState.dir_table_buf.table[i].name, request.name,8  ) == 0 ) && (memcmp(driverState.dir_table_buf.table[i].ext , request.ext,3) == 0) ){
                return 1;
            }
    }

    if (dir_loc == -1){
        return -1;
    }
    
    while(cluster_located < clusters_needed && k < CLUSTER_MAP_SIZE){
        if (driverState.fat_table.cluster_map[k] == 0){
            empty_locations[cluster_located] = k;
            cluster_located++;

        }
        k++;
    }
    // not enough space
    if (cluster_located < clusters_needed){
        return -1;
    }
    memcpy(driverState.dir_table_buf.table[dir_loc].name,request.name,8);
    driverState.dir_table_buf.table[dir_loc].user_attribute = UATTR_NOT_EMPTY;
    driverState.dir_table_buf.table[dir_loc].cluster_high = (empty_locations[0] >> 16) & 0xffff;
    driverState.dir_table_buf.table[dir_loc].cluster_low = empty_locations[0] & 0xffff;
    driverState.dir_table_buf.table[dir_loc].filesize = request.buffer_size;
    // time to write file/folder
    if (request.buffer_size == 0){
        // write folder
        struct FAT32DirectoryTable new_table = {0};
        memcpy(driverState.dir_table_buf.table[dir_loc].ext,request.ext,3);
        init_directory_table(&new_table, request.name, request.parent_cluster_number);
        driverState.dir_table_buf.table[dir_loc].attribute = ATTR_SUBDIRECTORY;
        driverState.fat_table.cluster_map[empty_locations[0]] = FAT32_FAT_END_OF_FILE;
        write_clusters(&driverState.fat_table,1,1);
        write_clusters(new_table.table, empty_locations[0], 1);


    }

    else if (request.buffer_size > 0){
        memcpy(driverState.dir_table_buf.table[dir_loc].ext,request.ext,3);
        memcpy(driverState.dir_table_buf.table[dir_loc].name,request.name,8);
        for (i = 0 ; i < clusters_needed ; i++){
            write_clusters(request.buf + i * CLUSTER_SIZE,empty_locations[i],1 );
            if(i != clusters_needed - 1){
                driverState.fat_table.cluster_map[empty_locations[i]] = empty_locations[i+1];
            }
            else{
                driverState.fat_table.cluster_map[empty_locations[i]] = FAT32_FAT_END_OF_FILE;
            }
        }
        write_clusters(&driverState.fat_table,1,1);
        driverState.dir_table_buf.table->user_attribute = UATTR_NOT_EMPTY;
    }

    write_clusters(driverState.dir_table_buf.table,request.parent_cluster_number,1);
    return 0;
}

int8_t delete(struct FAT32DriverRequest request){
    read_clusters(&driverState.dir_table_buf,request.parent_cluster_number,1);
    if(driverState.dir_table_buf.table[0].user_attribute == UATTR_NOT_EMPTY){
        uint8_t i;
        for(i=0;i<(CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry));i++){
            // Folder
            if(driverState.dir_table_buf.table[i].attribute == ATTR_SUBDIRECTORY){
                if(memcmp(driverState.dir_table_buf.table[i].name,request.name,8) == 0 ){
                    struct FAT32DirectoryTable tempDirTab;
                    uint32_t cluster_num = mergeClusterHighLow(driverState.dir_table_buf.table[i].cluster_high ,driverState.dir_table_buf.table[i].cluster_low);
                    read_clusters(&tempDirTab, cluster_num,1);
                    uint8_t j;
                    for(j=2;j<(CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry));j++){
                        if(tempDirTab.table[j].user_attribute == UATTR_NOT_EMPTY){
                            return 2;
                        }
                    }

                    memset(tempDirTab.table, 0, 512*4);
                    write_clusters(tempDirTab.table, cluster_num, 1);

                    memset(driverState.dir_table_buf.table[i].name,0,8);
                    memset(driverState.dir_table_buf.table[i].ext,0,3);
                    driverState.dir_table_buf.table[i].attribute = 0;
                    driverState.dir_table_buf.table[i].user_attribute = 0;
                    driverState.dir_table_buf.table[i].undelete = 0;
                    driverState.dir_table_buf.table[i].create_time = 0;
                    driverState.dir_table_buf.table[i].create_date = 0;
                    driverState.dir_table_buf.table[i].access_date = 0;
                    driverState.dir_table_buf.table[i].cluster_high = 0;
                    driverState.dir_table_buf.table[i].modified_time = 0;
                    driverState.dir_table_buf.table[i].modified_date = 0;
                    driverState.dir_table_buf.table[i].cluster_low = 0;
                    driverState.dir_table_buf.table[i].filesize = 0;
                    
                    driverState.fat_table.cluster_map[cluster_num] = 0;
                    // driverState.dir_table_buf.table[i].user_attribute = !UATTR_NOT_EMPTY;
                    // driverState.dir_table_buf.table[i].cluster_low = 0;
                    // driverState.dir_table_buf.table[i].cluster_high= 0;
                    // memset(driverState.dir_table_buf.table[i].na)
                    write_clusters(driverState.dir_table_buf.table,request.parent_cluster_number,1);
                    write_clusters(driverState.fat_table.cluster_map,1,1);
                    return 0;
                }
            }
            // File
            else{
                if(memcmp(driverState.dir_table_buf.table[i].name,request.name,8) == 0 && memcmp(driverState.dir_table_buf.table[i].ext,request.ext,3) == 0){
                    read_clusters(&driverState.dir_table_buf,request.parent_cluster_number,1);
                    uint32_t curr_cluster = mergeClusterHighLow(driverState.dir_table_buf.table[i].cluster_high, driverState.dir_table_buf.table[i].cluster_low);
                    uint32_t temp_cluster;
                    memset(driverState.cluster_buf.buf, 0, CLUSTER_SIZE);
                    do{
                        write_clusters(driverState.cluster_buf.buf,curr_cluster,1);
                        temp_cluster = driverState.fat_table.cluster_map[curr_cluster];
                        driverState.fat_table.cluster_map[curr_cluster] = 0;
                        curr_cluster = temp_cluster;
                    }while(curr_cluster != FAT32_FAT_END_OF_FILE);

                    memset(driverState.dir_table_buf.table[i].name,0,8);
                    memset(driverState.dir_table_buf.table[i].ext,0,3);
                    driverState.dir_table_buf.table[i].attribute = 0;
                    driverState.dir_table_buf.table[i].user_attribute = 0;
                    driverState.dir_table_buf.table[i].undelete = 0;
                    driverState.dir_table_buf.table[i].create_time = 0;
                    driverState.dir_table_buf.table[i].create_date = 0;
                    driverState.dir_table_buf.table[i].access_date = 0;
                    driverState.dir_table_buf.table[i].cluster_high = 0;
                    driverState.dir_table_buf.table[i].modified_time = 0;
                    driverState.dir_table_buf.table[i].modified_date = 0;
                    driverState.dir_table_buf.table[i].cluster_low = 0;
                    driverState.dir_table_buf.table[i].filesize = 0;
        
                    write_clusters(driverState.dir_table_buf.table,request.parent_cluster_number,1);
                    write_clusters(driverState.fat_table.cluster_map,1,1);
                    return 0;
                }
            }
        }
        return 1;
    }else{
        return -1;
    }
}
