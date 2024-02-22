#ifndef OS_KERNEL_MM_H
#define OS_KERNEL_MM_H

#include "types.h"

#define PAGE_SIZE 4096

typedef struct {
    unsigned int  base_addr_low;
    unsigned int  base_addr_high;
    unsigned int  length_low;
    unsigned int  length_high;
    unsigned int  type;
}check_memmory_item_t;

typedef struct {
    unsigned short          times;
    check_memmory_item_t*   data;
}check_memory_info_t;

typedef struct {
    uint    addr_start;
    uint    addr_end;
    uint    valid_mem_size;
    uint    pages_total;
    uint    pages_free;
    uint    pages_used;
}physics_memory_info_t;

typedef struct {
    uint            addr_base;
    uint            pages_total;
    uint            bitmap_item_used;
    uchar*          map;
}physics_memory_map_t;

void print_check_memory_info();

void memory_init();
void memory_map_int();

void* virtual_memory_init();

void* get_free_page();
void free_page(void* p);

void* kmalloc(size_t size);
void kfree_s(void *obj, int size);
#endif