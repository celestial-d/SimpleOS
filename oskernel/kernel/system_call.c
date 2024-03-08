#include "../include/linux/kernel.h"
#include "../include/linux/tty.h"
#include "../include/linux/sys.h"
#include "../include/linux/task.h"
#include "../include/linux/sched.h"
#include "../include/d_shell.h"
#include "../include/string.h"

#define SYSTEM_CALL_TABLE_SIZE  64

extern task_t* current;

void* system_call_table[SYSTEM_CALL_TABLE_SIZE] = {
        sys_write, sys_exit, sys_fork, sys_get_pid, sys_get_ppid, sys_active_ya_shell,
        sys_fopen, sys_fclose, sys_fread, sys_fwrite
};

ssize_t sys_write(int fd, const void *buf, size_t count) {
    return console_write((char*)buf, count);
}

int sys_exit(int status) {
    current_task_exit(status);
    sched();

    return 0;
}

pid_t sys_get_pid() {
    return get_task_pid(current);
}

pid_t sys_get_ppid() {
    return get_task_ppid(current);
}

void sys_active_ya_shell() {
    active_d_shell();
}

file_t* sys_fopen(const char *filename, const char *mode) {
    int flag = O_RDONLY;

    // mode to flag

    return kfopen(filename, flag);
}

int sys_fclose(FILE *stream) {
    if (NULL == stream) {
        return -1;
    }

    return kfclose(stream);
}

size_t sys_fread(void *ptr, size_t size, FILE *stream) {
    if (NULL == ptr || 0 == size || NULL == stream) {
        return -1;
    }

    memset(ptr, 0, size);

    return kfread(ptr, size, stream);
}

size_t sys_fwrite(const void *ptr, size_t size, FILE *stream) {
    if (NULL == ptr || 0 == size || NULL == stream) {
        return -1;
    }

    return kfwrite(ptr, size, stream);
}