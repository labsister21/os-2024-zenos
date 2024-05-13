#include "./header/user-shell.h"
#include <stdint.h>
#include "header/filesystem/fat32.h"
#include "header/stdlib/string.h"

#define strsplit(str, delim, result) syscall(20, (uint32_t)str, (uint32_t)delim, (uint32_t)result)
// #define memcpy()

static struct shellState shellState = {
    .workDir = ROOT_CLUSTER_NUMBER,
    .commandBuffer = {0},
    .bufferIndex = 0,
    .startingWriteLoc = {0, 0},
    .directory = {0},
    .extendedMode = false,
    .arrowBuffer = {0},
    .arrowBufferIndex = 255,
    .recentsCommand = {{0}},
    .recentsWriteIndex = 0,
    .recentsReadIndex = 0,
    .hasWritten = false,
    .singleLineCommandBuffer = {0}};

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

void shift_string(int from, int to, char *str, int size)
{
    int shiftLength = size - from;
    int j = to;

    for (int i = 0; i < shiftLength && j < size; i++)
    {
        str[j++] = str[from + i];
    }

    for (int i = j; i < size; i++)
    {
        str[i] = 0;
    }
}

void clean_command(char *str)
{
    int i = 0;
    bool not_space = false;
    while (str[i] != '\0' && !not_space) // find the first not space
    {
        if (str[i] != ' ')
        {
            not_space = true;
        }
        else
        {
            i++;
        }
    }
    if (not_space) // found a char, not just string of spaces
    {
        shift_string(i, 0, str, 256);
        int l = 0;
        while (str[l] != '\0') // iterate until the end of the string
        {
            if (str[l] == ' ') // if there is a space, check if its more than 1, if its more then shift it so that its just 1
            {
                int m = l;
                while (str[m] == ' ')
                {
                    m++;
                }
                if (m > l)
                {
                    shift_string(m, l + 1, str, 256);
                }
            }
            l++;
        }
    }
}

void process_commands()
{
    clean_command(shellState.commandBuffer);
    uint8_t row, col;
    syscall(8, (uint32_t)&row, 0, 0);
    syscall(9, (uint32_t)&col, 0, 0);

    if (row > 20)
    {
        syscall(11, 0, 0, 0);
        syscall(10, 0, 0, 0);
        syscall(8, (uint32_t)&row, 0, 0);
        syscall(9, (uint32_t)&col, 0, 0);
    }

    reset_shell_buffer();
    print_shell_prompt();
}

void copy_single_line()
{
    int tempidx;
    if (shellState.arrowBufferIndex != 255) // if the arrow is not at the end of the line
    {
        tempidx = shellState.bufferIndex + (255 - shellState.arrowBufferIndex);
    }
    else // else just use the normal one
    {
        tempidx = shellState.bufferIndex;
    }
    shellState.hasWritten = true;
    shellState.commandBuffer[tempidx] = '\0';
    strcpy(shellState.singleLineCommandBuffer, shellState.commandBuffer);
}

void remove_current_command(uint8_t length)
{
    syscall(10, (uint32_t)shellState.startingWriteLoc.row, (uint32_t)shellState.startingWriteLoc.col, 0);
    char space = ' ';
    for (int i = 0; i < length; i++)
    {
        syscall(5, (uint32_t)&space, (uint32_t)0, 0);
    }
    syscall(10, (uint32_t)shellState.startingWriteLoc.row, (uint32_t)shellState.startingWriteLoc.col, 0);
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
                if (shellState.recentsWriteIndex > 0)
                {
                    if (shellState.recentsReadIndex != 0)
                    {
                        shellState.recentsReadIndex--;

                        // cleaning out the current command
                        uint8_t command_length = strlen(shellState.commandBuffer);
                        remove_current_command(command_length);

                        // copy the previous command
                        strcpy(shellState.commandBuffer, shellState.recentsCommand[shellState.recentsReadIndex]);

                        // display the previous command
                        syscall(6, (uint32_t)shellState.recentsCommand[shellState.recentsReadIndex], (uint32_t)0xF, 0);
                        command_length = strlen(shellState.recentsCommand[shellState.recentsReadIndex]);

                        // resetting the command buffer indexes
                        shellState.bufferIndex = command_length;
                        shellState.arrowBufferIndex = 255;
                    }
                }
            }
            else if (currChar == (char)0x50) // DOWN arrow
            {
                if (shellState.recentsReadIndex != shellState.recentsWriteIndex) // if its the same then it points to nothing
                {
                    uint8_t command_length = strlen(shellState.commandBuffer);
                    remove_current_command(command_length);
                    if (shellState.recentsReadIndex == shellState.recentsWriteIndex - 1) // points to the botom of the list
                    {
                        shellState.recentsReadIndex++;
                        if (shellState.hasWritten) // write the previously written keys
                        {

                            // copy the command previously written before using the up and down keys
                            strcpy(shellState.commandBuffer, shellState.singleLineCommandBuffer);
                            syscall(6, (uint32_t)shellState.commandBuffer, (uint32_t)0xF, 0);
                            command_length = strlen(shellState.commandBuffer);

                            // resetting the command buffer indexes
                            shellState.bufferIndex = command_length;
                            shellState.arrowBufferIndex = 255;
                        }
                        else // nothing to write, make it blank and resets the buffer
                        {
                            reset_shell_buffer();
                        }
                    }
                    else // points to besides the bottom of the list
                    {
                        shellState.recentsReadIndex++;

                        // copy the commands in the list
                        strcpy(shellState.commandBuffer, shellState.recentsCommand[shellState.recentsReadIndex]);
                        syscall(6, (uint32_t)shellState.commandBuffer, (uint32_t)0xF, 0);
                        command_length = strlen(shellState.singleLineCommandBuffer);

                        // resetting the command buffer indexes
                        shellState.bufferIndex = command_length;
                        shellState.arrowBufferIndex = 255;
                    }
                }
            }
            else if (currChar == (char)0x4B) // LEFT arrow
            {
                if (shellState.bufferIndex > 0)
                {
                    // make an arrow buffer to remember what has passed
                    shellState.bufferIndex--;
                    shellState.arrowBuffer[shellState.arrowBufferIndex] = shellState.commandBuffer[shellState.bufferIndex];
                    shellState.arrowBufferIndex--;
                    if (col == 0)
                    {
                        syscall(10, (uint32_t)row - 1, (uint32_t)79, 0);
                    }
                    else
                    {
                        syscall(10, (uint32_t)row, (uint32_t)col - 1, 0);
                    }
                }
            }
            else if (currChar == (char)0x4D) // RIGHT arrow
            {
                if (shellState.arrowBufferIndex < 255)
                {
                    shellState.bufferIndex++;
                    shellState.arrowBufferIndex++;
                    if (col == 80)
                    {
                        syscall(10, (uint32_t)row + 1, (uint32_t)0, 0);
                    }
                    else
                    {
                        syscall(10, (uint32_t)row, (uint32_t)col + 1, 0);
                    }
                }
            }
        }
        else if (currChar == '\n') // Process Command
        {
            if (shellState.arrowBufferIndex != 255) // processing command while in the middle of text
            {
                uint8_t row, col;
                syscall(8, (uint32_t)&row, 0, 0);
                syscall(9, (uint32_t)&col, 0, 0);
                int arrowBufferLength = 255 - shellState.arrowBufferIndex;
                syscall(10, (uint32_t)row, (uint32_t)col + arrowBufferLength, 0);

                shellState.bufferIndex += arrowBufferLength;
                shellState.arrowBufferIndex += arrowBufferLength;
            }
            shellState.commandBuffer[shellState.bufferIndex] = '\0';
            shellState.hasWritten = false;
            syscall(5, (uint32_t)&currChar, (uint32_t)0xF, 0);
            if (shellState.bufferIndex > 0)
            {
                strcpy(shellState.recentsCommand[shellState.recentsWriteIndex], shellState.commandBuffer);
                shellState.recentsWriteIndex++;
                shellState.recentsReadIndex = shellState.recentsWriteIndex;
            }
            process_commands();
        }
        else if (shellState.arrowBufferIndex != 255) // if the arrow buffer is not empty indicating there is a char/string beside it
        {
            shellState.recentsReadIndex = shellState.recentsWriteIndex;
            uint8_t row, col;
            syscall(8, (uint32_t)&row, 0, 0);
            syscall(9, (uint32_t)&col, 0, 0);
            if (currChar != '\b') // not backspace = writing in the middle of a text
            {
                char temp[256];

                // insert the curr char into the command buffer
                shellState.commandBuffer[shellState.bufferIndex] = currChar;
                shellState.bufferIndex++;
                // shellState.commandBuffer[shellState.bufferIndex] = currChar;

                // make a duplicate index cause we dont want to change the original indexes
                int arrow_index = shellState.arrowBufferIndex;
                int command_index = shellState.bufferIndex;

                // inserts the currChar into the temp string
                int tempIDX = 0;
                temp[tempIDX] = currChar;
                tempIDX++;

                // calculate how many char in arrow buffer
                int arrowBufferLength = 255 - shellState.arrowBufferIndex;

                for (int j = 0; j < arrowBufferLength; j++)
                {
                    // inserting arrow buffer into a temp
                    arrow_index++;
                    temp[tempIDX] = shellState.arrowBuffer[arrow_index];

                    // inserting the arrow buffer into the command buffer
                    shellState.commandBuffer[command_index] = shellState.arrowBuffer[arrow_index];

                    // increment both indexes
                    command_index++;
                    tempIDX++;
                }

                // prints all the previous char in arrow buffer
                temp[tempIDX] = '\0';
                syscall(6, (uint32_t)temp, 0xF, 0);
                // syscall(6, (uint32_t)shellState.commandBuffer, 0xF, 0);

                // set the position back in the editing space
                syscall(10, (uint32_t)row, (uint32_t)col + 1, 0);
                copy_single_line();
            }
            else // backspacing in the middle of a text
            {
                if (col == 0 || shellState.bufferIndex == 0)
                {
                    shellState.hasWritten = false;
                }
                else
                {
                    // calculate the length of the char in arrow buffer
                    int arrowBufferLength = 255 - shellState.arrowBufferIndex;
                    int arrow_index = shellState.arrowBufferIndex;
                    int command_index = shellState.bufferIndex;

                    // put all the char in arrow buffer to temp to print out and also overwrite the command buffer part
                    char temp[256];
                    for (int i = 0; i < arrowBufferLength; i++)
                    {
                        arrow_index++;
                        temp[i] = shellState.arrowBuffer[arrow_index];
                        shellState.commandBuffer[command_index - 1] = temp[i];
                        command_index++;
                    }

                    // finish up temp
                    temp[arrowBufferLength] = ' ';
                    temp[arrowBufferLength + 1] = '\0';
                    shellState.bufferIndex--;

                    // delete the previous char
                    syscall(10, (uint32_t)row, (uint32_t)col - 1, 0);
                    syscall(6, (uint32_t)temp, 0xF, 0);
                    // set the position back in the editing space
                    syscall(10, (uint32_t)row, (uint32_t)col - 1, 0);
                    copy_single_line();
                }
            }
        }
        else if (currChar == '\b' && shellState.bufferIndex == 0) // backspacing at the beginning
        {
            shellState.hasWritten = false;
        }
        else if (currChar == '\b') // normal backspacing
        {
            shellState.recentsReadIndex = shellState.recentsWriteIndex;
            copy_single_line();
            shellState.bufferIndex--;
            shellState.commandBuffer[shellState.bufferIndex] = 0;
            syscall(5, (uint32_t)&currChar, (uint32_t)0xF, 0);
            if (shellState.bufferIndex == 0)
            {
                shellState.hasWritten = false;
            }
        }
        else
        {
            shellState.recentsReadIndex = shellState.recentsWriteIndex;
            shellState.commandBuffer[shellState.bufferIndex] = currChar;
            shellState.bufferIndex++;
            copy_single_line();
            syscall(5, (uint32_t)&currChar, (uint32_t)0xF, 0);
        }
    }
}

int main(void)
{
    // struct ClusterBuffer cl = {0};
    struct FAT32DriverRequest request = {
        .buf = (uint8_t *)0,
        .name = "shell",
        .ext = "\0\0\0",
        .parent_cluster_number = ROOT_CLUSTER_NUMBER,
        .buffer_size = 0x100000,
    };
    int32_t retcode;
    // if(retcode == 0){
    //     syscall(5, (uint32_t) 'c', 0xF, 0);

    // }
    syscall(0, (uint32_t)&request, (uint32_t)&retcode, 0);
    if (retcode == 0)
    {
        // syscall(6, (uint32_t) "owo\n", 4, 0xF);
    }

    syscall(7, 0, 0, 0);
    print_shell_prompt();
    while (true)
    {
        use_keyboard();
    }

    return 0;
}
