#include <stdint.h>
#include <stdbool.h>

#define SHELL_DIRECTORY "user@zenOS:~"
#define SHELL_PROMPT "$ "

#define BIOS_BLACK 0x0
#define BIOS_BLUE 0x1
#define BIOS_GREEN 0x2
#define BIOS_CYAN 0x3
#define BIOS_RED 0x4
#define BIOS_MAGENTA 0x5
#define BIOS_BROWN 0x6
#define BIOS_GRAY 0x7

#define BIOS_DARK_GRAY 0x8
#define BIOS_LIGHT_BLUE 0x9
#define BIOS_LIGHT_GREEN 0xA
#define BIOS_LIGHT_CYAN 0xB
#define BIOS_LIGHT_RED 0xC
#define BIOS_LIGHT_MAGENTA 0xD
#define BIOS_YELLOW 0xE
#define BIOS_WHITE 0xF

struct location
{
    uint8_t row;
    uint8_t col;
};

struct shellState
{
    uint32_t workDir;
    char commandBuffer[256];
    uint8_t bufferIndex;
    struct location startingWriteLoc;
    char directory[256];
    bool extendedMode;
    char arrowBuffer[256];
    int arrowBufferIndex;
    char recentsCommand[30][256];
    int recentsWriteIndex;
    int recentsReadIndex;
    bool hasWritten;
    char singleLineCommandBuffer[256];
};

void syscall(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx);

void reset_shell_buffer();

void shift_string(int from, int to, char *str, int size);

void clean_command(char *str);

void process_commands();

void print_shell_prompt();

void use_keyboard();