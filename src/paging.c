#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "header/memory/paging.h"

__attribute__((aligned(0x1000))) struct PageDirectory _paging_kernel_page_directory = {
    .table = {
        [0] = {
            .flag.present_bit = 1,
            .flag.write_bit = 1,
            .flag.use_pagesize_4_mb = 1,
            .lower_address = 0,
        },
        [0x300] = {
            .flag.present_bit = 1,
            .flag.write_bit = 1,
            .flag.use_pagesize_4_mb = 1,
            .lower_address = 0,
        },
    }};

static struct PageManagerState page_manager_state = {
    .page_frame_map = {[0 ... PAGE_FRAME_MAX_COUNT - 1] = FALSE},
    // TODO: Fill in if needed ...
};

void update_page_directory_entry(
    struct PageDirectory *page_dir,
    void *physical_addr,
    void *virtual_addr,
    struct PageDirectoryEntryFlag flag)
{
    uint32_t page_index = ((uint32_t)virtual_addr >> 22) & 0x3FF;
    page_dir->table[page_index].flag = flag;
    page_dir->table[page_index].lower_address = ((uint32_t)physical_addr >> 22) & 0x3FF;
    flush_single_tlb(virtual_addr);
}

void flush_single_tlb(void *virtual_addr)
{
    asm volatile("invlpg (%0)" : /* <Empty> */ : "b"(virtual_addr) : "memory");
}

/* --- Memory Management --- */
// TODO: Implement

bool paging_allocate_check(uint32_t amount)
{
    uint32_t required_frames = (amount + PAGE_FRAME_SIZE - 1) / PAGE_FRAME_SIZE;
    uint32_t free_frames = 0;
    for (uint32_t i = 0; i < PAGE_FRAME_MAX_COUNT; i++)
    {
        if (!page_manager_state.page_frame_map[i])
        {
            free_frames++;
            if (free_frames >= required_frames)
            {
                return true;
            }
        }
    }
    return false;
}

bool paging_allocate_user_page_frame(struct PageDirectory *page_dir, void *virtual_addr)
{
    /*
     * TODO: Find free physical frame and map virtual frame into it
     * - Find free physical frame in page_manager_state.page_frame_map[] using any strategies
     * - Mark page_manager_state.page_frame_map[]
     * - Update page directory with user flags:
     *     > present bit    true
     *     > write bit      true
     *     > user bit       true
     *     > pagesize 4 mb  true
     */

    uint32_t frame_index;
    for (frame_index = 0; frame_index < PAGE_FRAME_MAX_COUNT; frame_index++)
    {
        if (!page_manager_state.page_frame_map[frame_index])
        {
            page_manager_state.page_frame_map[frame_index] = true;
            break;
        }
    }
    if (frame_index == PAGE_FRAME_MAX_COUNT)
        return false;

    uint32_t page_index = ((uint32_t)virtual_addr >> 22) & 0x3FF;
    page_dir->table[page_index].flag.present_bit = 1;
    page_dir->table[page_index].flag.write_bit = 1;
    page_dir->table[page_index].flag.user_bit = 1;
    page_dir->table[page_index].flag.use_pagesize_4_mb = 1;
    page_dir->table[page_index].lower_address = frame_index;
    return true;
}

bool paging_free_user_page_frame(struct PageDirectory *page_dir, void *virtual_addr)
{
    /*
     * TODO: Deallocate a physical frame from respective virtual address
     * - Use the page_dir.table values to check mapped physical frame
     * - Remove the entry by setting it into 0
     */

    uint32_t page_dir_index = ((uint32_t)virtual_addr >> 22) & 0x3FF;
    if (page_dir->table[page_dir_index].flag.present_bit)
    {
        uint32_t frame_index = page_dir->table[page_dir_index].lower_address;
        page_manager_state.page_frame_map[frame_index] = false;
        page_dir->table[page_dir_index].flag = (struct PageDirectoryEntryFlag){0};
        page_dir->table[page_dir_index].lower_address = 0;
        flush_single_tlb(virtual_addr);
        return true;
    }
    return false;
}
