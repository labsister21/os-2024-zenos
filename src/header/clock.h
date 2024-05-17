#ifndef _CLOCK_H
#define _CLOCK_H

#include "cmos.h"

void syscallClock(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx);

void get_time_from_cmos(struct Time *curr_time);

void int_to_str(uint32_t num, char *str);

void print_time(char *time_str);

#endif