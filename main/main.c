#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "lib/vfat/vfat_interface.h"

int main(void)
{
    int fd;

    fd = open("/dev/mmcblk0", O_RDWR);
    if(fd == -1) {
        perror("device open is failed...\n");
        exit(-1);
    }

    vfat_parse_mbr(fd);

    close(fd);

    return 0;
}
