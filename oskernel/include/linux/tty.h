#ifndef OS_KERNEL_TTY_H
#define OS_KERNEL_TTY_H

#include "types.h"

void console_init(void);
int console_write(char *buf, u32 count);

#endif