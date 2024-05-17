#include "./header/cmos.h"

int get_update_in_progress_flag()
{
    out(CMOS_ADDRESS, 0x0A);
    return (in(CMOS_DATA) & 0x80);
}

unsigned char get_RTC_register(int reg)
{
    out(CMOS_ADDRESS, reg);
    return in(CMOS_DATA);
}

void read_rtc(struct Time *current_time)
{
    uint32_t last_second;
    uint32_t last_minute;
    uint32_t last_hour;
    uint32_t registerB;

    while (get_update_in_progress_flag())
        ;
    uint32_t second = get_RTC_register(0x00);
    uint32_t minute = get_RTC_register(0x02);
    uint32_t hour = get_RTC_register(0x04);

    do
    {
        last_second = second;
        last_minute = minute;
        last_hour = hour;

        while (get_update_in_progress_flag())
            ;
        second = get_RTC_register(0x00);
        minute = get_RTC_register(0x02);
        hour = get_RTC_register(0x04);

    } while ((last_second != second) || (last_minute != minute) || (last_hour != hour));

    registerB = get_RTC_register(0x0B);

    // Convert BCD to binary values

    if (!(registerB & 0x04))
    {
        second = (second & 0x0F) + ((second / 16) * 10);
        minute = (minute & 0x0F) + ((minute / 16) * 10);
        hour = (((hour & 0x0F) + (((hour & 0x70) / 16) * 10) + 7) % 24) | (hour & 0x80);
    }

    // Convert 12 hour clock to 24 hour clock and add 7 to synchronize with WIB
    if (!(registerB & 0x02) && (hour & 0x80))
    {
        hour = ((hour & 0x7F) + 12) % 24;
    }

    // Calculate the full (4-digit) year
    current_time->hour = hour;

    current_time->second = second;
}