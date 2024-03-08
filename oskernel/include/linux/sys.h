#ifndef OS_KERNEL_SYS_H
#define OS_KERNEL_SYS_H

#include "types.h"
#include "fs.h"

extern ssize_t sys_write(int fd, const void *buf, size_t count);
extern int sys_exit(int status);
extern int sys_fork();
extern pid_t sys_get_pid();
extern pid_t sys_get_ppid();

int sys_open(const char* pathname, int flags);
void sys_active_d_shell();
file_t* sys_fopen(const char *filename, const char *mode);
int sys_fclose(FILE *stream);
size_t sys_fread(void *ptr, size_t size, FILE *stream);
size_t sys_fwrite(const void *ptr, size_t size, FILE *stream);
#endif