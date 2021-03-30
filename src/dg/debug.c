#include "dg/debug.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>


int __file_fd = -1;

int dg_open(const char* path) {
    return (__file_fd = open(path, O_WRONLY | O_CREAT)) != -1;
}

int dg_close() {
    if (close(__file_fd) == -1)
        return -1;

    __file_fd = -1;
    return 0;
}

void __dg_print(const char* level, const char* msg) {
    if (__file_fd == -1)
        return;

    dprintf(__file_fd, "%s: %s\n", level, msg);
}

void dg_log(const char* msg) {
    __dg_print("\033[32mLOG\033[0m    ", msg);
}

void dg_wrn(const char* msg) {
    __dg_print("\033[33mWARNING\033[0m", msg);
}

void dg_err(const char* msg) {
    __dg_print("\033[31mERROR\033[0m  ", msg);
}

