#ifndef _CLOCK_BUDDY_H
#define _CLOCK_BUDDY_H

void syscallBuddy(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx);

void print_time(char *time_str);

#endif