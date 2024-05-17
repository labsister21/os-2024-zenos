#include "header/clock_buddy.h"

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

int main(void)
{
    char circle = 'o';
    char vertical_strip = '|';
    char left_strip = '/';
    char right_strip = '\\';
    char under_score = '_';
    char left_bracket = '(';
    char right_bracket = ')';

    syscallBuddy(16, (uint32_t)&circle, 0xF, 0);

    return 0;
}