#include <stdint.h>
#include <stdbool.h> 
#include "header/cpu/gdt.h"
#include "header/kernel-entrypoint.h"
#include "header/framebuffer.h"
#include "interrupt/idt.h"
#include "interrupt/interrupt.h"
#include "./header/driver/disk.h"

// void kernel_setup(void) {
//     uint32_t a;
//     uint32_t volatile b = 0x0000BABE;
//     __asm__("mov $0xCAFE0000, %0" : "=r"(a));
//     while (true) b += 1;
// }

// void kernel_setup(void) {
//     load_gdt(&_gdt_gdtr);
//     while (true);
// }

// void kernel_setup(void) {
//     framebuffer_clear();
//     framebuffer_write(3, 8,  'H', 0, 0xF);
//     framebuffer_write(3, 9,  'a', 0, 0xF);
//     framebuffer_write(3, 10, 'i', 0, 0xF);
//     framebuffer_write(3, 11, '!', 0, 0xF);
//     framebuffer_set_cursor(3, 10);
//     while (true);
// }

void kernel_setup(void) {
    // load_gdt(&_gdt_gdtr);
    // pic_remap();
    // initialize_idt();
    // framebuffer_clear();
    // framebuffer_set_cursor(0, 0);
    // __asm__("int $0x4");
    // while(true);
    load_gdt(&_gdt_gdtr);
    pic_remap();
    // // activate_keyboard_interrupt();
    initialize_idt();
    framebuffer_clear();
    framebuffer_set_cursor(0, 0);


    struct BlockBuffer b;

    read_blocks(&b, 17, 1);
    for (int i = 0 ; i < 128; i++) b.buf[i] = 1;
    write_blocks(&b,17,1);

    while (true);
}

