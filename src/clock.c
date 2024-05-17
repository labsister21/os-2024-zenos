#include "header/clock.h"
#include "header/interrupt/interrupt.h"

void syscallClock(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx)
{
    __asm__ volatile("mov %0, %%ebx" : /* <Empty> */ : "r"(ebx));
    __asm__ volatile("mov %0, %%ecx" : /* <Empty> */ : "r"(ecx));
    __asm__ volatile("mov %0, %%edx" : /* <Empty> */ : "r"(edx));
    __asm__ volatile("mov %0, %%eax" : /* <Empty> */ : "r"(eax));
    // Note : gcc usually use %eax as intermediate register,
    //        so it need to be the last one to mov
    __asm__ volatile("int $0x30");
}

void get_time_from_cmos(struct Time *curr_time)
{
    syscallClock(15, (uint32_t)curr_time, 0, 0);
}

void int_to_str(uint32_t num, char *str)
{
    if (num < 10)
    {
        str[0] = '0';
        str[1] = '0' + num;
    }
    else
    {
        str[0] = '0' + (num / 10);
        str[1] = '0' + (num % 10);
    }
}

void print_time(char *time_str)
{
    syscallClock(16, (uint32_t)time_str, 0xF, 0);
}

int main(void)
{
    struct Time curr_time;
    char time_str[9], hour_str[2], minute_str[2], second_str[2];

    while (true)
    {
        get_time_from_cmos(&curr_time);

        int_to_str(curr_time.hour, hour_str);
        int_to_str(curr_time.minute, minute_str);
        int_to_str(curr_time.second, second_str);

        time_str[0] = hour_str[0];
        time_str[1] = hour_str[1];
        time_str[2] = ':';
        time_str[3] = minute_str[0];
        time_str[4] = minute_str[1];
        time_str[5] = ':';
        time_str[6] = second_str[0];
        time_str[7] = second_str[1];
        time_str[8] = '\0';

        print_time(time_str);
    }
    return 0;
}