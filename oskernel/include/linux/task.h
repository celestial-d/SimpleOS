#ifndef OS_KERNEL_TASK_H
#define OS_KERNEL_TASK_H

#include "types.h"
#include "mm.h"

// upper bound of processes
#define NR_TASKS 64

typedef void* (*task_fun_t)(void*);

typedef enum task_state_t {
    TASK_INIT,     // init
    TASK_RUNNING,  // exec
    TASK_READY,    // ready
    TASK_BLOCKED,  // block
    TASK_SLEEPING, // sleep
    TASK_WAITING,  // wait
    TASK_DIED,     // died
} task_state_t;

typedef struct tss_t {
    u32 backlink; // prev task
    u32 esp0;     // ring0
    u32 ss0;      // ring0
    u32 esp1;     // ring1
    u32 ss1;      // ring1
    u32 esp2;     // ring2
    u32 ss2;      // ring2
    u32 cr3;
    u32 eip;
    u32 flags;
    u32 eax;
    u32 ecx;
    u32 edx;
    u32 ebx;
    u32 esp;
    u32 ebp;
    u32 esi;
    u32 edi;
    u32 es;
    u32 cs;
    u32 ss;
    u32 ds;
    u32 fs;
    u32 gs;
    u32 ldtr;
    u16 trace : 1;
    u16 reversed : 15;
    u16 iobase;
    u32 ssp;
} __attribute__((packed)) tss_t;

typedef struct task_t {
    int             pid;
    task_fun_t      function;
    task_state_t    state;

    tss_t           context;

    int             sched_times;

    int             cr3;
}task_t;

typedef union task_union_t {
    task_t task;
    char stack[PAGE_SIZE];
}task_union_t;

task_t* create_task();

void task_init();

int get_sched_times(task_t* task);

void current_task_exit();
#endif