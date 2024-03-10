#include "../include/linux/kernel.h"
#include "../include/linux/hd.h"
#include "../include/linux/fs.h"
#include "../include/linux/mm.h"
#include "../include/linux/task.h"
#include "../include/d_shell.h"
#include "../include/string.h"
#include "../include/assert.h"

extern void fs_test();
extern void test_page_fault(const char* param);

extern task_t* current;

bool g_active_shell = false;

// save input from keyboard
ushort g_shell_command_off = 0;
char g_shell_command[64] = {0};

static char* shell_functions[] = {
        "h | help: view all features",
        "ls | ls dirname: list directory contents. example: ls /",
        "cd dirname: enter the directory. example: cd /",
        "mkdir dirname: create directory. example: mkdir ziya",
        "touch filename: create file. example: touch ziya.exe",
        "rm [dirname|filename]: delete dir or file. example: rm /ziya",
        "echo: writes data to a file. example: echo ziya >> ziya.exe",
        "cat filname: view file content. example: cat ziya.exe",
        "print_super_block: view super block info",
        "print_root_dir: view root dir info",
        "print_bitmap: view block bitmap and inode bitmap",
        "test_pf [1|2]: trigger memory error and repair"
};

static void print_shell_functions() {
    for (int i = 0; i < sizeof(shell_functions) / sizeof(char*); ++i) {
        printk("  %s\n", shell_functions[i]);
    }
}

bool d_shell_is_active() {
    return true == g_active_shell;
}

void active_d_shell() {
    g_active_shell = true;

    printk("d shell activated! All functions are:\n");

    print_shell_functions();

    printk("[d-shell:%s]:", current->current_active_dir->name);
}

void close_d_shell() {
    g_active_shell = false;
}

void run_d_shell(char ch) {
    if (!g_active_shell) return;

    if (g_shell_command_off >= 64) {
        panic("The command length exceeds 64!!!!");
    }

    g_shell_command[g_shell_command_off++] = ch;
}

static int shell_command_size() {
    int size = 0;

    int off = 0;
    char ch;
    while ('\0' != (ch = g_shell_command[off++])) {
        if (' ' == ch) {
            size++;

            //considering " "
            while (' ' == g_shell_command[off]) {
                off++;
            }
        }
    }

    return size;
}


static pchar* parse_shell_command(OUT int* arr_len) {
    pchar* ret = kmalloc(shell_command_size() * sizeof(pchar));

    int size = 0;
    int off = 0;
    int command_len = 0;
    int command_start = 0;
    char ch;

    while ('\0' != (ch = g_shell_command[off++])) {
        if (' ' != ch) {
            command_len++;
        }

        if ('\0' == g_shell_command[off]) {
            pchar str = kmalloc(command_len + 1);
            memset(str, 0, command_len + 1);

            memcpy(str, &g_shell_command[command_start], command_len);

            ret[size++] = str;

            break;
        }

        if (' ' == ch) {
            pchar str = kmalloc(command_len + 1);
            memset(str, 0, command_len + 1);

            memcpy(str, &g_shell_command[command_start], command_len);

            ret[size++] = str;

            command_len = 0;
            command_start = off;

            while (' ' == g_shell_command[off]) {
                command_start++;

                off++;
            }
        }
    }

    if (NULL != arr_len) {
        *arr_len = size;
    }

    return ret;
}

void exec_d_shell() {
    if (!g_active_shell || 0 == g_shell_command_off) {
        goto end;
    }

    // annotate input
    int commands_len = 0;
    pchar *commands = parse_shell_command(&commands_len);

    // clean
    memset(g_shell_command, 0, 64);
    g_shell_command_off = 0;

    if (!strcmp("h", commands[0]) || !strcmp("help", commands[0])) {
        print_shell_functions();
    } else if (!strcmp("print_super_block", commands[0])) {
        print_super_block();
    } else if (!strcmp("print_block_bitmap", commands[0])) {
        print_block_bitmap();
    } else if (!strcmp("reset_block_bitmap", commands[0])) {
        reset_block_bitmap();
    } else if (!strcmp("print_inode_bitmap", commands[0])) {
        print_inode_bitmap();
    } else if (!strcmp("reset_inode_bitmap", commands[0])) {
        reset_inode_bitmap();
    } else if (!strcmp("reset_bitmap", commands[0])) {
        reset_bitmap();
    } else if (!strcmp("print_bitmap", commands[0])) {
        print_bitmap();
    } else if (!strcmp("print_root_dir", commands[0])) {
        print_root_dir();
    } else if (!strcmp("ls", commands[0])) {
        ls_current_dir();
    } else if (!strcmp("mkdir", commands[0])) {
        create_dir(commands[1]);
    } else if (!strcmp("rm", commands[0])) {
        rm_directory(commands[1]);
    } else if (!strcmp("cd", commands[0])) {
        cd_directory(commands[1]);
    } else if (!strcmp("touch", commands[0])) {
        create_file(commands[1]);
    } else if (!strcmp("echo", commands[0])) {
        write_file(commands[1], commands[3]);
    } else if (!strcmp("cat", commands[0])) {
        char *data = read_file(commands[1]);
        INFO_PRINT("%s\n", data);
    } else if (!strcmp("test_pf", commands[0])) {
        test_page_fault(commands[1]);
    } else {
        for (int i = 0; i < commands_len; ++i) {
            printk("%s\n", commands[i]);
        }
        printk("\n");
    }

    g_shell_command_off = 0;
    memset(g_shell_command, 0, 64);

    // free
    for (int i = 0; i < commands_len; ++i) {
        kfree_s(commands[i], strlen(commands[i]));
    }

end:
    printk("[d-shell:%s]:", current->current_active_dir->name);
}


void del_d_shell() {
    if (!g_active_shell) return;

    g_shell_command[--g_shell_command_off] = 0;
}

void print_shell_header() {
    printk("[d-shell:%s]:", current->current_active_dir->name);
}