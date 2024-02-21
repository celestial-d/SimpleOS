#include "../include/linux/kernel.h"
#include "../include/linux/task.h"

extern void init_tss_item(int gdt_index, int base, int limit);
extern void move_to_user();

extern tss_t tss;

void* kernel_thread(void* arg) {
    init_tss_item(6, (int) &tss, sizeof(tss_t) - 1);

    move_to_user();

    while (true) {
        asm volatile("sti; hlt;");
    }
}