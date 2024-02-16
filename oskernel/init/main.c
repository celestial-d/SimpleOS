#include "../include/linux/tty.h"
#include "../include/linux/kernel.h"
#include "../include/linux/traps.h"

void kernel_main(void) {
    console_init();
    gdt_init();
    idt_init();
    printk("os is cool\n");

    __asm__("sti;");
    while (true);
}