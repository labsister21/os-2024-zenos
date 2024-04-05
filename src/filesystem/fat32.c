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
    uint16_t i = 0;
    while((i<BLOCK_SIZE) && (!isEmpty)){
        if(block.buf[i] != fs_signature[i]){
            isEmpty = true;
        };
        i++;
    }
    return isEmpty;
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
    write_clusters(&driverState.fat_table,1,1);
    init_directory_table(&driverState.dir_table_buf, "root", 2);
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
            if(driverState.dir_table_buf.table[i].attribute == ATTR_SUBDIRECTORY){
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
            if(driverState.dir_table_buf.table[i].attribute == ATTR_SUBDIRECTORY){
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
        i--;
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

    
    for (i = 0 ; i < CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry) ; i++){
        if (driverState.dir_table_buf.table[i].user_attribute != UATTR_NOT_EMPTY && dir_loc == -1){
            // validate file or folder name
            dir_loc = i;
        }

        else if (request.buffer_size >= 0){
            if ( (memcmp(driverState.dir_table_buf.table[i].name, request.name,8  ) == 0 ) && (memcmp(driverState.dir_table_buf.table[i].ext , request.ext,3) == 0) ){
                return 1;
            }
        }
        else {
            // invalid buffer, buffer < 0
            return -1;
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
    driverState.dir_table_buf.table[dir_loc].cluster_high = (cluster_located >> 16) & 0xffff;
    driverState.dir_table_buf.table[dir_loc].cluster_low = cluster_located & 0xffff;
    driverState.dir_table_buf.table[dir_loc].filesize = request.buffer_size;
    // time to write file/folder
    if (request.buffer_size == 0){
        // write folder
        driverState.dir_table_buf.table[dir_loc].attribute = ATTR_SUBDIRECTORY;
        init_directory_table(request.buf,request.name,empty_locations[0]);

    }

    else if (request.buffer_size > 0){
        memcpy(driverState.dir_table_buf.table[dir_loc].ext,request.ext,3);
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
    }
    write_clusters(driverState.dir_table_buf.table,request.parent_cluster_number,1);
    return 0;
}

int8_t delete(struct FAT32DriverRequest request){
    read_clusters(&driverState.dir_table_buf,request.parent_cluster_number,1);
    uint8_t i;
    for(i=0;i<(CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry));i++){
        // Folder
        if(driverState.dir_table_buf.table[i].attribute == ATTR_SUBDIRECTORY || driverState.dir_table_buf.table[i].filesize == 0){
            if(memcmp(driverState.dir_table_buf.table[i].name,request.name,8) == 0 ){
                struct FAT32DirectoryTable tempDirTab;
                read_clusters(&tempDirTab,driverState.dir_table_buf.table[i].cluster_low,1);
                uint8_t j;
                for(j=0;j<(CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry));j++){
                    if(tempDirTab.table[j].user_attribute == UATTR_NOT_EMPTY){
                        return 2;
                    }
                }
                driverState.dir_table_buf.table[i].user_attribute = !UATTR_NOT_EMPTY;
                driverState.dir_table_buf.table[i].cluster_low = 0;
                write_clusters(&driverState.dir_table_buf,request.parent_cluster_number,1);
                write_clusters(&driverState.fat_table,1,1);
                return 0;
            }
        }
        // File
        else{
            if(memcmp(driverState.dir_table_buf.table[i].name,request.name,8) == 0 && memcmp(driverState.dir_table_buf.table[i].ext,request.ext,3) == 0){
                read_clusters(&driverState.dir_table_buf,request.parent_cluster_number,1);
                uint8_t n_cluster= (uint8_t)(driverState.dir_table_buf.table[i].filesize / CLUSTER_SIZE)+1;
                if(driverState.dir_table_buf.table[i].filesize % CLUSTER_SIZE !=0){
                    n_cluster++;
                }
                uint16_t curr_cluster = driverState.dir_table_buf.table[i].cluster_low;
                uint8_t temp_cluster;
                driverState.dir_table_buf.table[i].cluster_low = 0;
                driverState.dir_table_buf.table[i].user_attribute = !UATTR_NOT_EMPTY;
                write_clusters(&driverState.dir_table_buf, request.parent_cluster_number, 1);
                uint8_t j;
                for(j=0; j<n_cluster; j++){
                    temp_cluster = curr_cluster;
                    driverState.fat_table.cluster_map[curr_cluster] = 0;
                    if(j!=n_cluster-1){
                        curr_cluster = driverState.fat_table.cluster_map[temp_cluster];
                    }
                }
                write_clusters(&driverState.fat_table,1,1);
                return 0;
            }
        }
    }
    return 1;
}
