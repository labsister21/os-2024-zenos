#include "gdt.h"

/**
 * global_descriptor_table, predefined GDT.
 * Initial SegmentDescriptor already set properly according to Intel Manual & OSDev.
 * Table entry : [{Null Descriptor}, {Kernel Code}, {Kernel Data (variable, etc)}, ...].
 */
struct GlobalDescriptorTable global_descriptor_table = {
    .table = {
        {// TODO : Implement
            // Null
            .segment_low = 0,
            .base_low = 0, 
            .base_mid = 0, 
            .type_bit = 0, 
            .non_system = 0, 
            .descriptor_priv_level = 0, 
            .segment_present = 0, 
            .segment_high = 0, 
            .avail = 0, 
            .bit64_code_segment = 0, 
            .default_op_size = 0, 
            .granularity = 0, 
            .base_high = 0, 
        },
        {
            // TODO : Implement
            // Kernel code segment
            .segment_low = 0xFFFFF,
            .base_low = 0, 
            .base_mid = 0, 
            .type_bit = 0xA, 
            .non_system = 1, 
            .descriptor_priv_level = 0, 
            .segment_present = 1, 
            .segment_high = 0, 
            .avail = 0, 
            .bit64_code_segment = 0, 
            .default_op_size = 1, 
            .granularity = 1, 
            .base_high = 0,
        },
        {
            // TODO : Implement
            // Kernel data segment
            .segment_low = 0xFFFFF,
            .base_low = 0, 
            .base_mid = 0, 
            .type_bit = 0x2, 
            .non_system = 1, 
            .descriptor_priv_level = 0, 
            .segment_present = 1, 
            .segment_high = 0, 
            .avail = 0, 
            .bit64_code_segment = 0, 
            .default_op_size = 1, 
            .granularity = 1, 
            .base_high = 0,
        }}};

/**
 * _gdt_gdtr, predefined system GDTR.
 * GDT pointed by this variable is already set to point global_descriptor_table above.
 * From: https://wiki.osdev.org/Global_Descriptor_Table, GDTR.size is GDT size minus 1.
 */
struct GDTR _gdt_gdtr = {
    // TODO : Implement, this GDTR will point to global_descriptor_table.
    //        Use sizeof operator
};
