#include "../include/linux/fs.h"
#include "../include/linux/hd.h"
#include "../include/linux/bitmap.h"
#include "../include/linux/kernel.h"
#include "../include/assert.h"
#include "../include/asm/system.h"

extern char block_bitmap_buf[512];
extern bitmap_t block_bitmap;

extern hd_t* g_active_hd;
extern super_block_t* g_active_super_block;

void set_data_sector(u32 index, bool v) {
    bitmap_set(&block_bitmap, index - g_active_super_block->data_start_lba, v);

    // calculate which sector
    int bitmap_sector = g_active_super_block->block_bitmap_lba;

    // write back to hdd
    int write_size = bwrite(g_active_hd->dev_no, bitmap_sector, block_bitmap_buf, 512);
    assert(-1 != write_size);
}

int get_data_sector() {
    int index = bitmap_scan(&block_bitmap, 1);

    bitmap_set(&block_bitmap, index, 1);

    // calculate which sector
    int bitmap_sector = g_active_super_block->block_bitmap_lba;

    // write back to hdd
    int write_size = bwrite(g_active_hd->dev_no, bitmap_sector, block_bitmap_buf, 512);
    assert(-1 != write_size);

    return index + g_active_super_block->data_start_lba;
}