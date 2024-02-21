#include "../include/linux/kernel.h"

void kfread();

void* syscall_table[] = {printk, kfread};

void kfread() {
    printk("syscall!!!!\n");
}