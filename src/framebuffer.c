#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "header/framebuffer.h"
#include "header/stdlib/string.h"
#include "header/cpu/portio.h"

void enable_cursor(uint8_t cursor_start, uint8_t cursor_end)
{
    out(CURSOR_PORT_CMD, 0x0A);
    out(CURSOR_PORT_DATA, (in(CURSOR_PORT_DATA) & 0xC0) | cursor_start);

    out(CURSOR_PORT_CMD, 0x0B);
    out(CURSOR_PORT_DATA, (in(CURSOR_PORT_DATA) & 0xE0) | cursor_end);
}

void disable_cursor()
{
    out(CURSOR_PORT_CMD, 0x0A);
    out(CURSOR_PORT_DATA, 0x20);
}

void framebuffer_set_cursor(uint8_t r, uint8_t c)
{
    // TODO : Implement
    disable_cursor();
    enable_cursor(14, 15);
    uint16_t pos = 80 * r + c;
    out(CURSOR_PORT_CMD, 0x0F);
    out(CURSOR_PORT_DATA, (uint8_t)(pos & 0xFF));
    out(CURSOR_PORT_CMD, 0x0E);
    out(CURSOR_PORT_DATA, (uint8_t)((pos >> 8) & 0xFF));
}

void framebuffer_write(uint8_t row, uint8_t col, char c, uint8_t fg, uint8_t bg)
{
    // TODO : Implement
    uint16_t attrib = (bg << 4) | (fg & 0x0F);
    volatile uint16_t *framebuffer = (volatile uint16_t *)FRAMEBUFFER_MEMORY_OFFSET;
    framebuffer[row * 80 + col] = c | (attrib << 8);
}

void framebuffer_clear(void)
{
    // TODO : Implement
    volatile uint16_t *vga = (volatile uint16_t *)FRAMEBUFFER_MEMORY_OFFSET;
    uint16_t val = 0x0720;
    for (int row = 0; row < 25; row++)
    {
        for (int col = 0; col < 80; col++)
        {
            const int position = row * 80 + col;
            vga[position] = val;
        }
    }
}

uint16_t framebuffer_get_cursor()
{
    uint16_t position = 0;
    out(CURSOR_PORT_CMD, LowerByte);
    position |= in(CURSOR_PORT_DATA);
    out(CURSOR_PORT_CMD, UpperByte);
    position |= ((uint16_t)in(CURSOR_PORT_DATA)) << 8;
    return position;
}

uint8_t framebuffer_get_row()
{
    return framebuffer_get_cursor() / 80;
}

uint8_t framebuffer_get_col()
{
    return framebuffer_get_cursor() % 80;
}

void putchar(char c, uint32_t color)
{
    if (c == '\n')
    {
        framebuffer_set_cursor(framebuffer_get_row() + 1, 0);
    }
    else if (c == '\b' && framebuffer_get_col() == 0)
    {
        framebuffer_set_cursor(framebuffer_get_row() - 1, 79);
        framebuffer_write(framebuffer_get_row(), framebuffer_get_col(), ' ', color, 0);
    }
    else if (c == '\b')
    {
        framebuffer_set_cursor(framebuffer_get_row(), framebuffer_get_col() - 1);
        framebuffer_write(framebuffer_get_row(), framebuffer_get_col(), ' ', color, 0);
    }
    else
    {
        framebuffer_write(framebuffer_get_row(), framebuffer_get_col(), c, color, 0);
        framebuffer_set_cursor(framebuffer_get_row(), framebuffer_get_col() + 1);
    }
}

void puts(const char *str, uint32_t color)
{
    uint8_t i = 0;
    char c = str[i];

    while (c != '\0')
    {
        if (c == '\n')
        {
            framebuffer_set_cursor(framebuffer_get_row() + 1, 0);
            i++;
            c = str[i];
            continue;
        }
        framebuffer_write(framebuffer_get_row(), framebuffer_get_col(), c, color, 0);
        framebuffer_set_cursor(framebuffer_get_row(), framebuffer_get_col() + 1);
        i++;
        c = str[i];
    }
}

void put_time(const char *str, uint32_t color)
{
    uint8_t i = 0;
    char c = str[i];

    while (c != '\0')
    {
        framebuffer_write(25, 71 + i, c, color, 0);
        i++;
        c = str[i];
    }
}