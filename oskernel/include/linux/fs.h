#ifndef OS_KERNEL_FS_H
#define OS_KERNEL_FS_H

#include "types.h"

#define READ 0
#define WRITE 1
#define READA 2		/* read-ahead - don't pause */
#define WRITEA 3	/* "write-ahead" - silly, but somewhat useful */
#define CHECK 4     // check hdd info

#define O_ACCMODE	   0003
#define O_RDONLY	     00
#define O_WRONLY	     01
#define O_RDWR		     02
#ifndef O_CREAT
# define O_CREAT	   0100	/* Not fcntl.  */
#endif
#ifndef O_EXCL
# define O_EXCL		   0200	/* Not fcntl.  */
#endif
#ifndef O_NOCTTY
# define O_NOCTTY	   0400	/* Not fcntl.  */
#endif
#ifndef O_TRUNC
# define O_TRUNC	  01000	/* Not fcntl.  */
#endif
#ifndef O_APPEND
# define O_APPEND	  02000
#endif
#ifndef O_NONBLOCK
# define O_NONBLOCK	  04000
#endif
#ifndef O_NDELAY
# define O_NDELAY	O_NONBLOCK
#endif
#ifndef O_SYNC
# define O_SYNC	       04010000
#endif
#define O_FSYNC		O_SYNC
#ifndef O_ASYNC
# define O_ASYNC	 020000
#endif
#ifndef __O_LARGEFILE
# define __O_LARGEFILE	0100000
#endif

typedef struct _buffer_head_t {
    char*   data;
    u8      dev;
    uint    sector_from;
    uint    sector_count;
    u8      handler_state;
} __attribute__((packed)) buffer_head_t;

typedef struct _hd_request_t {
    u8  state;                      // 1 valid -1 invalid
    int dev;		                /* -1 if no request */
    int cmd;		                /* READ or WRITE */
    int errors;
    unsigned long sector;
    unsigned long nr_sectors;
    char * buffer;
    buffer_head_t* bh;
    struct request * next;
} __attribute__((packed)) hd_request_t;

typedef struct _d_inode_t {
    unsigned short i_mode;
    unsigned short i_uid;
    unsigned long i_size;
    unsigned long i_time;
    unsigned char i_gid;
    unsigned char i_nlinks;
    unsigned short i_zone[9];
    u8  i_zone_off;
} __attribute__((packed)) d_inode_t;

typedef struct _m_inode_t {
    unsigned short i_mode;
    unsigned short i_uid;
    unsigned long i_size;
    unsigned long i_time;
    unsigned char i_gid;
    unsigned char i_nlinks;
    unsigned short i_zone[9];
    u8  i_zone_off;
} __attribute__((packed)) m_inode_t;

typedef enum {
    FILE_TYPE_REGULAR = 1,
    FILE_TYPE_DIRECTORY
} file_type;

typedef struct _dir_entry_t {
    char name[16];
    ushort inode;
    file_type ft;
    u32 dir_index;
} __attribute__((packed)) dir_entry_t;


typedef struct _file_t {
    unsigned short f_mode;
    unsigned short f_flags;
    unsigned short f_count;
    m_inode_t* f_inode;
    uint f_pos;
    char    name[16];
} __attribute__((packed)) file_t;

void ll_rw_block(int rw, buffer_head_t* bh);

/**
 * send request
 * @param dev   which hdd
 * @param from  start
 * @param count how many sectors
 * @return
 */
buffer_head_t* bread(int dev, int from, int count);

size_t bwrite(int dev, int from, char* buff, int size);

void init_super_block();

void iset(u32 index, bool v);
int iget();

int get_data_sector();
void set_data_sector(u32 index, bool v);

void print_dir_entry(dir_entry_t* entry);

void create_root_dir();
void print_root_dir();

void ls_current_dir();

void create_dir(char* name);
void rm_directory(const char* filepath);
void cd_directory(const char* filepath);

void create_file(char* filename);
void write_file(const char* content, const char* filename);
char* read_file(char* filename);

file_t* kfopen(const char *filename, int flag);
int kfclose(FILE *stream);
size_t kfread(void *ptr, size_t size, FILE *stream);
size_t kfwrite(const void *ptr, size_t size, FILE *stream);

#endif