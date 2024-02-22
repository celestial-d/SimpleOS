#include "../include/linux/tty.h"
#include "../include/linux/kernel.h"
#include "../include/linux/traps.h"
#include "../include/linux/mm.h"
#include "../include/linux/task.h"
#include "../include/linux/sched.h"

extern void clock_init();
extern void init_tss_item(int gdt_index, int base, int limit);

extern tss_t tss;

void user_mode() {
    __asm__("int 0x80;");

    int age = 10;

    while (true);
}

void kernel_main(void) {
    console_init();
    gdt_init();
    idt_init();
    clock_init();

    print_check_memory_info();
    memory_init();
    memory_map_int();

    init_tss_item(6, &tss, sizeof(tss_t) - 1);

    task_init();

    __asm__("sti;");

    while (true) ;
}