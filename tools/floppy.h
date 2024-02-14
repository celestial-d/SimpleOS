#ifndef ZIYA_OS_KERNEL_FLOPPY_H
#define ZIYA_OS_KERNEL_FLOPPY_H

#include "fileinfo.h"

typedef struct {
    int     size;
    char*   content;
}Floppy;

Floppy* create_floppy();

void write_bootloader(Floppy* floppy, Fileinfo* fileinfo);

/**
 *
 * @param floppy
 * @param str
 * @param face
 * @param track
 * @param section
 */
void write_floppy(Floppy* floppy, char* str, int face, int track, int section);

void write_floppy_fileinfo(Floppy* floppy, Fileinfo* fileinfo, int face, int track, int section);

/**
 * @param name
 * @param floppy
 */
void create_image(const char* name, Floppy* floppy);

#endif //ZIYA_OS_KERNEL_FLOPPY_H