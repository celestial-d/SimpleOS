#include "../include/asm/system.h"
#include "../include/linux/kernel.h"
#include "../include/linux/mm.h"
#include "../include/linux/task.h"
#include "../include/string.h"

#define PDT_START_ADDR 0x20000

#define VIRTUAL_MEM_START 0x200000

extern task_t* current;

int* g_pdt = (int*)0x20000;

void* virtual_memory_init() {
    int* pdt = (int*)PDT_START_ADDR;

    // clear
    memset(pdt, 0, PAGE_SIZE);

    for (int i = 0; i < 4; ++i) {
        // pdt and pde
        int ptt = (int)PDT_START_ADDR + ((i + 1) * 0x1000);
        int pde = 0b00000000000000000000000000000111 | ptt;

        pdt[i] = pde;

        int* ptt_arr = (int*)ptt;

        if (0 == i) {
            // first mapping created for kernel
            for (int j = 0; j < 0x400; ++j) {
                int* item = &ptt_arr[j];

                int virtual_addr = j * 0x1000;
                *item = 0b00000000000000000000000000000111 | virtual_addr;
            }
        } else {
            //for (int j = 0; j < 0x400; ++j) {
            //    int* item = &ptt_arr[j];

            //    int virtual_addr = j * 0x1000;
            //    virtual_addr = virtual_addr + i * 0x400 * 0x1000;

            //    *item = 0b00000000000000000000000000000111 | virtual_addr;
            //}
        }
    }

    BOCHS_DEBUG_MAGIC

    set_cr3((uint)pdt);

    enable_page();

    BOCHS_DEBUG_MAGIC

    printk("pdt addr: 0x%p\n", pdt);

    return pdt;
}

int get_pde_by_addr(int addr) {
    int index = addr >> 22;
    INFO_PRINT("pdt index: %d\n", index);

    int ret = *(int*)(g_pdt + index);
    INFO_PRINT("pde val: 0x%x\n", ret);

    return ret;
}

int get_pte_by_addr(int addr) {
    int pde_index = addr >> 22;
    int pte_index = addr >> 12 & 0x3ff;

    INFO_PRINT("pde_index: %d, pte_index: %d\n", pde_index, pte_index);

    int* pde = *(int**)(g_pdt + pde_index);
    INFO_PRINT("pde val: 0x%08x\n", pde);

    int pte = *(pde + pte_index);
    INFO_PRINT("pte val: 0x%08x\n", pte);

    return pte;
}

static int* get_pttaddr_by_addr(int addr) {
    int pde_index = addr >> 22;
    int pte_index = addr >> 12 & 0x3ff;

    INFO_PRINT("pde_index: %d, pte_index: %d\n", pde_index, pte_index);

    int* pde = *(int**)(g_pdt + pde_index);
    INFO_PRINT("pde val: 0x%08x\n", pde);

    int* pttaddr = pde + pte_index;
    INFO_PRINT("pte addr: 0x%08x\n", pttaddr);

    return (int*)((int)pttaddr & 0xfffff000);
}

void handle_page_fault(int addr) {
    INFO_PRINT("===== start handle page fault ====\n");

    // abnormal for pte attributes?

    int pte = get_pte_by_addr(addr);

    // physical page
    if (0 == pte) {
        ERROR_PRINT("pte == 0, start handle..\n");


        void* page = get_free_page();
        int* pttaddr = get_pttaddr_by_addr(addr);

        INFO_PRINT("phyics page: 0x%08x\n", page);

        // calculate ptt index
        int ptt_index = addr >> 12 & 0x3ff;

        *(pttaddr + ptt_index) = (int)page | 0x007;

        return;
    }

    return;
}
