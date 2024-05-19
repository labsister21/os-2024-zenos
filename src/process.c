#include "header/process/process.h"
#include "header/memory/paging.h"
#include "header/stdlib/string.h"
#include "header/cpu/gdt.h"

static struct ProcessManagerState process_manager_state = {
    .active_process_count = 0,
    .is_used = {false},
    .process_name = {{0}}};

struct ProcessControlBlock *process_get_current_running_pcb_pointer(void)
{
    uint32_t i = 0;
    bool found = false;
    while(i < PROCESS_COUNT_MAX && !found){
        if(process_manager_state.is_used[i] &&
            process_list[i].metadata.state == RUNNING){
                found = true;
            }else{
                i++;
            }
    }
    if(found){
        return &process_list[i];
    }else{
        return NULL;
    }
}

struct ProcessControlBlock process_list[PROCESS_COUNT_MAX];

int32_t process_create_user_process(struct FAT32DriverRequest request)
{
    int32_t retcode = PROCESS_CREATE_SUCCESS;
    if (process_manager_state.active_process_count >= PROCESS_COUNT_MAX)
    {
        retcode = PROCESS_CREATE_FAIL_MAX_PROCESS_EXCEEDED;
        goto exit_cleanup;
    }

    // Ensure entrypoint is not located at kernel's section at higher half
    if ((uint32_t)request.buf >= KERNEL_VIRTUAL_ADDRESS_BASE)
    {
        retcode = PROCESS_CREATE_FAIL_INVALID_ENTRYPOINT;
        goto exit_cleanup;
    }

    // Check whether memory is enough for the executable and additional frame for user stack
    uint32_t page_frame_count_needed = ceil_div(request.buffer_size + PAGE_FRAME_SIZE, PAGE_FRAME_SIZE);
    if (!paging_allocate_check(page_frame_count_needed) || page_frame_count_needed > PROCESS_PAGE_FRAME_COUNT_MAX)
    {
        retcode = PROCESS_CREATE_FAIL_NOT_ENOUGH_MEMORY;
        goto exit_cleanup;
    }

    // Process PCB
    int32_t p_index = process_list_get_inactive_index();
    struct ProcessControlBlock *new_pcb = &(process_list[p_index]);

    process_manager_state.is_used[p_index] = true;
    process_manager_state.active_process_count++;

    new_pcb->metadata.process_id = p_index;

    struct PageDirectory *current_pd = paging_get_current_page_directory_addr();
    struct PageDirectory *new_pd = paging_create_new_page_directory();

    new_pcb->memory.virtual_addr_used[0] = (void *)(paging_allocate_user_page_frame(new_pd, request.buf)) + KERNEL_VIRTUAL_ADDRESS_BASE;
    new_pcb->memory.virtual_addr_used[1] = (void *)(paging_allocate_user_page_frame(new_pd, (void *)0xBFFFFFFC)) + KERNEL_VIRTUAL_ADDRESS_BASE;
    new_pcb->memory.page_frame_used_count = 2;

    // ganti ke virtual address baru
    paging_use_page_directory(new_pd);

    // Membaca dan melakukan load executable dari file system ke memory baru
    uint32_t res_code = read(request);

    if (!!res_code)
    {
        retcode = PROCESS_CREATE_FAIL_FS_READ_FAILURE;
        goto exit_cleanup;
    }

    memcpy(&process_manager_state.process_name[p_index], &request.name, 8);

    new_pcb->metadata.state = READY;

    paging_use_page_directory(current_pd);

    new_pcb->context.cpu.segment.ds = 0x20 | 0x3;
    new_pcb->context.cpu.segment.es = 0x20 | 0x3;
    new_pcb->context.cpu.segment.fs = 0x20 | 0x3;
    new_pcb->context.cpu.segment.gs = 0x20 | 0x3;
    new_pcb->context.eip = (uint32_t)request.buf;

    new_pcb->context.cpu.stack.ebp = 0xBFFFFFFC;
    new_pcb->context.cpu.stack.esp = 0xBFFFFFFC;

    new_pcb->context.page_directory_virtual_addr = new_pd;

    new_pcb->context.cs = 0x18 | 0x3;
    new_pcb->context.esp = 0xBFFFFFFC;
    new_pcb->context.ss = 0x20 | 0x3;

    new_pcb->context.eflags |= CPU_EFLAGS_BASE_FLAG | CPU_EFLAGS_FLAG_INTERRUPT_ENABLE;

    // setup metadata
    new_pcb->metadata.state = READY;

exit_cleanup:
    return retcode;
}

bool process_destroy(uint32_t pid)
{
    for (uint32_t i = 0; i < PROCESS_COUNT_MAX; i++)
    {
        if (process_list[i].metadata.process_id == pid)
        {
            // destroy process
            process_manager_state.active_process_count--;
            process_manager_state.is_used[i] = false;
            memset(&process_list[i], 0, sizeof(struct ProcessControlBlock));
            memset(&process_manager_state.process_name[i], 0, 8);
            return true;
        }
    }
    return false;
}

int32_t process_list_get_inactive_index()
{
    for (uint32_t i = 0; i < PROCESS_COUNT_MAX; i++)
    {
        if (!process_manager_state.is_used[i])
        {
            return i;
        }
    }
    return -1;
}

int32_t process_generate_new_pid()
{
    return process_list_get_inactive_index();
}

uint32_t ceil_div(uint32_t a, uint32_t b)
{
    uint32_t c = !!(a % b);
    return (a / b) + c;
}

void getInformation(char *name, char names[16][256], uint32_t process_ids[16])
{
    uint32_t curr = 0;
    for (uint32_t i = 0; i < 16; i++)
    {
        if (memcmp(name, process_manager_state.process_name[i], 8) == 0 || (name[0] == '\0' && process_manager_state.process_name[i][0] != '\0'))
        {
            // process name is the same !
            memcpy(names[curr], process_manager_state.process_name[i], 8);
            // memcpy((void*)process_ids[curr],&i,4);
            process_ids[curr] = i;
            curr++;
        }
    }
}