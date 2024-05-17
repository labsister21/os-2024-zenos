#include "./header/scheduler/scheduler.h"
#include "./header/interrupt/interrupt.h"

__attribute__((noreturn)) extern void process_context_switch(struct Context ctx);

void scheduler_init(void)
{
    activate_timer_interrupt();
}

void scheduler_save_context_to_current_running_pcb(struct Context ctx);

__attribute__((noreturn)) void scheduler_switch_to_next_process(void);