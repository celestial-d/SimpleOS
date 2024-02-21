#include "../include/linux/kernel.h"
#include "../include/linux/task.h"
#include "../include/linux/mm.h"

extern void* kernel_thread(void* arg);

extern task_t* current;

task_t* tasks[NR_TASKS] = {0};

static int find_empty_process() {
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

void* task_fun(void* arg) {
    for (int j = 0; j < 10; ++j) {
        printk("%d\n", j);
    }
}

task_t* create_task() {
    task_t* task = get_free_page();

    // init
    task->state = TASK_INIT;

    task->pid = find_empty_process();

    tasks[task->pid] = task;

    task->function = kernel_thread;

    task->sched_times = 0;

    task->state = TASK_READY;

    return task;
}

int get_sched_times(task_t* task) {
    return task->sched_times++;
}

void current_task_exit() {
    for (int i = 0; i < NR_TASKS; ++i) {
        task_t* tmp = tasks[i];

        if (current == tmp) {
            printk("task exit: 0x%08x\n", current);

            // first remove, then check parent
            tasks[i] = NULL;

            current = NULL;

            break;
        }
    }
}

void task_init() {
    create_task();
}