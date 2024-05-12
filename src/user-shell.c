#include "./header/user-shell.h"
#include <stdint.h>
#include "header/filesystem/fat32.h"
#include "header/stdlib/string.h"

// #define strsplit(str,delim,result) syscall(20, (uint32_t) str, (uint32_t) delim, (uint32_t) result)
// #define strlen(str,strlenvar) syscall(21, (uint32_t) str, (uint32_t) &strlenvar, 0)
// #define strcpy(dest,src) syscall(22, (uint32_t) dest, (uint32_t) src, 0)
#define get_dir(curr_parent_cluster_number, table) syscall(23, (uint32_t) curr_parent_cluster_number, (uint32_t) table, 0)
#define set_dir(curr_parent_cluster_number, table) syscall(25, (uint32_t) curr_parent_cluster_number, (uint32_t) table, 0)

// #define memcpy()

static struct shellState shellState = {
    .workDir = ROOT_CLUSTER_NUMBER,
    .commandBuffer = {0},
    .bufferIndex = 0,
    .startingWriteLoc = {0, 0},
    .directory = {0},
    .extendedMode = false,
    .arrowBuffer = {0},
    .arrowBufferIndex = 255};

void syscall(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx)
{
    __asm__ volatile("mov %0, %%ebx" : /* <Empty> */ : "r"(ebx));
    __asm__ volatile("mov %0, %%ecx" : /* <Empty> */ : "r"(ecx));
    __asm__ volatile("mov %0, %%edx" : /* <Empty> */ : "r"(edx));
    __asm__ volatile("mov %0, %%eax" : /* <Empty> */ : "r"(eax));
    // Note : gcc usually use %eax as intermediate register,
    //        so it need to be the last one to mov
    __asm__ volatile("int $0x30");
}

void finPath(char* destination, uint32_t current_cluster_number, char path[256]){
    struct FAT32DirectoryTable dirTable;
    get_dir(current_cluster_number,&dirTable);
    int x;
    for (x = 2 ; x < 64 ; x++){
        if (strcmp(dirTable.table[x].name , destination) == 0){
            char temp[256];
            strncpy(temp,path,256);
            strcat(temp, "/");
            
            strcat(temp,dirTable.table[x].name);
            syscall(6,(uint32_t)temp,0xf,0);
            if (dirTable.table[x].filesize > 0){
                syscall(6,(uint32_t)".",0xf,0);
                for (int i = 0 ; i < 3 ; i++){
                    syscall(5,(uint32_t)dirTable.table[x].ext[i],0xf,0);
                }

            }
            syscall(6,(uint32_t)"\n",0xf,0);
        }
        if (dirTable.table[x].attribute  == ATTR_SUBDIRECTORY ){
             char temp[256];
            strncpy(temp,path,256);
            strcat(temp, "/");
            strcat(temp,dirTable.table[x].name);
            uint32_t next_number = dirTable.table[x].cluster_high << 16 | dirTable.table[x].cluster_low;
            finPath(destination,next_number,temp);
        }
    }

}

void print_shell_prompt()
{
    char prompt[256] = SHELL_DIRECTORY;

    strcat(prompt, shellState.directory);
    strcat(prompt, SHELL_PROMPT);

    syscall(6, (uint32_t)prompt, BIOS_BLUE, 0);

    uint8_t row, col;
    syscall(8, (uint32_t)&row, 0, 0);
    syscall(9, (uint32_t)&col, 0, 0);
    syscall(10, (uint32_t)row, (uint32_t)col, 0);

    shellState.startingWriteLoc.col = col;
    shellState.startingWriteLoc.row = row;
}

void reset_shell_buffer()
{
    shellState.bufferIndex = 0;
    memset(shellState.commandBuffer, 0, 256);
}

void process_commands()
{      
    char buffer[16][256] = {0};
    char arg[128] = {0};
    strsplit(shellState.commandBuffer, ' ', buffer);
    int countCommands = 0;
    for (int z = 0 ; z < 16 ; z++){
        if (buffer[z][0] != 0){
            countCommands++;
        }
    }

    if (strcmp(buffer[0],"cd") == 0){
        if (countCommands > 2){
            syscall(6,(uint32_t)"too many arguments\n\n",0x4,0);
            reset_shell_buffer();
            print_shell_prompt();
            return;
        }
       
        strcpy(arg,buffer[1]);
        struct FAT32DirectoryTable table;
        get_dir( shellState.workDir, &table );
        // syscall(6,(uint32_t)arg, 0xf, 0);
        uint32_t parent_cluster_number = table.table[0].cluster_high << 16 | table.table[0].cluster_low;
        if (strcmp(arg, "..") == 0){

            if (shellState.workDir <= 2){

                reset_shell_buffer();
                print_shell_prompt();
                return;

            }
            char eachPath[16][256] = {0};
            char finalPath[256] = {0};
            strsplit(shellState.directory,'/',eachPath);
            int num_of_path = 0;
            for (int i = 0 ; i < 16 ; i++){
                if (eachPath[i][0] != 0){
                    num_of_path++;
                }
            }
            for (int j = 1 ; j < num_of_path ; j++){
                if (num_of_path != 1){
                    strcat(finalPath,"/");
                }
                strcat(finalPath,eachPath[j]);
            }
            strncpy(shellState.directory,finalPath,(uint32_t)256);
            shellState.workDir = parent_cluster_number;
            reset_shell_buffer();
            print_shell_prompt();
            return;

        }
        else{

            for (int i = 1;  i < 64 ; i++){
                if (table.table[i].attribute == ATTR_SUBDIRECTORY){
                    if (strcmp(table.table[i].name, arg) == 0){
                    strcat(shellState.directory, "/");
                    strcat(shellState.directory, arg);
                    shellState.workDir =  table.table[i].cluster_high << 16 | table.table[i].cluster_low;
                    reset_shell_buffer();
                    print_shell_prompt();
                    return;
                }
                }

            }

        }
    
    }
    else if (strcmp(buffer[0],"ls") == 0){
        if (countCommands > 1){
            syscall(6,(uint32_t)"too many arguments\n\n",0x4,0);
            reset_shell_buffer();
            print_shell_prompt();
            return;
        }
        struct FAT32DirectoryTable dirTable;
        get_dir(shellState.workDir,&dirTable);

        uint8_t z;
        for(z=2;z<64;z++)
        {
            if ((strcmp(&dirTable.table[z].name[0],"\0"))){
                int x;
                for(x=0;x<8;x++){
                    if(strcmp(&dirTable.table[z].name[x],"\0")){
                        syscall(5, (uint32_t) &dirTable.table[z].name[x], 0xF, 0);
                    }
                }
                if (dirTable.table[z].attribute == 0){
                    char dot[1];
                    char * dotraw = ".";
                    memcpy(dot,dotraw,1);
                    syscall(5, (uint32_t) dot, 0xF, 0);
                    int y;
                    for(y=0;y<3;y++){
                        syscall(5, (uint32_t) &dirTable.table[z].ext[y], 0xF, 0);
                    }
                }
                syscall(6, (uint32_t) "  ", 0xF, 0);
            }
        }
        syscall(6, (uint32_t)"\n\n",0xF,0);
    }
    else if (strcmp(buffer[0],"mkdir") == 0){
        if (countCommands > 2){
            syscall(6,(uint32_t)"too many arguments\n\n",0x4,0);
            reset_shell_buffer();
            print_shell_prompt();
            return;
        }
        struct FAT32DriverRequest req = {0};
        strcpy(req.name,buffer[1]);
        req.ext[0]  = '\0';
        req.ext[1]  = '\0';
        req.ext[2]  = '\0';
        req.parent_cluster_number = shellState.workDir;
        req.buffer_size = 0;
        int8_t return_code;
        syscall(24,(uint32_t ) &req, (uint32_t )&return_code,0);
        if (return_code == -1){
            syscall(6,(uint32_t)"Not enough space in the current working directory\n",0x4,0);
        } else if (return_code == 1){
            strcat(buffer[0],": ");
            strcat(buffer[0],"cannot create directory `");
            strcat(buffer[0], buffer[1]);
            strcat(buffer[0], "`: File exists\n\n");
            syscall(6,(uint32_t)buffer[0] ,0x4,0);
        }

    } 
    else if (strcmp(buffer[0],"cat") == 0){
        if (countCommands > 2){
            syscall(6,(uint32_t)"too many arguments\n\n",0x4,0);
            reset_shell_buffer();
            print_shell_prompt();
            return;
        }
        // parse file dengan extension
        char splitFilenameExt[16][256] = {0};
        strsplit(buffer[1],'.',splitFilenameExt);
        int countSplitResult = 0;
        for (int i = 0 ; i < 16 ; i++){
            if (splitFilenameExt[i][0] != 0){
                countSplitResult++;
            }
        }
        if (countSplitResult > 2){
            // invalid file
            syscall(6, (uint32_t) "cat: ", 0x4, 0);
            syscall(6, (uint32_t) buffer[1], 0x4, 0);
            syscall(6, (uint32_t) ": No such file or directory\n\n", 0x4, 0);
        }
        int8_t retcode;
        char outText[4*512*512];
        struct FAT32DriverRequest requestReadFile = {
        .buf =  outText,
        };
        memcpy(requestReadFile.name,splitFilenameExt[0],8);
        memcpy(requestReadFile.ext,splitFilenameExt[1],3);
        requestReadFile.parent_cluster_number = shellState.workDir;
        requestReadFile.buffer_size = 4*512*512;
        syscall(0, (uint32_t) &requestReadFile, (uint32_t) &retcode, 0);
        if(retcode == 0){
            syscall(6, (uint32_t)requestReadFile.buf,0xF,0);   
            syscall(6, (uint32_t) "\n\n",0xF,0);   
        }else{
            syscall(6, (uint32_t) "cat: ", 0x4, 0);
            syscall(6, (uint32_t) buffer[1], 0x4, 0);
            syscall(6, (uint32_t) ": No such file or directory\n\n", 0x4, 0);
        }
    }
    else if (strcmp(buffer[0],"mv") == 0){
        if (countCommands > 3){
            syscall(6,(uint32_t)"too many arguments\n\n",0x4,0);
            reset_shell_buffer();
            print_shell_prompt();
            return;
        }
        uint8_t countSplitParam1 = 0;
        uint8_t countSplitParam2 = 0;
        char splitFilenameExt1[16][256] = {0};
        char splitFilenameExt2[16][256] = {0};
        uint8_t param1ExtLength = 0;
        uint8_t param2ExtLength = 0;

        char eachPathParam1[16][256] = {0};
        strsplit(buffer[1],'/',eachPathParam1);
        int countPathParam1 = 0;
        for (int i = 0 ; i < 16 ; i++){
            if (eachPathParam1[i][0] != 0){
                countPathParam1++;
            }
        }

        char eachPathParam2[16][256] = {0};
        strsplit(buffer[2],'/',eachPathParam2);
        int countPathParam2 = 0;
        for (int i = 0 ; i < 16 ; i++){
            if (eachPathParam2[i][0] != 0){
                countPathParam2++;
            }
        }

        // cek apakah buffer[1] merupakan nama file
        strsplit(buffer[1],'.',splitFilenameExt1);
        for (int i = 0 ; i < 16 ; i++){
            if (splitFilenameExt1[i][0] != 0){
                countSplitParam1++;
            }else{
                break;
            }
        }
        if (countSplitParam1 == 2){
            for(int y=0;y<256;y++){
                if(splitFilenameExt1[1][y] != 0){
                    param1ExtLength++;
                }else{
                    break;
                }
            }
        }

        // cek apakah buffer[2] merupakan nama file
        strsplit(buffer[2],'.',splitFilenameExt2);
        for (int i = 0 ; i < 16 ; i++){
            if (splitFilenameExt2[i][0] != 0){
                countSplitParam2++;
            }
        }
        if (countSplitParam2 == 2){
            for(int y=0;y<256;y++){
                if(splitFilenameExt2[1][y] != 0){
                    param2ExtLength++;
                }else{
                    break;
                }
            }
        }
        bool isFound = false;
        // Rename file
        if(countSplitParam1 == 2 && countSplitParam2 == 2 && param1ExtLength <= 3 && param2ExtLength <= 3 && countPathParam1 == 1 && countPathParam2 == 1){
            struct FAT32DirectoryTable dirTable;
            get_dir(shellState.workDir,&dirTable);
            int x;
            for(x=2;x<64;x++){
                if(memcmp(dirTable.table[x].name, splitFilenameExt1[0], 8) == 0 && memcmp(dirTable.table[x].ext, splitFilenameExt1[1], 3) == 0){
                    memcpy(dirTable.table[x].name, splitFilenameExt2[0],8);
                    memcpy(dirTable.table[x].ext, splitFilenameExt2[1],3);
                    isFound = true;
                    break;
                };
            }
            if(isFound){
                set_dir(shellState.workDir, &dirTable);
            }else{
                syscall(6, (uint32_t) "mv: ", 0x4, 0);
                syscall(6, (uint32_t) buffer[1], 0x4, 0);
                syscall(6, (uint32_t) ": No such file or directory\n\n", 0x4, 0);
            }
        }else{
           // Memindahkan file/directory

           // Ambil directory entry dari file/folder yang ingin dipindah
           struct FAT32DirectoryEntry tempDirEntry;
            uint32_t currParentCluster = ROOT_CLUSTER_NUMBER;
            char currName[8];
            char currExt[3];
            struct FAT32DirectoryTable tempDirTable;
            for (int j = 0 ; j < countPathParam1 ; j++){
                isFound = false;
                get_dir(currParentCluster, &tempDirTable);
                memcpy(currExt,"\0\0\0",3);
                if(j == countPathParam1-1){
                    char splitFilenameExtTemp[16][256] = {0};
                    strsplit(eachPathParam1[j], '.', splitFilenameExtTemp);
                    int countSplitResult = 0;
                    for(int i=0;i<16;i++){
                        if(splitFilenameExtTemp[i][0] == '\0'){
                            break;
                        }else{
                            countSplitResult++;
                        }
                    }
                    if(countSplitResult == 2){
                        int countExtLengthTemp = 0;
                        for(int y=0; y<256;y++){
                            if(splitFilenameExtTemp[1][y] == '\0'){
                                break;
                            }else{
                                countExtLengthTemp++;
                            }
                        }
                        if(countExtLengthTemp <= 3){
                            memcpy(currExt, splitFilenameExtTemp[1],3);
                            memcpy(currName, splitFilenameExtTemp[0],8);
                        }else{
                            memcpy(currName,splitFilenameExtTemp[0],8);
                        }
                    }else{
                        memcpy(currName,splitFilenameExtTemp[0],8);
                    }
                }else{
                    memcpy(currName, eachPathParam1[j],8);
                }
                for(uint8_t i = 2; i<64 ;i++){
                    if(memcmp(tempDirTable.table[i].name,currName,8) == 0 && memcmp(tempDirTable.table[i].ext, currExt,3) == 0){
                        if(j == countPathParam1-1){
                            memcpy(&tempDirEntry, &tempDirTable.table[i],32);
                            memset(&tempDirTable.table[i],0,32);
                            set_dir(currParentCluster, &tempDirTable);
                        }else{
                            currParentCluster = tempDirTable.table[i].cluster_high << 16 | tempDirTable.table[i].   cluster_low;
                        }
                        isFound = true;
                        break;
                    }
                }
                if(!isFound){
                    break;
                }
            }

            if(!isFound){
                syscall(6, (uint32_t) "mv: ", 0x4, 0);
                syscall(6, (uint32_t) buffer[1], 0x4, 0);
                syscall(6, (uint32_t) ": No such file or directory\n\n", 0x4, 0);
            }else{
                // tempDirEntry sudah didapatkan, selanjutnya mencari cluster number dari tempat tujuan mv
                currParentCluster = ROOT_CLUSTER_NUMBER;
                for (int j = 0 ; j < countPathParam2 ; j++){
                    isFound = false;
                    get_dir(currParentCluster, &tempDirTable);
                    memcpy(currExt,"\0\0\0",3);
                    memcpy(currName, eachPathParam2[j],8);
                    for(uint8_t i = 2; i<64 ;i++){
                        if(memcmp(tempDirTable.table[i].name,currName,8) == 0 && memcmp(tempDirTable.table[i].ext, currExt,3) == 0){
                            currParentCluster = tempDirTable.table[i].cluster_high << 16 | tempDirTable.table[i].   cluster_low;
                            isFound = true;
                            break;
                        }
                    }
                    if(!isFound){
                        break;
                    }
                }
                if(!isFound){
                    syscall(6, (uint32_t) "mv: ", 0x4, 0);
                    syscall(6, (uint32_t) buffer[1], 0x4, 0);
                    syscall(6, (uint32_t) ": No such file or directory\n\n", 0x4, 0);
                }else{
                    get_dir(currParentCluster, &tempDirTable);
                    for(int x=2;x<64;x++){
                        if(tempDirTable.table[x].name[0] == '\0'){
                            memcpy(&tempDirTable.table[x],&tempDirEntry,32);
                            set_dir(currParentCluster, &tempDirTable);
                            break;
                        }
                    }
                }
            }
        }
    } else if (strcmp(buffer[0],"find") == 0){
        char temp[256] = {0};
        finPath(buffer[1],2,temp);


    }
    else{
        strcat(buffer[0],": ");
        strcat( buffer[0] ,"command not found\n\n");
        syscall(6,(uint32_t)buffer[0] ,0x4,0);
    }

    reset_shell_buffer();
    print_shell_prompt();
}

void use_keyboard()
{
    char currChar;
    syscall(4, (uint32_t)&currChar, 0, 0);
    if (currChar)
    {
        if (currChar == (char)0xE0) // check whether next char is an extended scancode
        {
            shellState.extendedMode = true;
        }
        else if (shellState.extendedMode) // if extended
        {
            shellState.extendedMode = false;
            uint8_t row, col;
            syscall(8, (uint32_t)&row, 0, 0);
            syscall(9, (uint32_t)&col, 0, 0);
            if (currChar == (char)0x48) // UP arrow
            {
            }
            else if (currChar == (char)0x50) // DOWN arrow
            {
            }
            else if (currChar == (char)0x4B) // LEFT arrow
            {
                // make an arrow buffer to remember what has passed
                shellState.bufferIndex--;
                shellState.arrowBuffer[shellState.arrowBufferIndex] = shellState.commandBuffer[shellState.bufferIndex];
                shellState.bufferIndex++;
                if (col == 0)
                {
                    syscall(10, (uint32_t)row - 1, (uint32_t)79, 0);
                }
                else
                {
                    syscall(10, (uint32_t)row, (uint32_t)col - 1, 0);
                }
            }
            else if (currChar == (char)0x4D) // RIGHT arrow
            {
            }
        }
        else if (currChar == '\b' && shellState.bufferIndex == 0) // backspacing at the beginning
        {
            // do nothing
        }
        else if (currChar == '\b') // normal backspacing
        {
            shellState.bufferIndex--;
            shellState.commandBuffer[shellState.bufferIndex] = 0;
            syscall(5, (uint32_t)&currChar, 0, 0);
        }
        else if (currChar == '\n') // Process Command
        {
            shellState.commandBuffer[shellState.bufferIndex] = 0;
            syscall(5, (uint32_t)&currChar, 0, 0);
            process_commands();
            // syscall(6, (uint32_t)"After process", 0, 0);
            
        }
        else if (shellState.arrowBufferIndex != 255) // if the arrow buffer is not empty indicating there is a char/string beside it
        {
            char temp[256];
            int i = 0;
            temp[i] = currChar;
            int arrowBufferLength = 255 - shellState.arrowBufferIndex;
            for(int j = 0; j < arrowBufferLength; j++){
                // inserting arrow buffer into a temp
                shellState.arrowBufferIndex++;
                i++;
                temp[i] = shellState.arrowBuffer[shellState.arrowBufferIndex];

                // inserting the arrow buffer into the command buffer
                shellState.commandBuffer[shellState.bufferIndex] = temp[i];
                shellState.bufferIndex++;
            }
            temp[i] = '\0';
            syscall(6, (uint32_t) temp, 0xF, 0);
        }
        else
        {
            shellState.commandBuffer[shellState.bufferIndex] = currChar;
            shellState.bufferIndex++;
            syscall(5, (uint32_t)&currChar, 0, 0);
        }
    }
}

// void handle_command(){
//     char current_commands[20][256] = {0};
//     char command[20];
//     char arg[20];

//     strsplit(shellState.commandBuffer,' ', current_commands);
//     memcpy(command,current_commands[0]);

//     if (memcmp(command,"cd") == 0){
//         memcpy(arg,current_commands[1]);
//         // set cursor location
        

//     }


// }

int main(void)
{
    struct FAT32DriverRequest request = {
        .buf = (uint8_t*)0,
        .name = "shell",
        .ext = "\0\0\0",
        .parent_cluster_number = ROOT_CLUSTER_NUMBER,
        .buffer_size = 0x100000,
    };


    int32_t retcode;
    syscall(0, (uint32_t) &request, (uint32_t) &retcode, 0);
    if (retcode == 0){
        syscall(6, (uint32_t) "owo\n", 4, 0xF);

    }
    
    syscall(7, 0, 0, 0);
    print_shell_prompt();
    while (true)
    {
        use_keyboard();
    }

    return 0;
}


