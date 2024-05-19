#include "header/buddy.h"

void syscallBuddy(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx)
{
    __asm__ volatile("mov %0, %%ebx" : /* <Empty> */ : "r"(ebx));
    __asm__ volatile("mov %0, %%ecx" : /* <Empty> */ : "r"(ecx));
    __asm__ volatile("mov %0, %%edx" : /* <Empty> */ : "r"(edx));
    __asm__ volatile("mov %0, %%eax" : /* <Empty> */ : "r"(eax));
    // Note : gcc usually use %eax as intermediate register,
    //        so it need to be the last one to mov
    __asm__ volatile("int $0x30");
}

void delay()
{
    int32_t i = 0;
    while (i < 10000)
    {
        int32_t j = 0;
        while (j < 10000)
        {
            j++;
        }
        i++;
    }
}

int main(void)
{
    char circle = 'o';
    char vertical_strip = '|';
    char left_strip = '/';
    char right_strip = '\\';
    char blank = ' ';
    char under_score = '_';
    char left_bracket = '(';
    char right_bracket = ')';
    char str_buddy[6] = "Buddy\0";
    char str_incoming[9] = "Incoming\0";
    char str_buddy_clear[6] = "     \0";
    char str_incoming_clear[9] = "        \0";

    char brainrot[3][9];
    memcpy(brainrot[0], "skibidi", strlen("skibidi") + 1);
    memcpy(brainrot[1], "rizz", strlen("rizz") + 1);
    memcpy(brainrot[2], "gyatt", strlen("gyatt") + 1);

    char brainrot_clear[3][9];
    memcpy(brainrot_clear[0], "       ", strlen("       ") + 1);
    memcpy(brainrot_clear[1], "    ", strlen("    ") + 1);
    memcpy(brainrot_clear[2], "     ", strlen("     ") + 1);

    // for buddy incoming text
    syscallBuddy(18, (uint32_t)str_buddy, 22, 5);
    syscallBuddy(18, (uint32_t)str_incoming, 23, 5);

    for (int i = 0; i < 5; i++)
    {
        // first part of body
        syscallBuddy(17, (uint32_t)&circle, 22, 16);
        syscallBuddy(17, (uint32_t)&left_strip, 23, 15);
        syscallBuddy(17, (uint32_t)&vertical_strip, 23, 16);
        syscallBuddy(17, (uint32_t)&right_strip, 23, 17);
        syscallBuddy(17, (uint32_t)&left_strip, 24, 15);
        syscallBuddy(17, (uint32_t)&right_strip, 24, 17);

        // clearing out the first part
        delay();
        syscallBuddy(17, (uint32_t)&blank, 22, 16);
        syscallBuddy(17, (uint32_t)&blank, 23, 15);
        syscallBuddy(17, (uint32_t)&blank, 23, 16);
        syscallBuddy(17, (uint32_t)&blank, 23, 17);
        syscallBuddy(17, (uint32_t)&blank, 24, 15);
        syscallBuddy(17, (uint32_t)&blank, 24, 17);

        // making the second part
        syscallBuddy(17, (uint32_t)&circle, 22, 16);
        syscallBuddy(17, (uint32_t)&right_strip, 22, 15);
        syscallBuddy(17, (uint32_t)&vertical_strip, 23, 16);
        syscallBuddy(17, (uint32_t)&left_strip, 22, 17);
        syscallBuddy(17, (uint32_t)&left_strip, 24, 15);
        syscallBuddy(17, (uint32_t)&right_strip, 24, 17);

        delay();
        syscallBuddy(17, (uint32_t)&blank, 22, 16);
        syscallBuddy(17, (uint32_t)&blank, 22, 15);
        syscallBuddy(17, (uint32_t)&blank, 23, 16);
        syscallBuddy(17, (uint32_t)&blank, 22, 17);
        syscallBuddy(17, (uint32_t)&blank, 24, 15);
        syscallBuddy(17, (uint32_t)&blank, 24, 17);
    }

    // first part of body
    syscallBuddy(17, (uint32_t)&circle, 22, 16);
    syscallBuddy(17, (uint32_t)&left_strip, 23, 15);
    syscallBuddy(17, (uint32_t)&vertical_strip, 23, 16);
    syscallBuddy(17, (uint32_t)&right_strip, 23, 17);
    syscallBuddy(17, (uint32_t)&left_strip, 24, 15);
    syscallBuddy(17, (uint32_t)&right_strip, 24, 17);

    // clearing out the first part
    delay();
    syscallBuddy(17, (uint32_t)&blank, 22, 16);
    syscallBuddy(17, (uint32_t)&blank, 23, 15);
    syscallBuddy(17, (uint32_t)&blank, 23, 16);
    syscallBuddy(17, (uint32_t)&blank, 23, 17);
    syscallBuddy(17, (uint32_t)&blank, 24, 15);
    syscallBuddy(17, (uint32_t)&blank, 24, 17);

    // making the second part
    syscallBuddy(17, (uint32_t)&circle, 22, 22);
    syscallBuddy(17, (uint32_t)&right_strip, 22, 21);
    syscallBuddy(17, (uint32_t)&vertical_strip, 23, 22);
    syscallBuddy(17, (uint32_t)&left_strip, 22, 23);
    syscallBuddy(17, (uint32_t)&left_strip, 24, 21);
    syscallBuddy(17, (uint32_t)&right_strip, 24, 23);

    delay();
    syscallBuddy(17, (uint32_t)&blank, 22, 22);
    syscallBuddy(17, (uint32_t)&blank, 22, 21);
    syscallBuddy(17, (uint32_t)&blank, 23, 22);
    syscallBuddy(17, (uint32_t)&blank, 22, 23);
    syscallBuddy(17, (uint32_t)&blank, 24, 21);
    syscallBuddy(17, (uint32_t)&blank, 24, 23);

    // making the third part
    syscallBuddy(17, (uint32_t)&under_score, 22, 27);
    syscallBuddy(17, (uint32_t)&circle, 22, 29);
    syscallBuddy(17, (uint32_t)&left_strip, 23, 28);
    syscallBuddy(17, (uint32_t)&right_strip, 23, 29);
    syscallBuddy(17, (uint32_t)&vertical_strip, 24, 27);
    syscallBuddy(17, (uint32_t)&right_strip, 24, 29);

    delay();
    syscallBuddy(17, (uint32_t)&blank, 22, 27);
    syscallBuddy(17, (uint32_t)&blank, 22, 29);
    syscallBuddy(17, (uint32_t)&blank, 23, 28);
    syscallBuddy(17, (uint32_t)&blank, 23, 29);
    syscallBuddy(17, (uint32_t)&blank, 24, 27);
    syscallBuddy(17, (uint32_t)&blank, 24, 29);

    // making the fourth part
    syscallBuddy(17, (uint32_t)&under_score, 23, 33);
    syscallBuddy(17, (uint32_t)&under_score, 23, 34);
    syscallBuddy(17, (uint32_t)&under_score, 23, 35);
    syscallBuddy(17, (uint32_t)&right_strip, 23, 36);
    syscallBuddy(17, (uint32_t)&circle, 23, 37);
    syscallBuddy(17, (uint32_t)&left_strip, 24, 32);
    syscallBuddy(17, (uint32_t)&right_bracket, 24, 33);
    syscallBuddy(17, (uint32_t)&vertical_strip, 24, 36);

    delay();
    syscallBuddy(17, (uint32_t)&blank, 23, 33);
    syscallBuddy(17, (uint32_t)&blank, 23, 34);
    syscallBuddy(17, (uint32_t)&blank, 23, 35);
    syscallBuddy(17, (uint32_t)&blank, 23, 36);
    syscallBuddy(17, (uint32_t)&blank, 23, 37);
    syscallBuddy(17, (uint32_t)&blank, 24, 32);
    syscallBuddy(17, (uint32_t)&blank, 24, 33);
    syscallBuddy(17, (uint32_t)&blank, 24, 36);

    // making the fift part
    syscallBuddy(17, (uint32_t)&under_score, 22, 38);
    syscallBuddy(17, (uint32_t)&under_score, 22, 39);
    syscallBuddy(17, (uint32_t)&under_score, 22, 40);
    syscallBuddy(17, (uint32_t)&vertical_strip, 22, 41);
    syscallBuddy(17, (uint32_t)&right_strip, 23, 41);
    syscallBuddy(17, (uint32_t)&circle, 23, 42);
    syscallBuddy(17, (uint32_t)&left_bracket, 24, 41);
    syscallBuddy(17, (uint32_t)&right_strip, 24, 43);

    delay();
    syscallBuddy(17, (uint32_t)&blank, 22, 38);
    syscallBuddy(17, (uint32_t)&blank, 22, 39);
    syscallBuddy(17, (uint32_t)&blank, 22, 40);
    syscallBuddy(17, (uint32_t)&blank, 22, 41);
    syscallBuddy(17, (uint32_t)&blank, 23, 41);
    syscallBuddy(17, (uint32_t)&blank, 23, 42);
    syscallBuddy(17, (uint32_t)&blank, 24, 41);
    syscallBuddy(17, (uint32_t)&blank, 24, 43);

    // making the sixth part
    syscallBuddy(17, (uint32_t)&right_strip, 22, 46);
    syscallBuddy(17, (uint32_t)&left_strip, 22, 48);
    syscallBuddy(17, (uint32_t)&vertical_strip, 23, 47);
    syscallBuddy(17, (uint32_t)&left_strip, 24, 46);
    syscallBuddy(17, (uint32_t)&circle, 24, 47);
    syscallBuddy(17, (uint32_t)&right_strip, 24, 48);

    delay();
    syscallBuddy(17, (uint32_t)&blank, 22, 46);
    syscallBuddy(17, (uint32_t)&blank, 22, 48);
    syscallBuddy(17, (uint32_t)&blank, 23, 47);
    syscallBuddy(17, (uint32_t)&blank, 24, 46);
    syscallBuddy(17, (uint32_t)&blank, 24, 47);
    syscallBuddy(17, (uint32_t)&blank, 24, 48);

    // making the seventh part
    syscallBuddy(17, (uint32_t)&vertical_strip, 22, 54);
    syscallBuddy(17, (uint32_t)&under_score, 22, 55);
    syscallBuddy(17, (uint32_t)&under_score, 22, 56);
    syscallBuddy(17, (uint32_t)&under_score, 22, 57);
    syscallBuddy(17, (uint32_t)&circle, 23, 52);
    syscallBuddy(17, (uint32_t)&left_strip, 23, 53);
    syscallBuddy(17, (uint32_t)&left_strip, 24, 51);
    syscallBuddy(17, (uint32_t)&right_bracket, 24, 53);

    delay();
    syscallBuddy(17, (uint32_t)&blank, 22, 54);
    syscallBuddy(17, (uint32_t)&blank, 22, 55);
    syscallBuddy(17, (uint32_t)&blank, 22, 56);
    syscallBuddy(17, (uint32_t)&blank, 22, 57);
    syscallBuddy(17, (uint32_t)&blank, 23, 52);
    syscallBuddy(17, (uint32_t)&blank, 23, 53);
    syscallBuddy(17, (uint32_t)&blank, 24, 51);
    syscallBuddy(17, (uint32_t)&blank, 24, 53);

    // making the eight part
    syscallBuddy(17, (uint32_t)&circle, 23, 58);
    syscallBuddy(17, (uint32_t)&left_strip, 23, 59);
    syscallBuddy(17, (uint32_t)&under_score, 23, 60);
    syscallBuddy(17, (uint32_t)&under_score, 23, 61);
    syscallBuddy(17, (uint32_t)&vertical_strip, 24, 58);
    syscallBuddy(17, (uint32_t)&left_bracket, 24, 61);
    syscallBuddy(17, (uint32_t)&right_strip, 24, 62);

    delay();
    syscallBuddy(17, (uint32_t)&blank, 23, 58);
    syscallBuddy(17, (uint32_t)&blank, 23, 59);
    syscallBuddy(17, (uint32_t)&blank, 23, 60);
    syscallBuddy(17, (uint32_t)&blank, 23, 61);
    syscallBuddy(17, (uint32_t)&blank, 24, 58);
    syscallBuddy(17, (uint32_t)&blank, 24, 61);
    syscallBuddy(17, (uint32_t)&blank, 24, 62);

    // making the nine part
    syscallBuddy(17, (uint32_t)&circle, 22, 65);
    syscallBuddy(17, (uint32_t)&under_score, 22, 67);
    syscallBuddy(17, (uint32_t)&left_strip, 23, 65);
    syscallBuddy(17, (uint32_t)&right_strip, 23, 66);
    syscallBuddy(17, (uint32_t)&left_strip, 24, 65);
    syscallBuddy(17, (uint32_t)&vertical_strip, 24, 67);

    delay();
    syscallBuddy(17, (uint32_t)&blank, 22, 65);
    syscallBuddy(17, (uint32_t)&blank, 22, 67);
    syscallBuddy(17, (uint32_t)&blank, 23, 65);
    syscallBuddy(17, (uint32_t)&blank, 23, 66);
    syscallBuddy(17, (uint32_t)&blank, 24, 65);
    syscallBuddy(17, (uint32_t)&blank, 24, 67);

    syscallBuddy(18, (uint32_t)str_buddy_clear, 22, 5);
    syscallBuddy(18, (uint32_t)str_incoming_clear, 23, 5);

    // making the nine part
    syscallBuddy(17, (uint32_t)&circle, 22, 71);
    syscallBuddy(17, (uint32_t)&under_score, 23, 69);
    syscallBuddy(17, (uint32_t)&under_score, 23, 70);
    syscallBuddy(17, (uint32_t)&vertical_strip, 23, 71);
    syscallBuddy(17, (uint32_t)&right_strip, 23, 72);
    syscallBuddy(17, (uint32_t)&left_strip, 24, 70);

    int brainrot_index = 1;
    int brainrot_index_clear = -1;

    while (true)
    {
        brainrot_index = (brainrot_index + 1) % 3;
        brainrot_index_clear = (brainrot_index_clear + 1) % 3;
        syscallBuddy(18, (uint32_t)brainrot_clear[brainrot_index_clear], 22, 72);
        syscallBuddy(18, (uint32_t)brainrot[brainrot_index], 22, 72);
        delay();
        syscallBuddy(17, (uint32_t)&blank, 23, 69);
        syscallBuddy(17, (uint32_t)&blank, 23, 70);
        syscallBuddy(17, (uint32_t)&blank, 24, 70);

        syscallBuddy(17, (uint32_t)&left_bracket, 24, 70);

        delay();
        syscallBuddy(17, (uint32_t)&blank, 24, 70);

        syscallBuddy(17, (uint32_t)&vertical_strip, 24, 70);

        delay();
        syscallBuddy(17, (uint32_t)&blank, 24, 70);

        syscallBuddy(17, (uint32_t)&left_bracket, 24, 70);

        delay();
        syscallBuddy(17, (uint32_t)&blank, 24, 70);

        syscallBuddy(17, (uint32_t)&under_score, 23, 69);
        syscallBuddy(17, (uint32_t)&under_score, 23, 70);
        syscallBuddy(17, (uint32_t)&left_strip, 24, 70);
    }

    return 0;
}