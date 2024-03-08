#include "../../include/linux/hd.h"
#include "../../include/linux/fs.h"
#include "../../include/linux/kernel.h"
#include "../../include/linux/task.h"
#include "../../include/assert.h"
#include "../../include/string.h"
#include "../../include/asm/io.h"

extern task_t* wait_for_request;
extern hd_request_t g_hd_request;

dev_handler_fun_t dev_interrupt_handler;


bool dev_handler_fast = false;


static int win_result() {
    int i = in_byte(HD_STATUS);


    g_hd_request.state = -1;



    if ((i & (BUSY_STAT | READY_STAT | WRERR_STAT | SEEK_STAT | ERR_STAT))
        == (READY_STAT | SEEK_STAT))
        return 0; /* ok */

    if (i & 1) {
        i = in_byte(HD_ERROR);
    }

    return i;
}

void do_identify() {



    g_hd_request.state = -1;

    int status = 0;
    if(0 != (status = win_result())) {
        panic("identify disk error: %d\n", status);
    }

    // read hdd
    port_read(HD_DATA, g_hd_request.buffer, 512 * g_hd_request.nr_sectors / 2);

    // unblock
    task_unblock(wait_for_request);
}

void read_intr() {



    g_hd_request.state = -1;

    int status = 0;
    if(0 != (status = win_result())) {
        panic("identify disk error: %d\n", status);
    }


    port_read(HD_DATA, g_hd_request.buffer, 512 * g_hd_request.nr_sectors / 2);

    task_unblock(wait_for_request);
}

void write_intr() {

    g_hd_request.state = -1;

    if(0 != (g_hd_request.bh->handler_state = win_result())) {
        panic("write disk error: %d\n", g_hd_request.bh->handler_state);
    }


    task_unblock(wait_for_request);
}

static void hd_out(char hd, int from, int count, unsigned int cmd, dev_handler_fun_t handler) {

    dev_interrupt_handler = handler;

    out_byte(HD_NSECTOR, count);
    out_byte(HD_SECTOR, from & 0xff);
    out_byte(HD_LCYL, from >> 8 & 0xff);
    out_byte(HD_HCYL, from >> 16 & 0xff);
    out_byte(HD_CURRENT, 0b11100000 | (hd << 4) | (from >> 24 & 0xf));
    out_byte(HD_COMMAND, cmd);
}

void do_hd_request() {
    dev_handler_fast = false;

    switch (g_hd_request.cmd) {
        case READ:
            hd_out(g_hd_request.dev, g_hd_request.sector, g_hd_request.nr_sectors, WIN_READ, read_intr);

            break;
        case WRITE:
            hd_out(g_hd_request.dev, g_hd_request.sector, g_hd_request.nr_sectors, WIN_WRITE, write_intr);


            int r = 0;
            for(int i = 0 ; i < 3000 && !(r = in_byte(HD_STATUS) & DRQ_STAT) ; i++);
            if (!r) {
                panic("Failed to write to the hard disk");
            }

            // begin to write
            port_write(HD_DATA, g_hd_request.buffer, 256);

            break;
        case CHECK:
            dev_handler_fast = true;
            hd_out(g_hd_request.dev, g_hd_request.sector, g_hd_request.nr_sectors, 0xec, do_identify);

            break;
        default:
            break;
    }
}


hd_t* get_hd_info(u8 dev) {
    buffer_head_t* bh = kmalloc(sizeof(buffer_head_t));

    bh->data = kmalloc(512);
    bh->dev = dev;
    bh->sector_from = 0;
    bh->sector_count = 1;

    ll_rw_block(CHECK, bh);

    //
    hd_t* hd = kmalloc(sizeof(hd_t));

    // get hdd info
    memcpy(hd->number, bh->data + 10 * 2, 10 * 2);
    hd->number[21] = '\0';

    memcpy(hd->model, bh->data + 27 * 2, 20 * 2);
    hd->model[41] = '\0';

    hd->sectors = *(int*)(bh->data + 60 * 2);

    kfree_s(bh->data, 512);
    kfree_s(bh, sizeof(buffer_head_t));


    return hd;
}

void print_disk_info(hd_t* info) {
    printk("===== Hard Disk Info Start =====\n");
    printk("Hard disk Serial number: %s\n", info->dev_no);
    printk("Drive model: %s\n", info->model);
    printk("Hard disk size: %d sectors, %d M\n", info->sectors, info->sectors * 512 / 1024 / 1024);
    printk("===== Hard Disk Info End =====\n");
}