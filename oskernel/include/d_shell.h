#ifndef OS_KERNEL_D_SHELL_H
#define OS_KERNEL_D_SHELL_H

#include "linux/types.h"

typedef struct _filepath_parse_data_t {
    u32     data_size;
    char**  data;
} __attribute__((packed)) filepath_parse_data_t;

bool d_shell_is_active();
void active_d_shell();
void close_d_shell();

void run_d_shell(char ch);
void exec_d_shell();
void del_d_shell();

void print_shell_header();

#endif