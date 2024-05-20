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

void kernel_setup(void)
{
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
        .buf = (uint8_t *)0,
        .name = "shell",
        .ext = "exe",
        .parent_cluster_number = ROOT_CLUSTER_NUMBER,
        .buffer_size = 0x100000,
    };

    // struct FAT32DriverRequest request2 = {
    //     .buf = (uint8_t *)0,
    //     .name = "clock",
    //     .ext = "\0\0\0",
    //     .parent_cluster_number = ROOT_CLUSTER_NUMBER,
    //     .buffer_size = 0x100000,
    // };

    // Set TSS.esp0 for interprivilege interrupt
    set_tss_kernel_current_stack();

    // Create init process and execute it
    process_create_user_process(request);
    scheduler_init();
    scheduler_switch_to_next_process();
}
