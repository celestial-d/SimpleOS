#include "../include/linux/sched.h"
#include "../include/linux/task.h"

extern void switch_task(task_t* task);
extern void switch_idle_task(task_t* task);

extern task_t* tasks[NR_TASKS];

task_t* current = NULL;

task_t* find_ready_task() {
    task_t *next = NULL;

    // check whether all tasks are scheduled
    bool is_all_zero = true;
    bool is_null = true;

    for (int i = 1; i < NR_TASKS; ++i) {
        task_t *task = tasks[i];

        if (NULL == task) continue;

        is_null = false;

        if (0 != task->counter) {
            is_all_zero = false;
            break;
        }
    }

    // if not task，is_all_zero should be true
    if (!is_null && is_all_zero) goto reset_task;

    // find out the scheduled task
    for (int i = 1; i < NR_TASKS; ++i) {
        task_t *task = tasks[i];

        if (NULL == task) {
            continue;
        }

        if (current == task && TASK_RUNNING == task->state) {
            task->state = TASK_READY;
        }

        if (TASK_READY != task->state) continue;

        if (NULL == next) {
            next = task;
        } else {
            if (task->counter > next->counter) {
                next = task;
            }
        }
    }

    if (NULL == next) {
        next = tasks[0];
    }

    return next;

    /**
     * if all counter == 0, means scheduled before
     */
reset_task:
    if (is_all_zero) {
        for (int i = 1; i < NR_TASKS; ++i) {
            task_t *tmp = tasks[i];

            if (NULL == tmp) continue;

            tmp->counter = tmp->priority;
        }

        return find_ready_task();
    }
}

void sched() {
    if (NULL != current) {
        if (TASK_SLEEPING != current->state) {
            current->state = TASK_READY;
        }

        current = NULL;
    }

    task_t* next = find_ready_task();

    next->state = TASK_RUNNING;

    current = next;

    switch_task(next);
}

void do_timer() {
    if (NULL == current) {
        sched();
    }

    if((NULL != current) && (current->counter > 0)) {
        current->counter--;
        return;
    }

    sched();
}