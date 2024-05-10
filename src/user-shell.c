#include "./header/user-shell.h"
#include <stdint.h>
#include "header/filesystem/fat32.h"
#include "header/stdlib/string.h"

#define strsplit(str,delim,result) syscall(20, (uint32_t) str, (uint32_t) delim, (uint32_t) result)
#define strlen(str,strlenvar) syscall(21, (uint32_t) str, (uint32_t) &strlenvar, 0)
// #define memcpy()

static struct shellState shellState = {
    .workDir = ROOT_CLUSTER_NUMBER,
    .commandBuffer = {0},
    .bufferIndex = 0,
    .startingWriteLoc = {0, 0},
    .directory = {0}};

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
}

void reset_shell_buffer()
{
    shellState.bufferIndex = 0;
    memset(shellState.commandBuffer, 0, 256);
}

void process_commands()
{
    reset_shell_buffer();
    print_shell_prompt();
}

void use_keyboard()
{
    char currChar;
    syscall(4, (uint32_t)&currChar, 0, 0);
    if (currChar)
    {
        if (currChar == '\b' && shellState.bufferIndex == 0) // backspacing at the beginning
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
    struct ClusterBuffer cl[2] = {0};
    struct FAT32DriverRequest request = {
        .buf = &cl,
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


