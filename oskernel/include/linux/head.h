#ifndef OS_KERNEL_HEAD_H
#define OS_KERNEL_HEAD_H

typedef struct gdt_item_t {
    unsigned short limit_low;      //  0 ~ 15
    unsigned int base_low : 24;    //  0 ~ 23
    unsigned char type : 4;        // segment type
    unsigned char segment : 1;
    unsigned char DPL : 2;         // Descriptor Privilege Level  0 ~ 3
    unsigned char present : 1;     // 1: in memory，0: in disk
    unsigned char limit_high : 4;
    unsigned char available : 1;
    unsigned char long_mode : 1;   // extent bit
    unsigned char big : 1;         // 32/16;
    unsigned char granularity : 1;
    unsigned char base_high;
} __attribute__((packed)) gdt_item_t;

typedef struct interrupt_gate_t {
    short offset0;    //  0 ~ 15
    short selector;   //
    char reserved;    //
    char type : 4;    //
    char segment : 1; //
    char DPL : 2;     // permission
    char present : 1; // valid or not
    short offset1;    //  16 ~ 31 位
} __attribute__((packed)) interrupt_gate_t;

typedef struct gdt_selector_t {
    char RPL : 2;
    char TI : 1;
    short index : 13;
} __attribute__((packed)) gdt_selector_t;

#pragma pack(2)
typedef struct xdt_ptr_t {
    short   limit;
    int     base;
} xdt_ptr_t;
#pragma pack()

#endif