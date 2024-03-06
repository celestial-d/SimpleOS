#include "../include/linux/sys.h"
#include "../include/linux/kernel.h"
#include "../include/linux/task.h"
#include "../include/linux/fs.h"
#include "../include/linux/hd.h"
#include "../include/asm/system.h"
#include "../include/string.h"
#include "../include/assert.h"

extern task_t* current;
extern hd_t* g_active_hd;
extern super_block_t* g_active_super_block;

static int open_root_dir(int flags) {
    CLI

    assert(NULL != current);

    int fd = find_empty_file_descriptor();

    // get root dic
    buffer_head_t* bh = bread(g_active_hd->dev_no, g_active_super_block->root_lba, 1);
    current->root_dir = bh->data;
    kfree_s(bh, sizeof(buffer_head_t));

    INFO_PRINT("dir entry name: %s\n", current->root_dir->name);

    // get root dic inode
    bh = bread(g_active_hd->dev_no, g_active_super_block->inode_table_lba, 1);
    current->root_dir_inode = bh->data;
    kfree_s(bh, sizeof(buffer_head_t));

    // create file descriptor
    file_t* file = kmalloc(sizeof(file_t));
    file->f_mode = flags;
    file->f_flags = 0;
    file->f_count = fd;
    file->f_inode = current->root_dir_inode;
    file->f_pos = 0;

    current->file_descriptor[fd] = file;
    current->current_active_dir = current->root_dir;
    current->current_active_dir_inode = current->root_dir_inode;

    STI

    return fd;
}

int sys_open(const char* pathname, int flags) {
    int fd = -1;

    INFO_PRINT("===== start open %s =====\n", pathname);

    if (!strcmp("/", pathname)) {
        fd = open_root_dir(flags);
    } else {

    }

    INFO_PRINT("===== end open %s =====\n", pathname);

    return fd;
}