#ifndef _PROCESS_H
#define _PROCESS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "../interrupt/interrupt.h"
#include "../memory/paging.h"
#include "../filesystem/fat32.h"

#define PROCESS_NAME_LENGTH_MAX 32
#define PROCESS_PAGE_FRAME_COUNT_MAX 8
#define PROCESS_COUNT_MAX 16

#define KERNEL_RESERVED_PAGE_FRAME_COUNT 4
#define KERNEL_VIRTUAL_ADDRESS_BASE 0xC0000000

#define CPU_EFLAGS_BASE_FLAG 0x2
#define CPU_EFLAGS_FLAG_CARRY 0x1
#define CPU_EFLAGS_FLAG_PARITY 0x4
#define CPU_EFLAGS_FLAG_AUX_CARRY 0x10
#define CPU_EFLAGS_FLAG_ZERO 0x40
#define CPU_EFLAGS_FLAG_SIGN 0x80
#define CPU_EFLAGS_FLAG_TRAP 0x100
#define CPU_EFLAGS_FLAG_INTERRUPT_ENABLE 0x200
#define CPU_EFLAGS_FLAG_DIRECTION 0x400
#define CPU_EFLAGS_FLAG_OVERFLOW 0x800
#define CPU_EFLAGS_FLAG_IO_PRIVILEGE 0x3000
#define CPU_EFLAGS_FLAG_NESTED_TASK 0x4000
#define CPU_EFLAGS_FLAG_MODE 0x8000
#define CPU_EFLAGS_FLAG_RESUME 0x10000
#define CPU_EFLAGS_FLAG_VIRTUAL_8086 0x20000
#define CPU_EFLAGS_FLAG_ALIGNMENT_CHECK 0x40000
#define CPU_EFLAGS_FLAG_VINTERRUPT_FLAG 0x80000
#define CPU_EFLAGS_FLAG_VINTERRUPT_PENDING 0x100000
#define CPU_EFLAGS_FLAG_CPUID_INSTRUCTION 0x200000
#define CPU_EFLAGS_FLAG_AES_SCHEDULE_LOAD 0x40000000
#define CPU_EFLAGS_FLAG_ALTER_INSTRUCTION 0x80000000

// Return code constant for process_create_user_process()
#define PROCESS_CREATE_SUCCESS 0
#define PROCESS_CREATE_FAIL_MAX_PROCESS_EXCEEDED 1
#define PROCESS_CREATE_FAIL_INVALID_ENTRYPOINT 2
#define PROCESS_CREATE_FAIL_NOT_ENOUGH_MEMORY 3
#define PROCESS_CREATE_FAIL_FS_READ_FAILURE 4

struct ProcessManagerState
{
    int active_process_count;
    int is_used[16];
};

/**
 * Get currently running process PCB pointer
 *
 * @return Will return NULL if there's no running process
 */
struct ProcessControlBlock *process_get_current_running_pcb_pointer(void);

/**
 * Create new user process and setup the virtual address space.
 * All available return code is defined with macro "PROCESS_CREATE_*"
 *
 * @note          This procedure assumes no reentrancy in ISR
 * @param request Appropriate read request for the executable
 * @return        Process creation return code
 */
int32_t process_create_user_process(struct FAT32DriverRequest request);

/**
 * Destroy process then release page directory and process control block
 *
 * @param pid Process ID to delete
 * @return    True if process destruction success
 */
bool process_destroy(uint32_t pid);

/* --- Process-related Memory Management --- */
#define PAGING_DIRECTORY_TABLE_MAX_COUNT 32

/**
 * Create new page directory prefilled with 1 page directory entry for kernel higher half mapping
 *
 * @return Pointer to page directory virtual address. Return NULL if allocation failed
 */
struct PageDirectory *paging_create_new_page_directory(void);

/**
 * Free page directory and delete all page directory entry
 *
 * @param page_dir Pointer to page directory virtual address
 * @return         True if free operation success
 */
bool paging_free_page_directory(struct PageDirectory *page_dir);

/**
 * Get currently active page directory virtual address from CR3 register
 *
 * @note   Assuming page directories lives in kernel memory
 * @return Page directory virtual address currently active (CR3)
 */
struct PageDirectory *paging_get_current_page_directory_addr(void);

/**
 * Change active page directory (indirectly trigger TLB flush for all non-global entry)
 *
 * @note                        Assuming page directories lives in kernel memory
 * @param page_dir_virtual_addr Page directory virtual address to switch into
 */
void paging_use_page_directory(struct PageDirectory *page_dir_virtual_addr);

// struct CPURegisterProcess
// {
//     struct
//     {
//         uint32_t edi;
//         uint32_t esi;
//     } __attribute__((packed)) index;
//     struct
//     {
//         uint32_t ebp;
//         uint32_t esp;
//     } __attribute__((packed)) stack;
//     struct
//     {
//         uint32_t ebx;
//         uint32_t edx;
//         uint32_t ecx;
//         uint32_t eax;
//     } __attribute__((packed)) general;
//     struct
//     {
//         uint32_t ss;
//         uint32_t gs;
//         uint32_t fs;
//         uint32_t es;
//         uint32_t ds;
//         uint32_t cs;
//     } __attribute__((packed)) segment;
// } __attribute__((packed));

/**
 * Contain information needed for task to be able to get interrupted and resumed later
 *
 * @param cpu                         All CPU register state
 * @param eip                         CPU instruction counter to resume execution
 * @param eflags                      Flag register to load before resuming the execution
 * @param page_directory_virtual_addr CPU register CR3, containing pointer to active page directory
 */
struct Context
{
    // TODO: Add important field here
    struct CPURegister cpu;
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
    uint32_t esp;
    uint32_t ss;
    struct PageDirectory *page_directory_virtual_addr;
};

typedef enum PROCESS_STATE
{
    // TODO: Add process states
    NOT_READY,
    READY,   // Process is ready to run and waiting in the ready queue
    RUNNING, // Process is currently being executed by the CPU
    BLOCKED
} PROCESS_STATE;

/**
 * Structure data containing information about a process
 *
 * @param metadata Process metadata, contain various information about process
 * @param context  Process context used for context saving & switching
 * @param memory   Memory used for the process
 */
struct ProcessControlBlock
{
    struct
    {
        uint32_t process_id;
        enum PROCESS_STATE state;

    } metadata;

    struct Context context;

    struct
    {
        void *virtual_addr_used[PROCESS_PAGE_FRAME_COUNT_MAX];
        uint32_t page_frame_used_count;
    } memory;
};

int32_t process_list_get_inactive_index();

uint32_t ceil_div(uint32_t a, uint32_t b);
int32_t process_generate_new_pid();

extern struct ProcessControlBlock process_list[PROCESS_COUNT_MAX];

#endif