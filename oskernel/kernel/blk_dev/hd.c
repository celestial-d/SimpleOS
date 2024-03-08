#include "../../include/linux/hd.h"
#include "../../include/linux/fs.h"
#include "../../include/linux/mm.h"
#include "../../include/linux/kernel.h"
#include "../../include/linux/bitmap.h"
#include "../../include/linux/task.h"
#include "../../include/string.h"
#include "../../include/assert.h"
#include "../../include/d_shell.h"

extern task_t* current;


#define HD_PARTITION_MAX    4

#define SUPER_BLOCK_MAX     16


#define HD_NUMBER_MEMORY_PTR    0x475


#define IDE_CHANNEL_NUMBER  2

// depends on qemu emulation
hd_channel_t    g_hd_channel[IDE_CHANNEL_NUMBER];

// total number of hdd
uint g_hd_number = 0;


hd_t* g_active_hd = NULL;


super_block_t   g_super_block[SUPER_BLOCK_MAX];


super_block_t*  g_active_super_block;

// block bitmap
char block_bitmap_buf[512] = {0};
bitmap_t block_bitmap;

// inode bitmap
char inode_bitmap_buf[512] = {0};
bitmap_t inode_bitmap;

static void _ide_channel_init() {
    for (int i = 0; i < IDE_CHANNEL_NUMBER; ++i) {
        hd_channel_t* channel = &g_hd_channel[i];

        if (0 == i) {
            channel->port_base = 0x1f0;
            channel->irq_no = 0x20 + 14;
        } else {
            channel->port_base = 0x170;
            channel->irq_no = 0x20 + 15;
        }
    }
}

static void _hd_init() {

    u8 hd_number = *(u8*)HD_NUMBER_MEMORY_PTR;

    g_hd_number = hd_number;

    INFO_PRINT("disk number: %d\n", hd_number);

    // init, at most 4 hdd due to limitation of qemu
    for (int i = 0; i < hd_number; ++i) {
        hd_t* hd;

        switch (i) {
            case 0:
                hd = &g_hd_channel[0].hd[0];

                hd->dev_no = i;
                hd->is_master = 1;
                hd->channel = &g_hd_channel[0];

                break;
            case 1:
                hd = &g_hd_channel[0].hd[1];

                hd->dev_no = i;
                hd->is_master = 0;
                hd->channel = &g_hd_channel[0];

                break;
            case 2:
                hd = &g_hd_channel[1].hd[0];

                hd->dev_no = i;
                hd->is_master = 1;
                hd->channel = &g_hd_channel[1];

                break;
            case 3:
                hd = &g_hd_channel[1].hd[1];

                hd->dev_no = i;
                hd->is_master = 0;
                hd->channel = &g_hd_channel[1];

                break;
        }

        if (1 == i) {
            g_active_hd = hd;
        }
    }
}

void hd_init() {
    INFO_PRINT("hd init...\n");

    _ide_channel_init();
    _hd_init();
}

void init_active_hd_info(u8 dev) {
    CLI

    buffer_head_t* bh = kmalloc(sizeof(buffer_head_t));

    bh->data = kmalloc(512);
    bh->dev = dev;
    bh->sector_from = 0;
    bh->sector_count = 1;

    ll_rw_block(CHECK, bh);

    //after interrupt, get hdd info
    memcpy(g_active_hd->number, bh->data + 10 * 2, 10 * 2);
    g_active_hd->number[21] = '\0';

    memcpy(g_active_hd->model, bh->data + 27 * 2, 20 * 2);
    g_active_hd->model[41] = '\0';

    g_active_hd->sectors = *(int*)(bh->data + 60 * 2);

    kfree_s(bh->data, 512);
    kfree_s(bh, sizeof(buffer_head_t));

    STI
}

void init_active_hd_partition() {
    CLI

    // read hdb MBR
    buffer_head_t* buff = bread(g_active_hd->dev_no, 0, 1);

    // get hdd partition info
    mbr_sector_t* mbr_sector = buff->data;
    memcpy(g_active_hd->partition, mbr_sector->partition, 64);

    // free
    kfree_s(buff->data, 512);
    kfree_s(buff, sizeof(buff));

    STI
}

static super_block_t* find_empty_super_block() {
    int index = -1;

    for (int i = 0; i < SUPER_BLOCK_MAX; ++i) {
        if (0 == g_super_block[i].type) {
            index = i;

            break;
        }
    }

    if (-1 == index) {
        panic("No superblock is available");
    }

    return &g_super_block[index];
}


static void load_super_block(buffer_head_t* buff) {
    INFO_PRINT("The superblock has been initialized. loading...\n");

    for (int i = 0; i < HD_PARTITION_MAX; ++i) {
        hd_partition_t* partition = &g_active_hd->partition[i];

        if (0 == partition->start_sect) {
            INFO_PRINT("hd partition %d is null! ignore..\n", i);

            continue;
        }

        // do not repeat
        if (0 != i) {
            buff = bread(g_active_hd->dev_no, g_active_hd->partition[i].start_sect + 1, 1);
        }

        // save superblock of each hdd
        memcpy(find_empty_super_block(), buff->data, 512);

        if (0 != i) {
            kfree_s(buff->data, 512);
            kfree_s(buff, sizeof(buffer_head_t));
        }

        INFO_PRINT("hd: %d, partition: %d, completed..\n", g_active_hd->dev_no, i);

        // mount
        if (0 == i) {
            mount_partition(&g_super_block[0]);

            // init block bitmap
            INFO_PRINT("read block bitmap sector: %d\n", g_super_block->block_bitmap_lba);

            memset(block_bitmap_buf, 0, 512);
            bitmap_make(&block_bitmap, block_bitmap_buf, 512, 0);

            // init inode bitmap
            INFO_PRINT("read inode bitmap sector: %d\n", g_super_block->inode_bitmap_lba);

            memset(inode_bitmap_buf, 0, 512);
            bitmap_make(&inode_bitmap, inode_bitmap_buf, 512, 0);
        }
    }
}

static bool check_super_block_is_init() {
    bool ret = false;


    buffer_head_t* buff = bread(g_active_hd->dev_no, g_active_hd->partition[0].start_sect + 1, 1);


    if (0 != *(uchar*)buff->data) {
        ret = true;

        load_super_block(buff);
    }

    kfree_s(buff->data, 512);
    kfree_s(buff, sizeof(buffer_head_t));

    return ret;
}

void init_super_block() {
    CLI

    assert(NULL != g_active_hd);

    INFO_PRINT("===== start: init super block =====\n");


    if (check_super_block_is_init()) {
        return;
    }

    INFO_PRINT("The superblock starts to initialize...\n");

    for (int i = 0; i < HD_PARTITION_MAX; ++i) {

        hd_partition_t* partition = &g_active_hd->partition[i];

        if (0 == partition->start_sect) {
            INFO_PRINT("hd partition %d is null! pass..\n", i);

            continue;
        }

        super_block_t* super_block = find_empty_super_block();
        super_block->type = EXT;
        super_block->lba_base = partition->start_sect;
        super_block->sector_count = partition->nr_sects;

        // 512 * 8 = 4096 sectors
        int save_free_sector_bitmap_sector = partition->nr_sects / 512 / 8;
        save_free_sector_bitmap_sector += (0 == partition->nr_sects % (512 * 8))? 0 : 1;


        super_block->block_bitmap_sects = save_free_sector_bitmap_sector;
        super_block->block_bitmap_lba = partition->start_sect + 1 + 1;


        super_block->inode_count = 4096;
        super_block->inode_bitmap_lba = super_block->block_bitmap_lba + super_block->block_bitmap_sects;
        super_block->inode_bitmap_sects = 1;


        super_block->inode_table_lba = super_block->inode_bitmap_lba + super_block->inode_bitmap_sects;


        super_block->inode_table_sects = super_block->inode_count * sizeof(d_inode_t) / 512;
        super_block->inode_table_sects += (0 == super_block->inode_count * sizeof(d_inode_t) % 512)? 0 : 1;

        super_block->root_lba = super_block->inode_table_lba + super_block->inode_table_sects;


        super_block->data_start_lba = super_block->root_lba + 1;


        if (0 == i) {
            mount_partition(super_block);


            bitmap_init(&block_bitmap, block_bitmap_buf, 512, 0);


            bitmap_init(&inode_bitmap, inode_bitmap_buf, 512, 0);
        }


        size_t write_size = bwrite(g_active_hd->dev_no, partition->start_sect + 1, super_block, 512);
        if (-1 == write_size) {
            panic("save super block fail");
        } else {
            INFO_PRINT("save super block success: dev: %d, partition index: %d\n", g_active_hd->dev_no, i);
        }
    }

    INFO_PRINT("===== end: init super block =====\n");

    STI
}

void mount_partition(super_block_t* block) {
    assert(NULL != block);

    if (NULL != g_active_super_block) {
        panic("Please uninstall and mount it");
    }

    g_active_super_block = block;
}

void unmount_partition() {
    g_active_super_block = NULL;
}

void print_super_block() {
    assert(NULL != g_active_super_block);

    printk("lba base: %d\n", g_active_super_block->lba_base);
    printk("sector count: %d\n", g_active_super_block->sector_count);
    printk("os bootloader sector: %d\n", g_active_super_block->lba_base);
    printk("super block sector: %d\n", g_active_super_block->lba_base + 1);
    printk("block bitmap lba: %d\n", g_active_super_block->block_bitmap_lba);
    printk("block bitmap sectors: %d\n", g_active_super_block->block_bitmap_sects);
    printk("inode bitmap lba: %d\n", g_active_super_block->inode_bitmap_lba);
    printk("inode bitmap sectors: %d\n", g_active_super_block->inode_bitmap_sects);
    printk("inode table lba: %d\n", g_active_super_block->inode_table_lba);
    printk("inode table sectors: %d\n", g_active_super_block->inode_table_sects);
    printk("root lba: %d\n", g_active_super_block->root_lba);
    printk("data start lba: %d\n", g_active_super_block->data_start_lba);
}

void print_block_bitmap() {
    INFO_PRINT("print block bitmap\n");

    assert(NULL != g_active_super_block);

    INFO_PRINT("block bitmap lba: %d\n", g_active_super_block->block_bitmap_lba);
    INFO_PRINT("block bitmap sectors: %d\n", g_active_super_block->block_bitmap_sects);

    printk("[mm]block bitmap: ");
    for (int i = 0; i < 10; ++i) {
        printk("0x%02x ", block_bitmap_buf[i]);
    }
    printk("\n");

    buffer_head_t* bh = bread(g_active_hd->dev_no, g_active_super_block->block_bitmap_lba, 1);
    printk("[hd]block bitmap: ");
    for (int i = 0; i < 10; ++i) {
        printk("0x%02x ", bh->data[i]);
    }
    printk("\n");

    kfree_s(bh->data, 512);
    kfree_s(bh, sizeof(buffer_head_t));
}

void reset_block_bitmap() {
    INFO_PRINT("reset block bitmap\n");


    memset(block_bitmap_buf, 0, 512);

    // write to hdd
    int write_size = bwrite(g_active_hd->dev_no, g_active_super_block->block_bitmap_lba, block_bitmap_buf, 512);
    assert(-1 != write_size);
}

void print_inode_bitmap() {
    INFO_PRINT("print inode bitmap\n");

    assert(NULL != g_active_super_block);

    INFO_PRINT("inode bitmap lba: %d\n", g_active_super_block->inode_bitmap_lba);
    INFO_PRINT("inode bitmap sectors: %d\n", g_active_super_block->inode_bitmap_sects);

    printk("[mm]inode bitmap: ");
    for (int i = 0; i < 10; ++i) {
        printk("0x%02x ", inode_bitmap_buf[i]);
    }
    printk("\n");

    buffer_head_t* bh = bread(g_active_hd->dev_no, g_active_super_block->inode_bitmap_lba, 1);
    printk("[hd]inode bitmap: ");
    for (int i = 0; i < 10; ++i) {
        printk("0x%02x ", bh->data[i]);
    }
    printk("\n");

    kfree_s(bh->data, 512);
    kfree_s(bh, sizeof(buffer_head_t));
}

void reset_inode_bitmap() {
    INFO_PRINT("reset inode bitmap\n");


    memset(inode_bitmap_buf, 0, 512);

    // write to hdd
    int write_size = bwrite(g_active_hd->dev_no, g_active_super_block->inode_bitmap_lba, inode_bitmap_buf, 512);
    assert(-1 != write_size);
}

void print_bitmap() {
    print_block_bitmap();
    print_inode_bitmap();
}

void reset_bitmap() {
    CLI

    reset_block_bitmap();
    reset_inode_bitmap();

    STI
}

void create_root_dir() {
    CLI

    INFO_PRINT("===== start: create root dir =====\n");

    int write_size;
    char* name = "/";

    // 1、create dir
    dir_entry_t* dir_entry = kmalloc(512);

    memset(dir_entry->name, 0, 16);
    memcpy(dir_entry->name, name, strlen(name));

    dir_entry->ft = FILE_TYPE_DIRECTORY;
    dir_entry->dir_index = 0;

    // 2、apply inode index
    dir_entry->inode = iget();

    // 3、write to hdd
    write_size = bwrite(g_active_hd->dev_no, g_active_super_block->root_lba, dir_entry, 512);
    assert(-1 != write_size);

    INFO_PRINT("[save root directory entry]sector: %d\n", g_active_super_block->root_lba);


    int inode_sector = g_active_super_block->inode_table_lba;

    INFO_PRINT("[save inode]inode sector: %d\n", inode_sector);

    // create inode
    memset(dir_entry, 0, 512);

    d_inode_t* inode = dir_entry;
    inode->i_mode = 777;
    inode->i_size = 0;
    inode->i_zone_off = 0;

    // apply data block
    inode->i_zone[inode->i_zone_off++] = get_data_sector();
    INFO_PRINT("zone: %d, off: %d\n", inode->i_zone[0], inode->i_zone_off);

    // inode is writen to hdd
    write_size = bwrite(g_active_hd->dev_no, inode_sector, inode, 512);
    assert(-1 != write_size);

    kfree_s(dir_entry, sizeof(dir_entry_t));

    end:
    INFO_PRINT("===== end: create root dir =====\n");

    STI
}

void print_root_dir() {
    assert(NULL != current);
    assert(NULL != current->current_active_dir);

    printk("[mm]current work dir: %s\n", current->current_active_dir->name);
    printk("[mm]current work dir inode: %d\n", current->current_active_dir->inode);
    printk("[mm]current work dir data index: %d\n", current->current_active_dir->dir_index);

    printk("[mm]current work dir data zone: ");
    for (int i = 0; i < current->current_active_dir_inode->i_zone_off; ++i) {
        printk("%d ", current->current_active_dir_inode->i_zone[i]);
    }
    printk("\n");

    buffer_head_t* bh = bread(g_active_hd->dev_no, g_active_super_block->root_lba, 1);
    dir_entry_t* dir = bh->data;
    kfree_s(bh, sizeof(buffer_head_t));

    printk("[hd]current work dir: %s\n", dir->name);
    printk("[hd]current work dir inode: %d\n", dir->inode);
    printk("[hd]current work dir data index: %d\n", dir->dir_index);
    kfree_s(bh->data, 512);

    bh = bread(g_active_hd->dev_no, g_active_super_block->inode_table_lba, 1);
    m_inode_t* inode = bh->data;

    printk("[hd]current work dir data zone: ");
    for (int i = 0; i < inode->i_zone_off; ++i) {
        printk("%d ", inode->i_zone[i]);
    }
    printk("\n");

    kfree_s(bh->data, 512);
    kfree_s(bh, sizeof(buffer_head_t));
}

void ls_current_dir() {
    // get root data
    dir_entry_t* entry = NULL;
    buffer_head_t* bh = NULL;
    u32 dir_inode = 0;
    u32 dir_index = 0;

    entry = current->current_active_dir;
    dir_inode = entry->inode;
    dir_index = entry->dir_index;

    printk("inode index: %d\n", dir_inode);
    printk("dir index: %d\n", dir_index);

    // get inode
    bh = bread(g_active_hd->dev_no, g_active_super_block->inode_table_lba, 1);
    m_inode_t* inode = bh->data + dir_inode * sizeof(m_inode_t);
    kfree_s(bh, sizeof(buffer_head_t));


    int zone = inode->i_zone[inode->i_zone_off - 1];
    kfree_s(bh->data, 512);

    printk("data zone: %d\n", zone);

    if (0 == dir_index) {
        printk("empty!\n");
        return;
    }

    // get dic
    bh = bread(g_active_hd->dev_no, zone, 1);

    // output
    entry = bh->data;
    for (int i = 0; i < dir_index; ++i) {
        printk("%s%s ", entry->name, (FILE_TYPE_DIRECTORY == entry->ft)? "[D]" : "[R]");

        entry++;
    }

    printk("\n");

    // free
    kfree_s(bh->data, 512);
    kfree_s(bh, sizeof(bh));
}


void create_dir(char* name) {
    CLI

    int write_size, inode_index;

    int parent_inode_current_zone = current->current_active_dir_inode->i_zone[current->current_active_dir_inode->i_zone_off - 1];
    assert(parent_inode_current_zone >= 0);

    INFO_PRINT("parent_inode_current_zone: %d\n", parent_inode_current_zone);


    buffer_head_t* bh = bread(g_active_hd->dev_no, parent_inode_current_zone, 1);

    INFO_PRINT("create %d directory\n", current->current_active_dir->dir_index);

    // 1、create dir
    dir_entry_t* dir_entry = bh->data + sizeof(dir_entry_t) * current->current_active_dir->dir_index++;

    memset(dir_entry->name, 0, 16);
    memcpy(dir_entry->name, name, strlen(name));

    dir_entry->ft = FILE_TYPE_DIRECTORY;
    dir_entry->dir_index = 0;

    // 2、apply inode index
    inode_index = dir_entry->inode = iget();
    print_inode_bitmap();

    // 3、write to hdd
    write_size = bwrite(g_active_hd->dev_no, parent_inode_current_zone, bh->data, 512);
    assert(-1 != write_size);

    INFO_PRINT("[save directory entry]sector: %d\n", parent_inode_current_zone);

    // change dir and write back to hdd
    memset(dir_entry, 0, 512);
    memcpy(dir_entry, current->current_active_dir, sizeof(dir_entry_t));

    INFO_PRINT("[write parent dir info]dir index:%d\n", current->current_active_dir->dir_index);

    write_size = bwrite(g_active_hd->dev_no, g_active_super_block->root_lba, dir_entry, 512);
    assert(-1 != write_size);

    //save inode
    int inode_sector = g_active_super_block->inode_table_lba;

    INFO_PRINT("[save inode]inode sector: %d\n", inode_sector);

    bh = bread(g_active_hd->dev_no, inode_sector, 1);

    d_inode_t* inode = bh->data + inode_index * sizeof(d_inode_t);

    inode->i_mode = 777;
    inode->i_size = 0;
    inode->i_zone_off = 0;

    // apply data block
    inode->i_zone[inode->i_zone_off++] = get_data_sector();
    print_block_bitmap();

    // write inode to hdd
    write_size = bwrite(g_active_hd->dev_no, inode_sector, bh->data, 512);
    assert(-1 != write_size);

    kfree_s(dir_entry, sizeof(dir_entry_t));

    STI
}

static int parse_filepath_get_data_size(const char* filepath) {
    assert(NULL != filepath);

    int ret = 0;

    char* tmp = filepath;
    char ch;
    while ('\0' != (ch = *tmp++)) {
        if ('/' == ch) ret++;
    }

    return ret;
}

static filepath_parse_data_t* parse_filepath(const char* filepath) {
    assert(NULL != filepath);

    filepath_parse_data_t* ret = kmalloc(sizeof(filepath_parse_data_t));

    ret->data_size = parse_filepath_get_data_size(filepath);
    ret->data = kmalloc(sizeof(char*) * ret->data_size);

    // analyze "/"
    char ch;
    int ch_off = 0;
    int ret_off = 0;
    while ('\0' != (ch = *(filepath + ch_off))) {
        if ('/' == ch) {
            *(ret->data + ret_off++) = (char *)ch_off;
        }

        ch_off += 1;
    }

    // get relationship of dirs
    for (int i = 0; i < ret->data_size; ++i) {

        int index = (int)ret->data[i];


        int index_next = (i != ret->data_size - 1)? (int)ret->data[i + 1] : strlen(filepath);


        assert(index_next - index - 1 < 16);


        ret->data[i] = kmalloc(16);
        memset(ret->data[i], 0, 16);
        memcpy(ret->data[i], filepath + index + 1, index_next - index - 1);
    }

    return ret;
}


static dir_entry_t* get_root_directory_children() {
    if (0 == current->root_dir->dir_index) {
        return NULL;
    }


    int zone = current->root_dir_inode->i_zone[current->root_dir_inode->i_zone_off - 1];


    buffer_head_t* bh = bread(g_active_hd->dev_no, zone, 1);

    dir_entry_t* ret = bh->data;


    kfree_s(bh, sizeof(bh));

    return ret;
}

void rm_directory(const char* filepath) {
    CLI

    assert(NULL != filepath);

    int write_size = 0;
    char* buf = kmalloc(512);


    filepath_parse_data_t* filepath_data = parse_filepath(filepath);
    if (0 == filepath_data->data_size) {
        printk("What to rm?\n");
        return;
    }


    dir_entry_t* children = get_root_directory_children();
    if (NULL == children || 0 == children->name[0]) { // 如果是空目录
        printk("empty!\n");
        return;
    }


    dir_entry_t* entry = NULL;
    dir_entry_t* tmp = (dir_entry_t*)children;
    while (NULL != tmp && (0 != tmp->name[0])) {
        if (!strcmp(filepath_data->data[0], tmp->name)) {
            entry = tmp;
            break;
        }

        tmp++;
    }


    if (NULL == entry) {
        printk("empty!\n");
        return;
    }

    print_dir_entry(entry);


    current->current_active_dir->dir_index--;

    memcpy(buf, current->current_active_dir, sizeof(dir_entry_t));

    INFO_PRINT("[write parent dir info]dir index:%d\n", current->current_active_dir->dir_index);

    write_size = bwrite(g_active_hd->dev_no, g_active_super_block->root_lba, buf, 512);
    assert(-1 != write_size);


    iset(entry->inode, 0);


    buffer_head_t* bh = bread(g_active_hd->dev_no, g_active_super_block->inode_table_lba, 1);
    m_inode_t* inode = bh->data + entry->inode * sizeof(d_inode_t);

    set_data_sector(inode->i_zone[0], 0);

    kfree_s(buf, 512);

    STI
}

void cd_directory(const char* filepath) {
    assert(NULL != filepath);

    char buff[16] = {0};
    buffer_head_t* bh = NULL;
    dir_entry_t* entry = NULL;

    if (!strcmp("/", filepath)) {
        //current dir
        memcpy(buff, filepath, 16);
        if (!strcmp(buff, current->current_active_dir->name)) {
            printk("Current working directory is %s, pass...\n", buff);
            kfree_s(entry, 512);
            return;
        }

        bh = bread(g_active_hd->dev_no, g_active_super_block->root_lba, 1);
        entry = bh->data;

        kfree_s(bh, sizeof(buffer_head_t));
    } else {
        // analyze path
        filepath_parse_data_t* filepath_data = parse_filepath(filepath);
        if (0 == filepath_data->data_size) {
            printk("What to rm?\n");
            return;
        }

        // current dir
        memcpy(buff, filepath_data->data[filepath_data->data_size - 1], 16);
        if (!strcmp(buff, current->current_active_dir->name)) {
            printk("Current working directory is %s, pass...\n", buff);
            kfree_s(entry, 512);
            return;
        }


        dir_entry_t* children = get_root_directory_children();
        if (NULL == children || 0 == children->name[0]) { // 如果是空目录
            printk("empty!\n");
            return;
        }

        // exist?
        dir_entry_t* tmp = (dir_entry_t*)children;
        while (NULL != tmp && (0 != tmp->name[0])) {
            if (!strcmp(filepath_data->data[0], tmp->name)) {
                entry = tmp;
                break;
            }

            tmp++;
        }
    }


    if (NULL == entry) {
        printk("empty!\n");
        return;
    }


    if (FILE_TYPE_DIRECTORY != entry->ft) {
        printk("Not a directory!\n");
        return;
    }

    print_dir_entry(entry);

    // get inode
    bh = bread(g_active_hd->dev_no, g_active_super_block->inode_table_lba, 1);
    m_inode_t* inode = bh->data + entry->inode * sizeof(d_inode_t);

    kfree_s(bh, sizeof(buffer_head_t));
    kfree_s(current->current_active_dir, sizeof(dir_entry_t));
    kfree_s(current->current_active_dir_inode, sizeof(m_inode_t));

    current->current_active_dir = entry;
    current->current_active_dir_inode = inode;
}

void print_dir_entry(dir_entry_t* entry) {
    assert(NULL != entry);

    printk("name: %s\n", entry->name);
    printk("file type: %d\n", entry->ft);
    printk("inode: %d\n", entry->inode);
    printk("dir index: %d\n", entry->dir_index);
}

void create_file(char* filename) {
    CLI

    int parent_inode_current_zone, write_size, inode_index;

    parent_inode_current_zone = current->current_active_dir_inode->i_zone[current->current_active_dir_inode->i_zone_off - 1];
    INFO_PRINT("parent data zone: %d\n", parent_inode_current_zone);


    buffer_head_t* bh = bread(g_active_hd->dev_no, parent_inode_current_zone, 1);

    INFO_PRINT("create %d file\n", current->current_active_dir->dir_index);


    dir_entry_t* dir_entry = bh->data + sizeof(dir_entry_t) * current->current_active_dir->dir_index++;

    memset(dir_entry->name, 0, 16);
    memcpy(dir_entry->name, filename, strlen(filename));

    dir_entry->ft = FILE_TYPE_REGULAR;
    dir_entry->dir_index = 0;


    inode_index = dir_entry->inode = iget();

    INFO_PRINT("file inode: %d\n", inode_index);


    write_size = bwrite(g_active_hd->dev_no, parent_inode_current_zone, bh->data, 512);
    assert(-1 != write_size);


    memset(dir_entry, 0, 512);
    memcpy(dir_entry, current->current_active_dir, sizeof(dir_entry_t));

    INFO_PRINT("next dir index:%d\n", current->current_active_dir->dir_index);

    write_size = bwrite(g_active_hd->dev_no, g_active_super_block->root_lba, dir_entry, 512);
    assert(-1 != write_size);


    int inode_sector = g_active_super_block->inode_table_lba;

    INFO_PRINT("inode sector: %d\n", inode_sector);

    bh = bread(g_active_hd->dev_no, inode_sector, 1);

    d_inode_t* inode = bh->data + inode_index * sizeof(d_inode_t);

    inode->i_mode = 777;
    inode->i_size = 512;
    inode->i_zone_off = 0;


    inode->i_zone[inode->i_zone_off++] = get_data_sector();

    INFO_PRINT("data sector: %d\n", inode->i_zone[inode->i_zone_off - 1]);


    write_size = bwrite(g_active_hd->dev_no, inode_sector, bh->data, 512);
    assert(-1 != write_size);

    kfree_s(dir_entry, sizeof(dir_entry_t));

    STI
}

void write_file(const char* content, const char* filename) {
    CLI

    assert(NULL != content);
    assert(NULL != filename);

    dir_entry_t* entry = NULL;


    dir_entry_t* children = get_root_directory_children();
    if (NULL == children || 0 == children->name[0]) {
        INFO_PRINT("file noexist!start create..\n", filename);

        create_file(filename);
        write_file(content, filename);
        return;
    }

    // file exist?
    dir_entry_t* tmp = (dir_entry_t*)children;
    while (NULL != tmp && (0 != tmp->name[0])) {
        if (!strcmp(filename, tmp->name)) {
            entry = tmp;
            break;
        }

        tmp++;
    }


    if (NULL == entry) {
        INFO_PRINT("file noexist!start create..\n", filename);

        create_file(filename);
        write_file(content, filename);
        return;
    }


    if (FILE_TYPE_REGULAR != entry->ft) {
        INFO_PRINT("Not a file!\n");
        return;
    }

    print_dir_entry(entry);

    // get inode
    buffer_head_t* bh = bread(g_active_hd->dev_no, g_active_super_block->inode_table_lba, 1);
    m_inode_t* inode = bh->data + entry->inode * sizeof(d_inode_t);
    int zone = inode->i_zone[inode->i_zone_off - 1];

    INFO_PRINT("data zone:%d\n", zone);

    // write content
    char* buff = inode;
    memset(buff, 0, 512);
    memcpy(buff, content, strlen(content));

    int write_size = bwrite(g_active_hd->dev_no, zone, buff, 512);
    assert(-1 != write_size);

    INFO_PRINT("write file success\n");

    kfree_s(inode, 512);
    kfree_s(bh, sizeof(buffer_head_t));

    STI
}

char* read_file(char* filename) {
    CLI

    assert(NULL != filename);

    char* ret = NULL;
    dir_entry_t* entry = NULL;

    // get dir
    dir_entry_t* children = get_root_directory_children();
    if (NULL == children || 0 == children->name[0]) {
        INFO_PRINT("file noexist:%s\n", filename);

        return ret;
    }

    // exist?
    dir_entry_t* tmp = (dir_entry_t*)children;
    while (NULL != tmp && (0 != tmp->name[0])) {
        if (!strcmp(filename, tmp->name)) {
            entry = tmp;
            break;
        }

        tmp++;
    }


    if (NULL == entry) {
        INFO_PRINT("file noexist:%s\n", filename);

        return ret;
    }

    // is dir?
    if (FILE_TYPE_REGULAR != entry->ft) {
        INFO_PRINT("Not a file!\n");

        return ret;
    }

    print_dir_entry(entry);

    // get inode
    buffer_head_t* bh = bread(g_active_hd->dev_no, g_active_super_block->inode_table_lba, 1);
    m_inode_t* inode = bh->data + entry->inode * sizeof(d_inode_t);
    int zone = inode->i_zone[inode->i_zone_off - 1];

    kfree_s(inode, 512);
    kfree_s(bh, sizeof(buffer_head_t));

    INFO_PRINT("data zone:%d\n", zone);

    bh = bread(g_active_hd->dev_no, zone, 1);
    ret = bh->data;

    STI

    return ret;
}

file_t* kfopen(const char *filename, int flag) {
    file_t* file = NULL;

    int fd = find_empty_file_descriptor();
    INFO_PRINT("fd: %d\n", fd);

    dir_entry_t* entry = NULL;

    // get dir
    dir_entry_t* children = get_root_directory_children();
    if (NULL == children || 0 == children->name[0]) {
        INFO_PRINT("file noexist:%s\n", filename);

        return file;
    }


    dir_entry_t* tmp = (dir_entry_t*)children;
    while (NULL != tmp && (0 != tmp->name[0])) {
        if (!strcmp(filename, tmp->name)) {
            entry = tmp;
            break;
        }

        tmp++;
    }


    if (NULL == entry) {
        INFO_PRINT("file noexist:%s\n", filename);

        return file;
    }


    if (FILE_TYPE_REGULAR != entry->ft) {
        INFO_PRINT("Not a file!\n");

        return file;
    }

    print_dir_entry(entry);


    buffer_head_t* bh = bread(g_active_hd->dev_no, g_active_super_block->inode_table_lba, 1);
    m_inode_t* inode = bh->data + entry->inode * sizeof(d_inode_t);

    kfree_s(inode, 512);
    kfree_s(bh, sizeof(buffer_head_t));

    // create file descriptor
    file = kmalloc(sizeof(file_t));
    file->f_mode = flag;
    file->f_flags = 0;
    file->f_count = fd;
    file->f_inode = inode;
    file->f_pos = 0;

    memcpy(file->name, filename, strlen(filename));

    current->file_descriptor[fd] = file;

    INFO_PRINT("file: 0x%x\n", file);

    return file;
}

int kfclose(FILE *stream) {
    int fd = 0;

    // exist?
    for (int i = 0; i < NR_OPEN; ++i) {
        if (current->file_descriptor[i] = stream) {
            fd = i;

            break;
        }
    }

    if (0 != fd) {
        INFO_PRINT("found: 0x%x\n", stream);
    } else {
        ERROR_PRINT("noexist: 0x%x\n", stream);
        return -1;
    }

    // delete
    current->file_descriptor[fd] = NULL;

    return 1;
}


size_t kfread(void *ptr, size_t size, FILE *stream) {
    if (NULL == ptr || 0 == size || NULL == stream) {
        return -1;
    }

    size_t ret = 0;


    file_t* file = (file_t*)stream;
    int zone = file->f_inode->i_zone[0];
    INFO_PRINT("data zone: %d\n", zone);


    buffer_head_t* bh = bread(g_active_hd->dev_no, zone, 1);


    memcpy(ptr, bh->data, size);


    kfree_s(bh->data, 512);
    kfree_s(bh, sizeof(buffer_head_t));

    return ret;
}

size_t kfwrite(const void *ptr, size_t size, FILE *stream) {
    if (NULL == ptr || 0 == size || NULL == stream) {
        return -1;
    }

    size_t ret = 0;


    file_t* file = (file_t*)stream;
    int zone = file->f_inode->i_zone[0];
    INFO_PRINT("data zone: %d\n", zone);


    buffer_head_t* bh = bread(g_active_hd->dev_no, zone, 1);


    memcpy(bh->data, ptr, size);

    ret = bwrite(g_active_hd->dev_no, zone, bh->data, 1);


    kfree_s(bh->data, 512);
    kfree_s(bh, sizeof(buffer_head_t));

    return ret;
}