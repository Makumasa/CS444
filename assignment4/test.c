#include <stdio.h>
#include <sys/syscall.h>
#include <stdlib.h>

#define MEM_SIZE 359
#define MEM_USED 360

int main() {
        unsigned char *buf0, *buf1, *buf2, *buf3, *buf4, *buf5, *buf6, *buf7;

        buf0 = malloc(1234);
        buf1 = malloc(8192);
        free(buf0);
        buf2 = malloc(4096);
        buf3 = malloc(512);
        buf4 = malloc(4096);
        free(buf3);
        buf5 = malloc(256);
        buf6 = malloc(1048576);
        free(buf1);
        free(buf4);
        buf7 = malloc(2048);

        long size = (long)syscall(MEM_SIZE);
        long used = (long)syscall(MEM_USED);

        printf("Memory size: %l; memory used: %l\n", size, used);
        free(buf2);
        free(buf5);
        free(buf6);
        free(buf7);

        return 0;
}
