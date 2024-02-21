#include "../include/linux/kernel.h"
#include "../include/linux/task.h"
#include "../include/linux/mm.h"
#include "../include/linux/sched.h"
#include "../include/string.h"
#include "../include/asm/system.h"

extern void sched_task();

extern task_t* current;
extern int jiffy;
extern int cpu_tickes;

task_t* tasks[NR_TASKS] = {0};

int find_empty_process() {
    int ret = 0;

    bool is_finded = false;

    for (int i = 0; i < NR_TASKS; ++i) {
        if (0 == tasks[i]) {
            is_finded = true;

            ret = i;
            break;
        }
    }

    if (!is_finded) {
        printk("no valid pid\n");
        return -1;
    }

    return ret;
}

task_t* create_task(char* name, task_fun_t fun, int priority) {
    task_union_t* task = (task_union_t*)kmalloc(4096);
    memset(task, 0, PAGE_SIZE);

    task->task.pid = find_empty_process();
    task->task.ppid = (NULL == current)? 0 : current->pid;

    task->task.priority = priority;
    task->task.counter = priority;
    task->task.scheduling_times = 0;

    strcpy(task->task.name, name);

    // add to tasks
    tasks[task->task.pid] = &(task->task);

    task->task.tss.cr3 = (int)task + sizeof(task_t);
    task->task.tss.eip = fun;

    // r0 stack
    task->task.esp0 = (int)task + PAGE_SIZE;
    task->task.ebp0 = task->task.esp0;

    task->task.tss.esp0 = task->task.esp0;

    task->task.state = TASK_READY;

    return task;
}

void* t1_fun(void* arg) {
    printk("t1: %d\n", 1);
}

void* t2_fun(void* arg) {
    printk("t2: %d\n", 2);
    task_sleep(500);
}

void* t3_fun(void* arg) {
    printk("t3: %d\n", 3);
    task_sleep(300);
}

void* idle(void* arg) {
    create_task("t1", t1_fun, 4);
    create_task("t2", t2_fun, 2);
    create_task("t3", t3_fun, 3);

    while (true) {
        printk("idle task running...\n");
        __asm__ volatile ("sti;");
        __asm__ volatile ("hlt;");
    }
}

void task_init() {
    create_task("idle", idle, 0);
}

int inc_scheduling_times(task_t* task) {
    return task->scheduling_times++;
}

pid_t get_task_pid(task_t* task) {
    return task->pid;
}

pid_t get_task_ppid(task_t* task) {
    return task->ppid;
}

void task_exit(int code, task_t* task) {
    for (int i = 1; i < NR_TASKS; ++i) {
        task_t* tmp = tasks[i];

        if (task == tmp) {
            printk("task exit: %s\n", tmp->name);

            tmp->exit_code = code;

            // first remove, then deal with parent
            tasks[i] = NULL;

            current = NULL;

            kfree_s(tmp, 4096);

            break;
        }
    }
}

void current_task_exit(int code) {
    for (int i = 1; i < NR_TASKS; ++i) {
        task_t* tmp = tasks[i];

        if (current == tmp) {
            printk("task exit: %s\n", tmp->name);

            tmp->exit_code = code;

            // first remove, then deal with parent
            tasks[i] = NULL;

            current = NULL;

            break;
        }
    }
}

void task_sleep(int ms) {
    CLI

    if (NULL == current) {
        printk("task sleep: current task is null\n");
        return;
    }

    int ticks = ms / jiffy;
    ticks += (0 == ms % jiffy)? 0 : 1;

    current->counter = cpu_tickes + ticks;
    current->state = TASK_SLEEPING;

    sched_task();
}

void task_wakeup() {
    for (int i = 1; i < NR_TASKS; ++i) {
        task_t* task = tasks[i];

        if (NULL == task) continue;
        if (TASK_SLEEPING != task->state) continue;

        if (cpu_tickes >= task->counter) {
            task->state = TASK_READY;
            task->counter = task->priority;
        }
    }
}