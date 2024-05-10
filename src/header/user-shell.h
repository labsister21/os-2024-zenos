#define SHELL_DIRECTORY "user@zenOS:~"
#define SHELL_PROMPT "$ "

struct location{
    uint8_t row;
    uint8_t col;
}

struct shellState{
    uint32_t workDir;
    char commandBuffer[256];
    uint8_t bufferIndex;
    struct location startingWriteLoc;
    char directory[256];
}

void print_shell_prompt();

void use_keyboard();