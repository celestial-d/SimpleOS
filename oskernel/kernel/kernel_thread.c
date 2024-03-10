#include "../include/linux/kernel.h"
#include "../include/linux/hd.h"
#include "../include/linux/fs.h"
#include "../include/linux/task.h"
#include "../include/linux/sys.h"
#include "../include/d_shell.h"
#include "../include/string.h"

extern task_t* current;
extern hd_t* g_active_hd;

task_t* wait_for_request = NULL;

void test_page_fault(const char* param) {
    if (!strcmp(param, "1")) {
        INFO_PRINT("single page fault..\n");

        int cr2 = get_cr2();
        INFO_PRINT("cr2: 0x%x\n", cr2);

        // assume virtual memory only support 4M page
        int* p = (int*)(4 * 1024 * 1024);
        *p = 1;

        INFO_PRINT("handle page fault success: %d\n", *p);
    } else if (!strcmp(param, "2")) {
        int cr2 = get_cr2();
        INFO_PRINT("cr2: 0x%x\n", cr2);

        handle_page_fault(cr2);
    }
}

void* kernel_thread_fun(void* arg) {
    hd_init();

    // get hdb info
    init_active_hd_info(g_active_hd->dev_no);
    init_active_hd_partition();

    // create superblk
    init_super_block();

    reset_bitmap();

    // create root dir
    create_root_dir();

    // create /
    sys_open("/", O_RDWR);

    active_d_shell();

    while (true);
}