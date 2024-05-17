#include "./header/scheduler/scheduler.h"
#include "./header/interrupt/interrupt.h"
#include "./header/process/process.h"
#include "./header/stdlib/string.h"

__attribute__((noreturn)) extern void process_context_switch(struct Context ctx);

void scheduler_init(void)
{
    activate_timer_interrupt();
}

void scheduler_save_context_to_current_running_pcb(struct Context ctx)
{
    struct ProcessControlBlock *current_pcb = process_get_current_running_pcb_pointer();

    memcpy(&(*current_pcb).context.cpu, &ctx.cpu, sizeof(struct CPURegister));
    memcpy(&(*current_pcb).context.eip, &ctx.cpu, sizeof(uint32_t));
    memcpy(&(*current_pcb).context.eflags, &ctx.cpu, sizeof(uint32_t));
}

__attribute__((noreturn)) void scheduler_switch_to_next_process(void)
{
    // implement a scheduling algorithm
}