#include "../include/asm/system.h"
#include "../include/linux/mm.h"
#include "../include/linux/kernel.h"
#include "../include/string.h"

#define ARDS_ADDR   0x1100


#define VALID_MEMORY_FROM           0x100000

#define ZONE_VALID 1        // ards can be used
#define ZONE_RESERVED 2     // ards cannot use

physics_memory_info_t g_physics_memory;
physics_memory_map_t g_physics_memory_map;

void memory_init() {
    check_memory_info_t* p = (check_memory_info_t*)ARDS_ADDR;
    check_memmory_item_t* p_data = (check_memmory_item_t*)(ARDS_ADDR + 2);

    for (int i = 0; i < p->times; ++i) {
        check_memmory_item_t* tmp = p_data + i;

        if (tmp->base_addr_low > 0 && tmp->type == ZONE_VALID) {
            g_physics_memory.addr_start = tmp->base_addr_low;
            g_physics_memory.valid_mem_size = tmp->length_low;
            g_physics_memory.addr_end = tmp->base_addr_low + tmp->length_low;
        }
    }

    // if no memory to use
    if (VALID_MEMORY_FROM != g_physics_memory.addr_start) {
        printk("[%s:%d] no valid physics memory\n", __FILE__, __LINE__);
        return;
    }

    g_physics_memory.pages_total = (g_physics_memory.addr_end - g_physics_memory.addr_start) >> 12;
    g_physics_memory.pages_used = 0;
    g_physics_memory.pages_free = g_physics_memory.pages_total - g_physics_memory.pages_used;
}

void memory_map_int() {
    // validate
    if (VALID_MEMORY_FROM != g_physics_memory.addr_start) {
        printk("[%s:%d] no valid physics memory\n", __FILE__, __LINE__);
        return;
    }

    g_physics_memory_map.addr_base = (uint)VALID_MEMORY_FROM;
    g_physics_memory_map.map = (uchar*)VALID_MEMORY_FROM;

    // total # of pages can be used
    g_physics_memory_map.pages_total = g_physics_memory.pages_total;

    // reset
    memset(g_physics_memory_map.map, 0, g_physics_memory_map.pages_total);

    // 1B map 1 page
    g_physics_memory_map.bitmap_item_used = g_physics_memory_map.pages_total / PAGE_SIZE;
    if (0 != g_physics_memory_map.pages_total % PAGE_SIZE) {
        g_physics_memory_map.bitmap_item_used += 1;
    }

    for (int i = 0; i < g_physics_memory_map.bitmap_item_used; ++i) {
        g_physics_memory_map.map[i] = 1;
    }

    printk("physics memory map position: 0x%X(%dM) - 0x%X(%dM)\n",
           g_physics_memory_map.map,
           ((int)g_physics_memory_map.map) / 1024 / 1024,
           g_physics_memory_map.addr_base,
           g_physics_memory_map.addr_base / 1024 / 1024);

    printk("physical memory starts here: 0x%X(%dM), used: %d pages\n",
           g_physics_memory_map.addr_base, g_physics_memory_map.addr_base / 1024 / 1024,
           g_physics_memory_map.bitmap_item_used);
}

void print_check_memory_info() {
    check_memory_info_t* p = (check_memory_info_t*)ARDS_ADDR;
    check_memmory_item_t* p_data = (check_memmory_item_t*)(ARDS_ADDR + 2);

    unsigned short times = p->times;

    printk("====== memory check info =====\n");

    for (int i = 0; i < times; ++i) {
        check_memmory_item_t* tmp = p_data + i;

        printk("\t %x, %x, %x, %x, %d\n", tmp->base_addr_high, tmp->base_addr_low,
               tmp->length_high, tmp->length_low, tmp->type);
    }

    printk("====== memory check info =====\n");
}

void* get_free_page() {
    bool find = false;

    int i = g_physics_memory_map.bitmap_item_used;
    for (; i < g_physics_memory.pages_total; ++i) {
        if (0 == g_physics_memory_map.map[i]) {
            find = true;
            break;
        }
    }

    if (!find) {
        printk("memory used up!");
        return NULL;
    }

    g_physics_memory_map.map[i] = 1;
    g_physics_memory_map.bitmap_item_used++;

    void* ret = (void*)(g_physics_memory_map.addr_base + (i << 12));

    printk("[%s]return: 0x%X, used: %d pages\n", __FUNCTION__, ret, g_physics_memory_map.bitmap_item_used);

    return ret;
}

void free_page(void* p) {
    if (p < g_physics_memory.addr_start || p > g_physics_memory.addr_end) {
        printk("invalid address!");
        return;
    }

    int index = (int)(p - g_physics_memory_map.addr_base) >> 12;

    g_physics_memory_map.map[index] = 0;
    g_physics_memory_map.bitmap_item_used--;

    printk("[%s]return: 0x%X, used: %d pages\n", __FUNCTION__, p, g_physics_memory_map.bitmap_item_used);
}