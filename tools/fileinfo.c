#include "fileinfo.h"
#include "common.h"

Fileinfo* read_file(const char* filename) {
    if (NULL == filename) {
        return NULL;
    }

    // 1 create object
    Fileinfo* fileinfo = calloc(1, sizeof(Fileinfo));
    if (NULL == fileinfo) {
        perror("calloc fail: ");
        exit(-1);
    }

    fileinfo->name = filename;

    // 2 open a file
    FILE* file = NULL;
    if (NULL == (file = fopen(filename, "rb"))) {
        perror("fopen fail");
        exit(1);
    }

    // 3 get file size
    if (0 != fseek(file, 0, SEEK_END)) {
        perror("fseek fail");
        exit(1);
    }

    fileinfo->size = (int)ftell(file);
    if (-1 == fileinfo->size) {
        perror("ftell fail");
        exit(1);
    }

    // reset file pointer
    fseek(file, 0, SEEK_SET);

    // 4 memory allocation
    fileinfo->content = calloc(1, fileinfo->size);
    if (NULL == fileinfo->content) {
        perror("calloc fail: ");
        exit(-1);
    }

    // 5 read a file
    int readsize = fread(fileinfo->content, sizeof(char), fileinfo->size, file);
    if (readsize != fileinfo->size) {
        perror("fread fail: ");
        exit(-1);
    }

    // 6 close a file
    fclose(file);

    return fileinfo;
}

