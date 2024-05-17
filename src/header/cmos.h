#ifndef _CMOS_H
#define _CMOS_H

#include "cpu/portio.h"

#define CURRENT_YEAR 2023

#define CMOS_ADDRESS 0x70
#define CMOS_DATA 0x71

struct Time
{
    uint32_t second;
    uint32_t minute;
    uint32_t hour;
};

// flag to check whether an update is in progress
int get_update_in_progress_flag();

// read the registers to get values from RTC
unsigned char get_RTC_register(int reg);

void read_rtc(struct Time *);

#endif