#include "../include/linux/fs.h"
#include "../include/linux/hd.h"
#include "../include/linux/bitmap.h"
#include "../include/linux/kernel.h"
#include "../include/assert.h"

extern char inode_bitmap_buf[512];
extern bitmap_t inode_bitmap;

extern hd_t* g_active_hd;
extern super_block_t* g_active_super_block;

void iset(u32 index, bool v) {
    bitmap_set(&inode_bitmap, index, v);

    // calculate location of bitmap
    int bitmap_sector = g_active_super_block->inode_bitmap_lba;

    // write back to hdd
    int write_size = bwrite(g_active_hd->dev_no, bitmap_sector, inode_bitmap_buf, 512);
    assert(-1 != write_size);
}

int iget() {
    int index = bitmap_scan(&inode_bitmap, 1);

    bitmap_set(&inode_bitmap, index, 1);

    // calculate location of bitmap
    int bitmap_sector = g_active_super_block->inode_bitmap_lba;

    // write back to hdd
    int write_size = bwrite(g_active_hd->dev_no, bitmap_sector, inode_bitmap_buf, 512);
    assert(-1 != write_size);

    return index;
}