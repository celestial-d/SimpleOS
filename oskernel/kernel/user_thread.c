#include "../include/unistd.h"
#include "../include/stdio.h"

// from shell
void fs_test() {
    printf("start test file system\n");

    FILE* file = fopen("zz", "r");
    printf("file = 0x%x\n", file);

    char buff[512] = {0};
    fread(buff, 512, file);
    printf("read data: %s\n", buff);

    fwrite("hello", 5, file);

    fread(buff, 512, file);
    printf("read data: %s\n", buff);

    fclose(file);

    return;
}

void user_mode() {
    printf("enter user mode..\n");

    uactive_d_shell();

    while (true);
}