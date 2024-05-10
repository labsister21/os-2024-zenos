#include "../header/cpu/gdt.h"
#include "../header/interrupt/interrupt.h"

/**
 * global_descriptor_table, predefined GDT.
 * Initial SegmentDescriptor already set properly according to Intel Manual & OSDev.
 * Table entry : [{Null Descriptor}, {Kernel Code}, {Kernel Data (variable, etc)}, ...].
 */
struct GlobalDescriptorTable global_descriptor_table = {
    .table = {
        {0},
        {
            .segment_high = 0xF, 
            .segment_low = 0xFFFF,
            .base_high = 0, 
            .base_mid = 0, 
            .base_low = 0, 
            .non_system = 1, 
            .type_bit = 0xA, 
            .descriptor_priv_level = 0, 
            .segment_present = 1, 
            .default_op_size = 1, 
            .bit64_code_segment = 0, 
            .granularity = 1, 
            // .avail = 1, 
        },
        {
            .segment_low = 0xFFFF,
            .base_low = 0, 
            .base_mid = 0, 
            .type_bit = 0x2, 
            .non_system = 1, 
            .descriptor_priv_level = 0, 
            .segment_present = 1, 
            .segment_high = 0xF, 
            // .avail = 0, 
            .bit64_code_segment = 0, 
            .default_op_size = 1, 
            .granularity = 1, 
            .base_high = 0,
        },
        {
            .segment_low = 0xFFFF,
            .base_low = 0, 
            .base_mid = 0, 
            .type_bit = 0xA, 
            .non_system = 1, 
            .descriptor_priv_level = 3, 
            .segment_present = 1, 
            .segment_high = 0xF, 
            // .avail = 0, 
            .bit64_code_segment = 0, 
            .default_op_size = 1, 
            .granularity = 1, 
            .base_high = 0,
        },
        {
            .segment_low = 0xFFFF,
            .base_low = 0, 
            .base_mid = 0, 
            .type_bit = 0x2, 
            .non_system = 1, 
            .descriptor_priv_level = 3, 
            .segment_present = 1, 
            .segment_high = 0xF, 
            // .avail = 0, 
            .bit64_code_segment = 0, 
            .default_op_size = 1, 
            .granularity = 1, 
            .base_high = 0,
        },
        {
            .segment_low = sizeof(struct TSSEntry),
            .base_low = 0, 
            .base_mid = 0, 
            .type_bit = 0x9, 
            .non_system = 0, 
            .descriptor_priv_level = 0, 
            .segment_present = 1, 
            .segment_high = (sizeof(struct TSSEntry) & (0xF << 16)) >> 16, 
            // .avail = 0, 
            .bit64_code_segment = 0, 
            .default_op_size = 1, 
            .granularity = 0, 
            .base_high = 0,
        },
        {0}
    }
};

/**
 * _gdt_gdtr, predefined system GDTR.
 * GDT pointed by this variable is already set to point global_descriptor_table above.
 * From: https://wiki.osdev.org/Global_Descriptor_Table, GDTR.size is GDT size minus 1.
 */
struct GDTR _gdt_gdtr = {
    .size = sizeof(global_descriptor_table) - 1,
    .address = &global_descriptor_table
};

void gdt_install_tss(void) {
    uint32_t base = (uint32_t) &_interrupt_tss_entry;
    global_descriptor_table.table[5].base_high = (base & (0xFF << 24)) >> 24;
    global_descriptor_table.table[5].base_mid  = (base & (0xFF << 16)) >> 16;
    global_descriptor_table.table[5].base_low  = base & 0xFFFF;
}
