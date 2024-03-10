#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "header/framebuffer.h"
#include "header/stdlib/string.h"
#include "header/cpu/portio.h"

void enable_cursor(uint8_t cursor_start, uint8_t cursor_end){
	out(CURSOR_PORT_CMD, 0x0A);
	out(CURSOR_PORT_DATA, (in(CURSOR_PORT_DATA) & 0xC0) | cursor_start);
 
	out(CURSOR_PORT_CMD, 0x0B);
	out(CURSOR_PORT_DATA, (in(CURSOR_PORT_DATA) & 0xE0) | cursor_end);
}

void disable_cursor(){
	out(CURSOR_PORT_CMD, 0x0A);
	out(CURSOR_PORT_DATA, 0x20);
}

void framebuffer_set_cursor(uint8_t r, uint8_t c) {
    // TODO : Implement
    disable_cursor();
    enable_cursor(14, 15);
    uint16_t pos = 80 * r + c;
    out(CURSOR_PORT_CMD, 0x0F);
	out(CURSOR_PORT_DATA, (uint8_t) (pos & 0xFF));
	out(CURSOR_PORT_CMD, 0x0E);
	out(CURSOR_PORT_DATA, (uint8_t) ((pos >> 8) & 0xFF));
}

void framebuffer_write(uint8_t row, uint8_t col, char c, uint8_t fg, uint8_t bg) {
    // TODO : Implement
    uint16_t attrib = (bg << 4) | (fg & 0x0F);
    volatile uint16_t* framebuffer = (volatile uint16_t*)FRAMEBUFFER_MEMORY_OFFSET;
    framebuffer[row * 80 + col] = c | (attrib << 8);
}

void framebuffer_clear(void) {
    // TODO : Implement
    volatile uint16_t *vga = (volatile uint16_t *)FRAMEBUFFER_MEMORY_OFFSET;
    uint16_t val = 0x0720;
    for (int row = 0; row < 25; row++) {
        for (int col = 0; col < 80; col++) {
            const int position = row * 80 + col;
            vga[position] = val;
        }
    }
}