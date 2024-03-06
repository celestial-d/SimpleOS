#ifndef OS_KERNEL__IDE_H
#define OS_KERNEL__IDE_H

#include "types.h"

/* Hd controller regs. Ref: IBM AT Bios-listing */
#define HD_DATA		0x1f0	/* _CTL when writing */
#define HD_ERROR	0x1f1	/* see err-bits */
#define HD_NSECTOR	0x1f2	/* nr of sectors to read/write */
#define HD_SECTOR	0x1f3	/* starting sector */
#define HD_LCYL		0x1f4	/* starting cylinder */
#define HD_HCYL		0x1f5	/* high byte of starting cyl */
#define HD_CURRENT	0x1f6	/* 101dhhhh , d=drive, hhhh=head */
#define HD_STATUS	0x1f7	/* see status-bits */
#define HD_PRECOMP HD_ERROR	/* same io address, read=error, write=precomp */
#define HD_COMMAND HD_STATUS	/* same io address, read=status, write=cmd */

#define HD_CMD		0x3f6

/* Bits of HD_STATUS */
#define ERR_STAT	0x01    // 1 means error
#define INDEX_STAT	0x02
#define ECC_STAT	0x04
#define DRQ_STAT	0x08
#define SEEK_STAT	0x10
#define WRERR_STAT	0x20
#define READY_STAT	0x40
#define BUSY_STAT	0x80

/* Values for HD_COMMAND */
#define WIN_RESTORE		0x10
#define WIN_READ		0x20
#define WIN_WRITE		0x30
#define WIN_VERIFY		0x40
#define WIN_FORMAT		0x50
#define WIN_INIT		0x60
#define WIN_SEEK 		0x70
#define WIN_DIAGNOSE	0x90
#define WIN_SPECIFY		0x91

/* Bits for HD_ERROR */
#define MARK_ERR	0x01	/* Bad address mark ? */
#define TRK0_ERR	0x02	/* couldn't find track 0 */
#define ABRT_ERR	0x04
#define ID_ERR		0x10
#define ECC_ERR		0x40
#define	BBD_ERR		0x80

#define port_read(port, buf, nr) \
__asm__("cld;rep;insw"::"d" (port),"D" (buf),"c" (nr))

#define port_write(port, buf, nr) \
__asm__("cld;rep;outsw"::"d" (port),"S" (buf),"c" (nr))

typedef void (*dev_handler_fun_t)(void);

typedef struct _hd_partition_t {
    unsigned char boot_ind;		/* 0x80 - active (unused) */
    unsigned char head;
    unsigned char sector;
    unsigned char cyl;
    unsigned char sys_ind;
    unsigned char end_head;
    unsigned char end_sector;
    unsigned char end_cyl;
    unsigned int start_sect;	/* starting sector counting from 0 */
    unsigned int nr_sects;		/* nr of sectors in partition */
} __attribute__((packed)) hd_partition_t;

typedef struct _hd_channel_t hd_channel_t;

typedef struct _hd_t {
    u8              dev_no;
    u8              is_master;      // master 1
    hd_partition_t  partition[4];   // do not consider
    hd_channel_t*   channel;

    // hdd identify return
    char number[10 * 2 + 1];
    char model[20 * 2 + 1];
    int sectors;
} __attribute__((packed)) hd_t;


typedef struct _hd_channel_t {
    hd_t    hd[2];      // index=0 main
    u16     port_base;
    u8      irq_no;
} __attribute__((packed)) hd_channel_t;

typedef struct _mbr_sector_t {
    u8  code[446];
    hd_partition_t partition[4];
    u16 magic;
} __attribute__((packed)) mbr_sector_t;

typedef enum _fs_type_t {
    EXT = 1,
    FAT32,
    NTFS,
} fs_type_t;

typedef struct _super_block_t {
    fs_type_t   type;
    u32 lba_base;
    u32 sector_count;
    u32 inode_count;
    u32 data_start_lba;
    u32 root_lba;

    u32 block_bitmap_lba;
    u32 block_bitmap_sects;

    u32 inode_bitmap_lba;
    u32 inode_bitmap_sects;

    u32 inode_table_lba;
    u32 inode_table_sects;

    char padding[512 - 48];
} __attribute__((packed)) super_block_t;

void hd_init();

void do_hd_request();

hd_t* get_hd_info(u8 dev);

void print_disk_info(hd_t* info);

void init_active_hd_info(u8 dev);
void init_active_hd_partition();

void mount_partition(super_block_t* block);
void unmount_partition();

void print_super_block();

void print_block_bitmap();
void reset_block_bitmap();

void print_inode_bitmap();
void reset_inode_bitmap();

void print_bitmap();
void reset_bitmap();

#endif