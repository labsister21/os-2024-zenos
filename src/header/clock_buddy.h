#ifndef _CLOCK_BUDDY_H
#define _CLOCK_BUDDY_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

void syscallBuddy(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx);

void delay();

void print_time(char *time_str);

#endif