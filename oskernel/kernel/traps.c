#include "../include/asm/io.h"
#include "../include/linux/traps.h"

#define PIC_M_CTRL  0x20    // main chip port
#define PIC_M_DATA  0x21    // main chip port
#define PIC_S_CTRL  0xa0    // other chip port
#define PIC_S_DATA  0xa1    // other chip port
#define PIC_EOI     0x20    // inform when it finishes

void send_eoi(int idt_index) {
    if (idt_index >= 0x20 && idt_index < 0x28) {
        out_byte(PIC_M_CTRL, PIC_EOI);
    } else if (idt_index >= 0x28 && idt_index < 0x30) {
        out_byte(PIC_M_CTRL, PIC_EOI);
        out_byte(PIC_S_CTRL, PIC_EOI);
    }
}

void write_xdt_ptr(xdt_ptr_t* p, short limit, int base) {
    p->limit = limit;
    p->base = base;
}