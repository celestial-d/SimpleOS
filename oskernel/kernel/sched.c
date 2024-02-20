#include "../include/linux/sched.h"
#include "../include/linux/task.h"
#include "../include/linux/kernel.h"
#include "../include/asm/system.h"

extern void switch_task(task_t* task);

extern task_t* tasks[NR_TASKS];

task_t* current = NULL;

task_t* find_ready_task() {
    task_t* next = NULL;

    for (int i = 0; i < NR_TASKS; ++i) {
        task_t* task = tasks[i];

        if (NULL == task) continue;
        if (TASK_READY != task->state) continue;

        next = task;
    }

    return next;
}

void sched() {
    if (NULL != current) {
        if (TASK_SLEEPING != current->state) {
            current->state = TASK_READY;
        }

        current = NULL;
    }

    task_t* next = find_ready_task();

    if (NULL == next) {
        printk("no task..\n");

        while (true);
    }

    next->state = TASK_RUNNING;

    current = next;

    switch_task(next);
}