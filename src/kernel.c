#include <stdint.h>
#include <stdbool.h>
#include "./header/cpu/gdt.h"
#include "./header/kernel-entrypoint.h"
#include "./header/framebuffer.h"
#include "./header/interrupt/idt.h"
#include "./header/interrupt/interrupt.h"
#include "./header/driver/disk.h"
#include "./header/filesystem/fat32.h"
#include "./header/stdlib/string.h"
#include "./header/keyboard.h"
#include "./header/stdlib/string.h"
#include "./header/memory/paging.h"
#include "./header/process/process.h"
#include "./header/scheduler/scheduler.h"

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

// void kernel_setup(void) {
//     load_gdt(&_gdt_gdtr);
//     pic_remap();
//     initialize_idt();
//     framebuffer_clear();
//     framebuffer_set_cursor(0, 0);
//     __asm__("int $0x4");
//     while(true);
// }

// // TESTING FILE SYSTEM
// void kernel_setup(void) {

//     struct FAT32DriverRequest req;
//     char* name = "love1";

//     memcpy(req.name,name,5);
//     req.ext[0]  = 't';
//     req.ext[1]  = 'x';
//     req.ext[2]  = 't';
//     req.parent_cluster_number = 2;
//     req.buffer_size = 23;
//     char test[CLUSTER_SIZE] = "I love All of you !!!!";
//     req.buf = test;

//     struct FAT32DriverRequest req2;
//     char* name2 = "love2";

//     memcpy(req2.name,name2,5);
//     req2.ext[0]  = '\0';
//     req2.ext[1]  = '\0';
//     req2.ext[2]  = '\0';
//     req2.parent_cluster_number = 2;
//     req2.buffer_size = 0;

//     struct FAT32DriverRequest req3;
//     char* name3 = "love3N";

//     memcpy(req3.name,name3,7);
//     req3.ext[0]  = 't';
//     req3.ext[1]  = 'x';
//     req3.ext[2]  = 't';
//     req3.parent_cluster_number = 4;
//     req3.buffer_size = 3607;
//     char test3[CLUSTER_SIZE*2] = "I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3I love All of you againNested3OKKKKKK";
//     req3.buf = test3;

//     initialize_filesystem_fat32();

//     /////////////////////////create
//     // write(req);
//     // write(req2);
//     // write(req3);
//     ///////////////////////////

//     ///////////////////////////////////////////// DELETE
//     // delete(req);
//     //delete(req2);
//     delete(req3);
//         ///////////////////////////////////////////// DELETE

//     while (true);
// }

// Keyboard Test
// void kernel_setup(void) {
//     load_gdt(&_gdt_gdtr);
//     pic_remap();
//     initialize_idt();
//     activate_keyboard_interrupt();
//     framebuffer_clear();
//     framebuffer_set_cursor(0, 0);

//     keyboard_state_activate();
//     char c;
//     while (true) {

//         get_keyboard_buffer(&c);
//         if(c){
//             if(c == '\n'){
//                 framebuffer_set_cursor(framebuffer_get_row()+ 1, 0);
//             }else if(c == '\b'){
//                 framebuffer_set_cursor(framebuffer_get_row(), framebuffer_get_col() - 1);
//                 framebuffer_write(framebuffer_get_row(), framebuffer_get_col(), ' ', 0xF, 0);
//             }else{
//                 framebuffer_write(framebuffer_get_row(), framebuffer_get_col(), c, 0xF, 0);
//                 framebuffer_set_cursor(framebuffer_get_row(), framebuffer_get_col()+ 1);
//             }

//         }
//     }
// }

// Ini buat bab 2
// void kernel_setup(void)
// {
//     load_gdt(&_gdt_gdtr);
//     pic_remap();
//     initialize_idt();
//     activate_keyboard_interrupt();
//     framebuffer_clear();
//     framebuffer_set_cursor(0, 0);
//     initialize_filesystem_fat32();
//     gdt_install_tss();
//     set_tss_register();

//     // Allocate first 4 MiB virtual memory
//     paging_allocate_user_page_frame(&_paging_kernel_page_directory, (uint8_t *)0);

//     // Write shell into memory
//     struct FAT32DriverRequest request = {
//         .buf = (uint8_t *)0,
//         .name = "shell",
//         .ext = "\0\0\0",
//         .parent_cluster_number = ROOT_CLUSTER_NUMBER,
//         .buffer_size = 0x100000,
//     };
//     read(request);

//     // Set TSS $esp pointer and jump into shell
//     set_tss_kernel_current_stack();
//     // Create & execute process 0
//     process_create_user_process(request);
//     paging_use_page_directory(process_list[0].context.page_directory_virtual_addr);
//     kernel_execute_user_program((void *)0x0);
// }

// void kernel_setup(void)
// {
//     load_gdt(&_gdt_gdtr);
//     pic_remap();
//     initialize_idt();
//     activate_keyboard_interrupt();
//     framebuffer_clear();
//     framebuffer_set_cursor(0, 0);
//     initialize_filesystem_fat32();
//     gdt_install_tss();
//     set_tss_register();

//     // Shell request
//     struct FAT32DriverRequest request = {
//         .buf = (uint8_t *)0,
//         .name = "shell",
//         .ext = "\0\0\0",
//         .parent_cluster_number = ROOT_CLUSTER_NUMBER,
//         .buffer_size = 0x100000,
//     };

//     // Set TSS.esp0 for interprivilege interrupt
//     set_tss_kernel_current_stack();

//     // Create & execute process 0
//     process_create_user_process(request);
//     paging_use_page_directory(process_list[0].context.page_directory_virtual_addr);
//     kernel_execute_user_program((void *)0x0);
// }


void kernel_setup(void) {
    load_gdt(&_gdt_gdtr);
    pic_remap();
    initialize_idt();
    activate_keyboard_interrupt();
    framebuffer_clear();
    framebuffer_set_cursor(0, 0);
    initialize_filesystem_fat32();
    gdt_install_tss();
    set_tss_register();

    // Write shell into memory
    struct FAT32DriverRequest request = {
        .buf                   = (uint8_t*) 0,
        .name                  = "shell",
        .ext                   = "\0\0\0",
        .parent_cluster_number = ROOT_CLUSTER_NUMBER,
        .buffer_size           = 0x100000,
    };

    // Set TSS.esp0 for interprivilege interrupt
    set_tss_kernel_current_stack();

    // Create init process and execute it
    process_create_user_process(request);
    scheduler_init();
    scheduler_switch_to_next_process();
}
