#ifndef OS_KERNEL_STDARG_H
#define OS_KERNEL_STDARG_H

typedef char* va_list;

#define va_start(p, count) (p = (va_list)&count + sizeof(char*))
// 1、revise p_args; 2、get value
#define va_arg(p, t) (*(t*)((p += sizeof(char*)) - sizeof(char*)))
#define va_end(p) (p = 0)

#endif