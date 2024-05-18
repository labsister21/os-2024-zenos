#include "./header/user-shell.h"
#include <stdint.h>
#include "header/filesystem/fat32.h"
#include "header/stdlib/string.h"

// #define strsplit(str,delim,result) syscall(20, (uint32_t) str, (uint32_t) delim, (uint32_t) result)
// #define strlen(str,strlenvar) syscall(21, (uint32_t) str, (uint32_t) &strlenvar, 0)
// #define strcpy(dest,src) syscall(22, (uint32_t) dest, (uint32_t) src, 0)
#define get_dir(curr_parent_cluster_number, table) syscall(23, (uint32_t)curr_parent_cluster_number, (uint32_t)table, 0)
#define set_dir(curr_parent_cluster_number, table) syscall(25, (uint32_t)curr_parent_cluster_number, (uint32_t)table, 0)

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

// void deleteAll(uint32_t current_cluster_number, int *retcode)
// {
//     struct FAT32DirectoryTable dirTable;
//     syscall(23, current_cluster_number, (uint32_t)&dirTable, 0);
//     int x;
//     for (x = 2; x < 64; x++)
//     {
//         if (dirTable.table[x].name[0] != '\0')
//         {
//             char outText[4 * 512 * 512];
//             struct FAT32DriverRequest req = {
//                 .buf = outText,

//             };
//             req.parent_cluster_number = current_cluster_number;
//             req.buffer_size = dirTable.table[x].filesize;
//             memcpy(req.name, dirTable.table[x].name, 8);
//             memcpy(req.ext, dirTable.table[x].ext, 3);
//             if (dirTable.table[x].attribute == ATTR_SUBDIRECTORY)
//             {
//                 uint32_t nextClusterNumber = dirTable.table[x].cluster_high << 16 | dirTable.table[x].cluster_low;
//                 deleteAll(nextClusterNumber, retcode);
//             }
//             syscall(3, (uint32_t)&req, (uint32_t)retcode, 0);
//         }
//     }
// }

void finPath(char *destination, uint32_t current_cluster_number, char path[256], bool *found)
{
    struct FAT32DirectoryTable dirTable;
    get_dir(current_cluster_number, &dirTable);
    int x;
    for (x = 2; x < 64; x++)
    {
        if (strcmp(dirTable.table[x].name, destination) == 0)
        {
            char temp[256];
            strncpy(temp, path, 256);
            strcat(temp, "/");

            strcat(temp, dirTable.table[x].name);
            syscall(6, (uint32_t)temp, 0xf, 0);
            if (dirTable.table[x].filesize > 0)
            {
                syscall(6, (uint32_t) ".", 0xf, 0);
                for (int i = 0; i < 3; i++)
                {
                    syscall(5, (uint32_t)&dirTable.table[x].ext[i], 0xf, 0);
                }
            }
            syscall(6, (uint32_t) "\n", 0xf, 0);
            *found = true;
        }
        if (dirTable.table[x].attribute == ATTR_SUBDIRECTORY)
        {
            char temp[256];
            strncpy(temp, path, 256);
            strcat(temp, "/");
            strcat(temp, dirTable.table[x].name);
            uint32_t next_number = dirTable.table[x].cluster_high << 16 | dirTable.table[x].cluster_low;
            finPath(destination, next_number, temp, found);
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

    char buffer[16][256] = {0};
    char arg[128] = {0};
    strsplit(shellState.commandBuffer, ' ', buffer);
    int countCommands = 0;
    for (int z = 0; z < 16; z++)
    {
        if (buffer[z][0] != 0)
        {
            countCommands++;
        }
    }

    if (strcmp(buffer[0], "cd") == 0)
    {
        if (countCommands > 2)
        {
            syscall(6, (uint32_t) "cd: too many arguments\n\n", 0x4, 0);
            reset_shell_buffer();
            print_shell_prompt();
            return;
        }

        strcpy(arg, buffer[1]);
        struct FAT32DirectoryTable table;
        get_dir(shellState.workDir, &table);
        // syscall(6,(uint32_t)arg, 0xf, 0);
        uint32_t parent_cluster_number = table.table[0].cluster_high << 16 | table.table[0].cluster_low;
        if (strcmp(arg, "..") == 0)
        {

            if (shellState.workDir <= 2)
            {

                reset_shell_buffer();
                print_shell_prompt();
                return;
            }
            char eachPath[16][256] = {0};
            char finalPath[256] = {0};
            strsplit(shellState.directory, '/', eachPath);
            int num_of_path = 0;
            for (int i = 0; i < 16; i++)
            {
                if (eachPath[i][0] != 0)
                {
                    num_of_path++;
                }
            }
            for (int j = 1; j < num_of_path; j++)
            {
                if (num_of_path != 1)
                {
                    strcat(finalPath, "/");
                }
                strcat(finalPath, eachPath[j]);
            }
            strncpy(shellState.directory, finalPath, (uint32_t)256);
            shellState.workDir = parent_cluster_number;
            reset_shell_buffer();
            print_shell_prompt();
            return;
        }
        else
        {

            for (int i = 1; i < 64; i++)
            {
                if (table.table[i].attribute == ATTR_SUBDIRECTORY)
                {
                    if (strcmp(table.table[i].name, arg) == 0)
                    {
                        strcat(shellState.directory, "/");
                        strcat(shellState.directory, arg);
                        shellState.workDir = table.table[i].cluster_high << 16 | table.table[i].cluster_low;
                        reset_shell_buffer();
                        print_shell_prompt();
                        return;
                    }
                }
            }
            char message[256];
            strcpy(message, "cd: ");
            strcat(message, buffer[1]);
            strcat(message, ": No such directory\n\n");
            syscall(6, (uint32_t)message, 0x4, 0);
        }
    }
    else if (strcmp(buffer[0], "ls") == 0)
    {
        if (countCommands > 1)
        {
            syscall(6, (uint32_t) "too many arguments\n\n", 0x4, 0);
            reset_shell_buffer();
            print_shell_prompt();
            return;
        }
        struct FAT32DirectoryTable dirTable;
        get_dir(shellState.workDir, &dirTable);

        uint8_t z;
        if (shellState.workDir == 2)
        {
            z = 3;
        }
        else
        {
            z = 2;
        }
        for (z = 3; z < 64; z++)
        {
            if ((strcmp(&dirTable.table[z].name[0], "\0")))
            {
                int x;
                for (x = 0; x < 8; x++)
                {
                    if (strcmp(&dirTable.table[z].name[x], "\0"))
                    {
                        syscall(5, (uint32_t)&dirTable.table[z].name[x], 0xF, 0);
                    }
                }
                if (dirTable.table[z].attribute == 0)
                {
                    char dot[1];
                    char *dotraw = ".";
                    memcpy(dot, dotraw, 1);
                    syscall(5, (uint32_t)dot, 0xF, 0);
                    int y;
                    for (y = 0; y < 3; y++)
                    {
                        syscall(5, (uint32_t)&dirTable.table[z].ext[y], 0xF, 0);
                    }
                }
                syscall(6, (uint32_t) "  ", 0xF, 0);
            }
        }
        syscall(6, (uint32_t) "\n\n", 0xF, 0);
    }
    else if (strcmp(buffer[0], "mkdir") == 0)
    {
        if (countCommands > 2)
        {
            syscall(6, (uint32_t) "too many arguments\n\n", 0x4, 0);
            reset_shell_buffer();
            print_shell_prompt();
            return;
        }
        struct FAT32DriverRequest req = {0};
        strcpy(req.name, buffer[1]);
        req.ext[0] = '\0';
        req.ext[1] = '\0';
        req.ext[2] = '\0';
        req.parent_cluster_number = shellState.workDir;
        req.buffer_size = 0;
        int8_t return_code;
        syscall(24, (uint32_t)&req, (uint32_t)&return_code, 0);
        if (return_code == -1)
        {
            syscall(6, (uint32_t) "Not enough space in the current working directory\n", 0x4, 0);
        }
        else if (return_code == 1)
        {
            strcat(buffer[0], ": ");
            strcat(buffer[0], "cannot create directory `");
            strcat(buffer[0], buffer[1]);
            strcat(buffer[0], "`: File exists\n\n");
            syscall(6, (uint32_t)buffer[0], 0x4, 0);
        }
    }
    else if (strcmp(buffer[0], "cat") == 0)
    {
        if (countCommands > 2)
        {
            syscall(6, (uint32_t) "cat: too many arguments\n\n", 0x4, 0);
            reset_shell_buffer();
            print_shell_prompt();
            return;
        }
        if (countCommands < 2)
        {
            syscall(6, (uint32_t) "cat: too few arguments\n\n", 0x4, 0);
            reset_shell_buffer();
            print_shell_prompt();
            return;
        }
        // parse file dengan extension
        char splitFilenameExt[16][256] = {0};
        strsplit(buffer[1], '.', splitFilenameExt);
        int countSplitResult = 0;
        for (int i = 0; i < 16; i++)
        {
            if (splitFilenameExt[i][0] != 0)
            {
                countSplitResult++;
            }
        }
        if (countSplitResult > 2)
        {
            // invalid file
            syscall(6, (uint32_t) "cat: ", 0x4, 0);
            syscall(6, (uint32_t)buffer[1], 0x4, 0);
            syscall(6, (uint32_t) ": No such file or directory\n\n", 0x4, 0);
        }
        int8_t retcode;
        char outText[4 * 512 * 512];
        struct FAT32DriverRequest requestReadFile = {
            .buf = outText,
        };

        memcpy(requestReadFile.name, splitFilenameExt[0], 8);
        memcpy(requestReadFile.ext, splitFilenameExt[1], 3);
        if (shellState.workDir == 2 && memcmp(requestReadFile.name, "shell\0\0", 8) == 0 && memcmp(requestReadFile.ext, "\0\0\0", 3) == 0)
        {
            syscall(6, (uint32_t) "cat: ", 0x4, 0);
            syscall(6, (uint32_t)buffer[1], 0x4, 0);
            syscall(6, (uint32_t) ": No such file or directory\n\n", 0x4, 0);
            reset_shell_buffer();
            print_shell_prompt();
            return;
        }
        requestReadFile.parent_cluster_number = shellState.workDir;
        requestReadFile.buffer_size = 4 * 512 * 512;
        syscall(0, (uint32_t)&requestReadFile, (uint32_t)&retcode, 0);
        if (retcode == 0)
        {
            syscall(6, (uint32_t)requestReadFile.buf, 0xF, 0);
            syscall(6, (uint32_t) "\n\n", 0xF, 0);
        }
        else
        {
            syscall(6, (uint32_t) "cat: ", 0x4, 0);
            syscall(6, (uint32_t)buffer[1], 0x4, 0);
            syscall(6, (uint32_t) ": No such file or directory\n\n", 0x4, 0);
        }
    }
    else if (strcmp(buffer[0], "mv") == 0)
    {
        if (countCommands > 3)
        {
            syscall(6, (uint32_t) "too many arguments\n\n", 0x4, 0);
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
        strsplit(buffer[1], '/', eachPathParam1);
        int countPathParam1 = 0;
        for (int i = 0; i < 16; i++)
        {
            if (eachPathParam1[i][0] != 0)
            {
                countPathParam1++;
            }
        }

        char eachPathParam2[16][256] = {0};
        strsplit(buffer[2], '/', eachPathParam2);
        int countPathParam2 = 0;
        for (int i = 0; i < 16; i++)
        {
            if (eachPathParam2[i][0] != 0)
            {
                countPathParam2++;
            }
        }

        // cek apakah buffer[1] merupakan nama file
        strsplit(buffer[1], '.', splitFilenameExt1);
        for (int i = 0; i < 16; i++)
        {
            if (splitFilenameExt1[i][0] != 0)
            {
                countSplitParam1++;
            }
            else
            {
                break;
            }
        }
        if (countSplitParam1 == 2)
        {
            for (int y = 0; y < 256; y++)
            {
                if (splitFilenameExt1[1][y] != 0)
                {
                    param1ExtLength++;
                }
                else
                {
                    break;
                }
            }
        }

        // cek apakah buffer[2] merupakan nama file
        strsplit(buffer[2], '.', splitFilenameExt2);
        for (int i = 0; i < 16; i++)
        {
            if (splitFilenameExt2[i][0] != 0)
            {
                countSplitParam2++;
            }
        }
        if (countSplitParam2 == 2)
        {
            for (int y = 0; y < 256; y++)
            {
                if (splitFilenameExt2[1][y] != 0)
                {
                    param2ExtLength++;
                }
                else
                {
                    break;
                }
            }
        }
        bool isFound = false;
        // Rename file
        if (countSplitParam1 == 2 && countSplitParam2 == 2 && param1ExtLength <= 3 && param2ExtLength <= 3 && countPathParam1 == 1 && countPathParam2 == 1)
        {
            struct FAT32DirectoryTable dirTable;
            get_dir(shellState.workDir, &dirTable);
            int x;
            for (x = 2; x < 64; x++)
            {
                if (memcmp(dirTable.table[x].name, splitFilenameExt1[0], 8) == 0 && memcmp(dirTable.table[x].ext, splitFilenameExt1[1], 3) == 0)
                {
                    memcpy(dirTable.table[x].name, splitFilenameExt2[0], 8);
                    memcpy(dirTable.table[x].ext, splitFilenameExt2[1], 3);
                    isFound = true;
                    break;
                };
            }
            if (isFound)
            {
                set_dir(shellState.workDir, &dirTable);
            }
            else
            {
                syscall(6, (uint32_t) "mv: ", 0x4, 0);
                syscall(6, (uint32_t)buffer[1], 0x4, 0);
                syscall(6, (uint32_t) ": No such file or directory\n\n", 0x4, 0);
            }
        }
        else
        {
            // Memindahkan file/directory

            // Ambil directory entry dari file/folder yang ingin dipindah
            struct FAT32DirectoryEntry tempDirEntry;
            uint32_t currParentCluster = ROOT_CLUSTER_NUMBER;
            char currName[8];
            char currExt[3];
            struct FAT32DirectoryTable tempDirTable;
            for (int j = 0; j < countPathParam1; j++)
            {
                isFound = false;
                get_dir(currParentCluster, &tempDirTable);
                memcpy(currExt, "\0\0\0", 3);
                if (j == countPathParam1 - 1)
                {
                    char splitFilenameExtTemp[16][256] = {0};
                    strsplit(eachPathParam1[j], '.', splitFilenameExtTemp);
                    int countSplitResult = 0;
                    for (int i = 0; i < 16; i++)
                    {
                        if (splitFilenameExtTemp[i][0] == '\0')
                        {
                            break;
                        }
                        else
                        {
                            countSplitResult++;
                        }
                    }
                    if (countSplitResult == 2)
                    {
                        int countExtLengthTemp = 0;
                        for (int y = 0; y < 256; y++)
                        {
                            if (splitFilenameExtTemp[1][y] == '\0')
                            {
                                break;
                            }
                            else
                            {
                                countExtLengthTemp++;
                            }
                        }
                        if (countExtLengthTemp <= 3)
                        {
                            memcpy(currExt, splitFilenameExtTemp[1], 3);
                            memcpy(currName, splitFilenameExtTemp[0], 8);
                        }
                        else
                        {
                            memcpy(currName, splitFilenameExtTemp[0], 8);
                        }
                    }
                    else
                    {
                        memcpy(currName, splitFilenameExtTemp[0], 8);
                    }
                }
                else
                {
                    memcpy(currName, eachPathParam1[j], 8);
                }
                for (uint8_t i = 2; i < 64; i++)
                {
                    if (memcmp(tempDirTable.table[i].name, currName, 8) == 0 && memcmp(tempDirTable.table[i].ext, currExt, 3) == 0)
                    {
                        if (j == countPathParam1 - 1)
                        {
                            memcpy(&tempDirEntry, &tempDirTable.table[i], 32);
                            memset(&tempDirTable.table[i], 0, 32);
                            set_dir(currParentCluster, &tempDirTable);
                        }
                        else
                        {
                            currParentCluster = tempDirTable.table[i].cluster_high << 16 | tempDirTable.table[i].cluster_low;
                        }
                        isFound = true;
                        break;
                    }
                }
                if (!isFound)
                {
                    break;
                }
            }

            if (!isFound)
            {
                syscall(6, (uint32_t) "mv: ", 0x4, 0);
                syscall(6, (uint32_t)buffer[1], 0x4, 0);
                syscall(6, (uint32_t) ": No such file or directory\n\n", 0x4, 0);
            }
            else
            {
                // tempDirEntry sudah didapatkan, selanjutnya mencari cluster number dari tempat tujuan mv
                currParentCluster = ROOT_CLUSTER_NUMBER;
                for (int j = 0; j < countPathParam2; j++)
                {
                    isFound = false;
                    get_dir(currParentCluster, &tempDirTable);
                    memcpy(currExt, "\0\0\0", 3);
                    memcpy(currName, eachPathParam2[j], 8);
                    for (uint8_t i = 2; i < 64; i++)
                    {
                        if (memcmp(tempDirTable.table[i].name, currName, 8) == 0 && memcmp(tempDirTable.table[i].ext, currExt, 3) == 0)
                        {
                            currParentCluster = tempDirTable.table[i].cluster_high << 16 | tempDirTable.table[i].cluster_low;
                            isFound = true;
                            break;
                        }
                    }
                    if (!isFound)
                    {
                        break;
                    }
                }
                if (!isFound)
                {
                    syscall(6, (uint32_t) "mv: ", 0x4, 0);
                    syscall(6, (uint32_t)buffer[1], 0x4, 0);
                    syscall(6, (uint32_t) ": No such file or directory\n\n", 0x4, 0);
                }
                else
                {
                    get_dir(currParentCluster, &tempDirTable);
                    for (int x = 2; x < 64; x++)
                    {
                        if (tempDirTable.table[x].name[0] == '\0')
                        {
                            memcpy(&tempDirTable.table[x], &tempDirEntry, 32);
                            set_dir(currParentCluster, &tempDirTable);
                            break;
                        }
                    }
                }
            }
        }
    }
    else if (strcmp(buffer[0], "find") == 0)
    {
        char temp[256] = {0};
        bool found = false;
        if (countCommands > 2)
        {
            syscall(6, (uint32_t) "find: too many arguments\n\n", 0x4, 0);
            reset_shell_buffer();
            print_shell_prompt();
            return;
        }

        if (countCommands < 2)
        {
            syscall(6, (uint32_t) "find: too few arguments\n\n", 0x4, 0);
            reset_shell_buffer();
            print_shell_prompt();
            return;
        }

        finPath(buffer[1], 2, temp, &found);
        if (!found)
        {
            char message[256];
            strcpy(message, "find: ");
            strcat(message, buffer[1]);
            strcat(message, ": No such file/directory\n\n");
            syscall(6, (uint32_t)message, 0x4, 0);
            reset_shell_buffer();
            print_shell_prompt();
            return;
        }
        syscall(6, (uint32_t) "\n", 0xf, 0);
    }
    else if (strcmp(buffer[0], "rm") == 0)
    {
        bool found = false;
        struct FAT32DirectoryTable dirTable;
        get_dir(shellState.workDir, &dirTable);
        // determining if file or not
        char result[16][256] = {0};
        strsplit(buffer[1], '.', result);
        uint32_t startEntry = 2;
        if (shellState.workDir == 2)
        {
            startEntry = 3;
        }
        if (result[1][0] != '\0' && strlen(result[1]) <= 3)
        {
            // deleting file
            // searching for file with same extension
            for (int i = startEntry; i < 64; i++)
            {
                if ((memcmp(dirTable.table[i].name, result[0], 8) == 0) && (memcmp(dirTable.table[i].ext, result[1], 3) == 0) && (dirTable.table[i].name[0] != '\0'))
                {
                    // struct ClusterBuffer cl = {0};
                    char outText[4 * 512 * 512];
                    struct FAT32DriverRequest req = {
                        .buf = outText,

                    };
                    req.parent_cluster_number = shellState.workDir;
                    req.buffer_size = dirTable.table[i].filesize;
                    memcpy(req.name, dirTable.table[i].name, 8);
                    memcpy(req.ext, dirTable.table[i].ext, 3);

                    int retcode;
                    syscall(3, (uint32_t)&req, (uint32_t)&retcode, 0);
                    if (retcode == 0)
                    {
                        syscall(6, (uint32_t) "Success!\n\n", 0xf, 0);
                    }
                    else
                    {
                        syscall(6, (uint32_t) "Fail..", 0x4, 0);
                    }
                    found = true;
                    break;
                }
            }
        }
        else
        {
            // delete folder
            //  for (int i = startEntry ; i < 64 ; i++){
            //     if ( (memcmp(dirTable.table[i].name ,result[0],8) == 0) && (memcmp(dirTable.table[i].ext, result[1],3) == 0) && (dirTable.table[i].name[0] != '\0')){
            //         // struct ClusterBuffer cl = {0};
            //         char outText[4*512*512];
            //         struct FAT32DriverRequest req = {
            //             .buf                   = outText,

            //         };
            //         req.parent_cluster_number = shellState.workDir;
            //         req.buffer_size = dirTable.table[i].filesize;
            //         memcpy(req.name,dirTable.table[i].name, 8);
            //         memcpy(req.ext,dirTable.table[i].ext,3);

            //         int retcode;
            //         uint32_t nextClusterNumber = dirTable.table[i].cluster_high << 16 | dirTable.table[i].cluster_low;
            //         deleteAll(nextClusterNumber,&retcode);
            //         syscall(3, (uint32_t) &req, (uint32_t) &retcode,0);
            //         if (retcode == 0){
            //             syscall(6, (uint32_t)"Success!\n\n",0xf,0);
            //         } else{
            //             syscall(6, (uint32_t)"Fail..", 0x4,0);
            //         }
            //         found = true;
            //         break;
            //     }
            // }
            syscall(6, (uint32_t) "Can't delete folder\n\n", 0x4, 0);
        }
        if (!found)
        {
            syscall(6, (uint32_t) "File/Directory does not exists\n\n", 0x4, 0);
        }
    }
    else if (strcmp(buffer[0], "cp") == 0)
    {
        // do copy
        if (countCommands > 3 || countCommands < 2)
        {
            syscall(6, (uint32_t) "cp: invalid arguments\n\n", 0x4, 0);
            reset_shell_buffer();
            print_shell_prompt();
            return;
        }

        // splitting first path and second path
        char path1[16][256];
        char path2[16][256];
        strsplit(buffer[1], '/', path1);
        strsplit(buffer[2], '/', path2);

        // search for file, note that extension is only at the end
        char outText[4 * 512 * 512];
        struct FAT32DriverRequest req = {
            .buf = outText,
            .buffer_size = 4 * 512 * 512,
            .name = "\0\0\0\0\0\0\0",
            .ext = "\0\0\0",
        };

        req.parent_cluster_number = ROOT_CLUSTER_NUMBER;
        uint8_t i = 0;

        // uint32_t prev_dir = ROOT_CLUSTER_NUMBER;
        uint32_t curr_dir = ROOT_CLUSTER_NUMBER;
        int8_t retcode = -1;
        while (path1[i][0] != '\0' && i < 15)
        {
            char fileName[16][256] = {0};

            // possible seg fault
            // if final destionation, in file/folder format
            if (path1[i + 1][0] == '\0')
            {
                strsplit(path1[i], '.', fileName);
                if (fileName[1][0] == '\0')
                {
                    syscall(6, (uint32_t) "cannot copy a folder!\n\n", 0x4, 0);
                }
            }
            else
            {
                memcpy(fileName[0], path1[i], 8);
            }
            memcpy(req.name, fileName[0], 8);
            memcpy(req.ext, fileName[1], 3);
            syscall(50, curr_dir, (uint32_t)&req, (uint32_t)&retcode);

            if (retcode == -1)
            {
                // unable to find it
                break;
            }
            else
            {
                struct FAT32DirectoryTable table_dir;
                get_dir(curr_dir, &table_dir);
                if (table_dir.table[retcode].attribute == ATTR_SUBDIRECTORY)
                {
                    // currently is folder, find next folder
                    req.parent_cluster_number = table_dir.table[retcode].cluster_high << 16 | table_dir.table[retcode].cluster_low;
                    // prev_dir = curr_dir;
                    curr_dir = req.parent_cluster_number;
                    i++;
                }
                else
                {
                    // file found
                    break;
                }
            }
        }
        if (retcode == -1)
        {
            syscall(6, (uint32_t) "file/folder does not exists\n\n", 0x4, 0);
            reset_shell_buffer();
            print_shell_prompt();
            return;
        }
        else
        {
            // find location for next copy process
            char outText[4 * 512 * 512];
            struct FAT32DriverRequest reqTarget = {
                .buf = outText,
                .buffer_size = CLUSTER_SIZE,
                .name = "\0\0\0\0\0\0\0",
                .ext = "\0\0\0",
            };
            reqTarget.parent_cluster_number = ROOT_CLUSTER_NUMBER;
            uint8_t j = 0;
            int8_t targetRetcode = 0;
            uint8_t targetCurrDir = ROOT_CLUSTER_NUMBER;
            // bool copy_to_folder = false;
            while (path2[j][0] != '\0' && j < 15)
            {
                memcpy(reqTarget.name, path2[j], 8);
                syscall(50, targetCurrDir, (uint32_t)&reqTarget, (uint32_t)&targetRetcode);
                if (targetRetcode == -1)
                {
                    syscall(6, (uint32_t) "target file/folder does not exists\n\n", 0x4, 0);
                    reset_shell_buffer();
                    print_shell_prompt();
                    return;
                }
                else
                {
                    struct FAT32DirectoryTable table_dir;
                    get_dir(targetCurrDir, &table_dir);
                    if (table_dir.table[targetRetcode].attribute == ATTR_SUBDIRECTORY)
                    {
                        targetCurrDir = table_dir.table[targetRetcode].cluster_high << 16 | table_dir.table[targetRetcode].cluster_low;
                        reqTarget.parent_cluster_number = targetCurrDir;
                        j++;
                    }
                    else
                    {
                        syscall(6, (uint32_t) "Something went wrong \n\n", 0x4, 0);
                        reset_shell_buffer();
                        print_shell_prompt();
                        return;
                    }
                }
            }
            int8_t returnCodeCopy;

            syscall(0, (uint32_t)&req, (uint32_t)&returnCodeCopy, 0);
            if (returnCodeCopy != 0)
            {
                syscall(6, (uint32_t) "Something went wrong with the copying process! \n\n", 0x4, 0);
                reset_shell_buffer();
                print_shell_prompt();
                return;
            }
            uint32_t filesize;
            filesize = strlen(req.buf);
            reqTarget.buffer_size = filesize;
            memcpy(reqTarget.buf, req.buf, reqTarget.buffer_size);
            memcpy(reqTarget.name, req.name, 8);
            memcpy(reqTarget.ext, req.ext, 3);
            syscall(24, (uint32_t)&reqTarget, (uint32_t)&returnCodeCopy, 0);
            if (returnCodeCopy != 0)
            {
                syscall(6, (uint32_t) "Something went wrong with the writing process! \n\n", 0x4, 0);
                reset_shell_buffer();
                print_shell_prompt();
                return;
            }
            else
            {
                syscall(6, (uint32_t) "Success ! \n\n", 0xf, 0);
                reset_shell_buffer();
                print_shell_prompt();
                return;
            }
        }
    }
    else if (strcmp(buffer[0], "ps") == 0)
    {
        // split untuk nama proses
        if (countCommands > 3)
        {
            char message[256];
            strcpy(message, "ps");
            strcat(message, ": invalid commands\n\n");
            reset_shell_buffer();
            print_shell_prompt();
            return;
        }
        else
        {
            char process_names[16][256] = {0};
            uint32_t process_ids[16] = {0};
            syscall(51, (uint32_t)buffer[1], (uint32_t)process_names, (uint32_t)process_ids);
            char space = ' ';
            char newLine = '\n';
            for (int i = 0; i < 16; i++)
            {
                if (process_names[i][0] != 0)
                {
                    syscall(6, (uint32_t)process_names[i], 0xf, 0);

                    syscall(5, (uint32_t)&space, 0xf, 0);
                    char id = process_ids[i] + '0';
                    syscall(5, (uint32_t)&id, 0xf, 0);
                    syscall(5, (uint32_t)&newLine, 0xf, 0);
                }
            }
            syscall(5, (uint32_t)&newLine, 0xf, 0);
        }
    }
    else if (strcmp(buffer[0], "exec") == 0)
    {
        if (countCommands != 2)
        {
            // something went wrong
        }
        else
        {
            // splitting first path and second path
            char path1[16][256];
            strsplit(buffer[1], '/', path1);

            // search for file, note that extension is only at the end
            char outText[4 * 512 * 512];
            struct FAT32DriverRequest req = {
                .buf = outText,
                .buffer_size = 4 * 512 * 512,
                .name = "\0\0\0\0\0\0\0",
                .ext = "\0\0\0",
            };

            req.parent_cluster_number = shellState.workDir;
            uint8_t i = 0;

            // uint32_t prev_dir = ROOT_CLUSTER_NUMBER;
            uint32_t curr_dir = shellState.workDir;
            int8_t retcode = -1;
            while (path1[i][0] != '\0' && i < 15)
            {
                char fileName[16][256] = {0};

                // possible seg fault
                // if final destionation, in file/folder format
                if (path1[i + 1][0] == '\0')
                {
                    strsplit(path1[i], '.', fileName);
                    // if (fileName[1][0] == '\0')
                    // {
                    //     syscall(6, (uint32_t) "cannot copy a folder!\n\n", 0x4, 0);
                    // }
                }
                else
                {
                    memcpy(fileName[0], path1[i], 8);
                }
                memcpy(req.name, fileName[0], 8);
                memcpy(req.ext, fileName[1], 3);
                syscall(50, curr_dir, (uint32_t)&req, (uint32_t)&retcode);

                if (retcode == -1)
                {
                    // unable to find it
                    break;
                }
                else
                {
                    struct FAT32DirectoryTable table_dir;
                    get_dir(curr_dir, &table_dir);
                    if (table_dir.table[retcode].attribute == ATTR_SUBDIRECTORY)
                    {
                        // currently is folder, find next folder
                        req.parent_cluster_number = table_dir.table[retcode].cluster_high << 16 | table_dir.table[retcode].cluster_low;
                        // prev_dir = curr_dir;
                        curr_dir = req.parent_cluster_number;
                        i++;
                    }
                    else
                    {
                        // file found
                        break;
                    }
                }
            }
            if (retcode == -1)
            {
                syscall(6, (uint32_t) "file/folder does not exists\n\n", 0x4, 0);
                reset_shell_buffer();
                print_shell_prompt();
                return;
            }
            else
            {
                // have found the file needed
                // write process !
                int32_t return_code = 0;
                syscall(52, (uint32_t)&req, (uint32_t)&return_code, 0);
                if (return_code != 0)
                {
                    syscall(6, (uint32_t) "Something went wrong..\n\n", 0x4, 0);
                    reset_shell_buffer();
                    print_shell_prompt();
                    return;
                }
                else
                {
                    syscall(6, (uint32_t) "Success! \n\n", 0xF, 0);
                    reset_shell_buffer();
                    print_shell_prompt();
                    return;
                }
            }
        }
    }
    else if (strcmp(buffer[0], "kill") == 0)
    {
        if (countCommands != 2)
        {
            syscall(6, (uint32_t) "Success! \n\n", 0x4, 0);
            reset_shell_buffer();
            print_shell_prompt();
            return;
        }

        bool retcode = false; // ebx
        uint32_t pid = 0;
        for (int i = 0; buffer[1][i] != '\0'; i++)
        {
            pid = pid * 10 + (buffer[1][i] - '0');
        }
        if (pid == 0)
        {
            syscall(6, (uint32_t) "Failed! \n\n", 0x4, 0);
            reset_shell_buffer();
            print_shell_prompt();
            return;
        }
        syscall(14, (uint32_t)&retcode, pid, 0);
        if (retcode)
        {
            syscall(6, (uint32_t) "Success! \n\n", 0xf, 0);
            reset_shell_buffer();
            print_shell_prompt();
            return;
        }
        else
        {
            syscall(6, (uint32_t) "Failed! \n\n", 0x4, 0);
            reset_shell_buffer();
            print_shell_prompt();
            return;
        }
    }
    else
    {
        strcat(buffer[0], ": ");
        strcat(buffer[0], "command not found\n\n");
        syscall(6, (uint32_t)buffer[0], 0x4, 0);
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
        .buf = (uint8_t *)0,
        .name = "shell",
        .ext = "\0\0\0",
        .parent_cluster_number = ROOT_CLUSTER_NUMBER,
        .buffer_size = 0x100000,
    };

    int32_t retcode;
    syscall(0, (uint32_t)&request, (uint32_t)&retcode, 0);
    if (retcode == 0)
    {
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
