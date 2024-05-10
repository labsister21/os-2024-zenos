#include "./header/user-shell.h"
#include <stdint.h>
#include "header/filesystem/fat32.h"

// static struct shellState shellState = {
//     .workDir =  ROOT_CLUSTER_NUMBER,
//     .commandBuffer = {0},
//     .bufferIndex = 0,
//     .startingWriteLoc = {0,0},
//     .directory = {0}
// };


void syscall(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx) {
    __asm__ volatile("mov %0, %%ebx" : /* <Empty> */ : "r"(ebx));
    __asm__ volatile("mov %0, %%ecx" : /* <Empty> */ : "r"(ecx));
    __asm__ volatile("mov %0, %%edx" : /* <Empty> */ : "r"(edx));
    __asm__ volatile("mov %0, %%eax" : /* <Empty> */ : "r"(eax));
    // Note : gcc usually use %eax as intermediate register,
    //        so it need to be the last one to mov
    __asm__ volatile("int $0x30");
}

void print_shell_prompt(){
    // char prompt[256] = SHELL_DIRECTORY;
    
    // strcat(prompt, state.dir_string);
    // strcat(prompt, SHELL_PROMPT);

    // syscall(6, )

}

void use_keyboard(){
    // char currChar;
    // syscall(4, (uint32_t) &currChar, 0, 0);
    // if(currChar){
    //     if(c == '\b' && shellState.buffer_index == 0){
    //         // do nothing
    //     }else if(c == '\b') {
    //         shellState.bufferIndex--;
    //         shellState.command_buffer[shellState.bufferIndex] = 0;
    //         syscall()
            
    //     }
    // }

}

int main(void) {
    // struct ClusterBuffer      cl[2]   = {0};
    // struct FAT32DriverRequest request = {
    //     .buf                   = &cl,
    //     .name                  = "shell",
    //     .ext                   = "\0\0\0",
    //     .parent_cluster_number = ROOT_CLUSTER_NUMBER,
    //     .buffer_size           = CLUSTER_SIZE,
    // };
    // int32_t retcode;
    // syscall(0, (uint32_t) &request, (uint32_t) &retcode, 0);
    // if (retcode == 0)
    //     syscall(6, (uint32_t) "owo\n", 4, 0xF);

    // char buf;
    // syscall(7, 0, 0, 0);
    // while (true) {
    //     syscall(4, (uint32_t) &buf, 0, 0);
    //     syscall(5, (uint32_t) &buf, 0xF, 0);
    // }

    // char prompt[256] = SHELL_DIRECTORY;
    // while(true){
    // syscall(6, (uint32_t) prompt, (uint32_t) BIOS_BLUE, 0);

    // }
    struct ClusterBuffer      cl   = {0};
    struct FAT32DriverRequest request = {
        .buf                   = &cl,
        .name                  = "shell",
        .ext                   = "\0\0\0",
        .parent_cluster_number = ROOT_CLUSTER_NUMBER,
        .buffer_size           = CLUSTER_SIZE,
    };
    int32_t retcode;
    // if(retcode == 0){
    //     syscall(5, (uint32_t) 'c', 0xF, 0);
        
    // }
    syscall(0, (uint32_t) &request, (uint32_t) &retcode, 0);
    if (retcode == 0){
        // syscall(6, (uint32_t) "owo\n", 4, 0xF);
        // syscall(5, (uint32_t) 'c', 0xF, 0);
        }

    char buf;
    syscall(7, 0, 0, 0);
    while (true) {
        syscall(4, (uint32_t) &buf, 0, 0);
        if(buf){
            syscall(5, (uint32_t) &buf, 0, 0);
        }
    }

    return 0;
}