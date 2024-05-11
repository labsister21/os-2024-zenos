#include "./header/user-shell.h"
#include <stdint.h>
#include "header/filesystem/fat32.h"
#include "header/stdlib/string.h"

// #define strsplit(str,delim,result) syscall(20, (uint32_t) str, (uint32_t) delim, (uint32_t) result)
// #define strlen(str,strlenvar) syscall(21, (uint32_t) str, (uint32_t) &strlenvar, 0)
// #define strcpy(dest,src) syscall(22, (uint32_t) dest, (uint32_t) src, 0)
#define get_dir(curr_parent_cluster_number, table) syscall(23, (uint32_t) curr_parent_cluster_number, (uint32_t) table, 0)

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

    if (strcmp(buffer[0],"cd") == 0){
       
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
    // struct ClusterBuffer cl[2] = {0};
    struct FAT32DriverRequest request = {
        .buf = (uint8_t*)0,
        .name = "shell",
        .ext = "\0\0\0",
        .parent_cluster_number = ROOT_CLUSTER_NUMBER,
        .buffer_size = 0x100000,
    };


    int32_t retcode;
    // if(retcode == 0){
    //     syscall(5, (uint32_t) 'c', 0xF, 0);

    // }
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


