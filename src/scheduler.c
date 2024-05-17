#include "./header/scheduler/scheduler.h"
#include "./header/interrupt/interrupt.h"
#include "./header/process/process.h"
#include "./header/stdlib/string.h"
#include "./header/memory/paging.h"

// static uint32_t next_process = 0;
__attribute__((noreturn)) extern void process_context_switch(struct Context ctx);

void scheduler_init(void)
{
    activate_timer_interrupt();
    // paging_use_page_directory(process_list[0].context.page_directory_virtual_addr);
    // process_list[0].metadata.state = RUNNING;
}

void scheduler_save_context_to_current_running_pcb(struct Context ctx)
{
    struct ProcessControlBlock *current_pcb = process_get_current_running_pcb_pointer();

    memcpy(&(*current_pcb).context, &ctx, sizeof(struct Context));
}

__attribute__((noreturn)) void scheduler_switch_to_next_process(void)
{
    // // // implement a scheduling algorithm
    // int i;
    // for (i = 0; i < PROCESS_COUNT_MAX; i++)
    // {
    //     if (process_list[i].metadata.state == RUNNING)
    //     {
    //         process_list[i].metadata.state = READY;
    //         break;
    //     }
    // }

    // if (i == PROCESS_COUNT_MAX)
    // {
    //     // something went wrong here !
    // }
    // i = (i + 1) % 16;
    // // set back to
    // // trying to find the next process to run
    // bool found = false;
    // while (!found)
    // {
    //     if (process_list[i].metadata.state == READY)
    //     {
    //         found = true;
    //     }
    //     else
    //     {
    //         i = (i + 1) % 16;
    //     }
    // }
    process_list[0].metadata.state = RUNNING;

    // prepare the next process
    paging_use_page_directory(process_list[0].context.page_directory_virtual_addr);
    pic_ack(IRQ_TIMER + PIC1_OFFSET);
    process_context_switch(process_list[0].context);
}