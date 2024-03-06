#ifndef OS_KERNEL_TASK_H
#define OS_KERNEL_TASK_H

#include "types.h"
#include "mm.h"
#include "fs.h"
// upper bound of processes
#define NR_TASKS 64
#define NR_OPEN 16

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
    tss_t           tss;
    int             pid;
    int             ppid;
    char            name[32];
    task_state_t    state;
    int             exit_code;
    int             counter;
    int             priority;
    int             scheduling_times;
    int             esp0;                   // esp3 saved in tss
    int             ebp0;
    int             esp3;
    int             ebp3;

    bool            resume_from_irq;

    file_t*         file_descriptor[NR_OPEN];

    dir_entry_t*    current_active_dir;     // related to ""pwd
    m_inode_t*      current_active_dir_inode;

    dir_entry_t*    root_dir;
    m_inode_t*      root_dir_inode;

    int             magic;

}task_t;

typedef union task_union_t {
    task_t task;
    char stack[PAGE_SIZE];
}task_union_t;

task_t* create_task(char* name, task_fun_t fun, int priority);
void task_init();

void task_exit(int code, task_t* task);
void current_task_exit(int code);

void task_sleep(int ms);
void task_wakeup();

int inc_scheduling_times(task_t* task);

pid_t get_task_pid(task_t* task);
pid_t get_task_ppid(task_t* task);

task_t* create_child(char* name, task_fun_t fun, int priority);

int get_esp3(task_t* task);
void set_esp3(task_t* task, int esp);

void task_block(task_t* task);
void task_unblock(task_t* task);

void set_block(task_t* task);
bool is_blocked(task_t* task);

int find_empty_file_descriptor();

#endif