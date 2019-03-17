#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define FIB_DEV "/dev/fibonacci"

int main()
{
    int fd;
    long long sz;

    char buf[16] = {0};
    char write_buf[] = "testing writing";
    int offset = 100;  // TODO: test something bigger than the limit
    int i = 0;

    struct timespec t1, t2;

    fd = open(FIB_DEV, O_RDWR);

    if (fd < 0) {
        perror("Failed to open character device");
        exit(1);
    }

    for (i = 0; i <= offset; i++) {
        sz = write(fd, write_buf, strlen(write_buf));
        printf("Writing to " FIB_DEV ", returned the sequence %lld\n", sz);
    }

    for (i = 0; i <= offset; i++) {
        lseek(fd, i, SEEK_SET);
        memset(buf, 0, 16);
        memcpy(buf, "fast", 4);
        clock_gettime(CLOCK_REALTIME, &t1);
        sz = read(fd, buf, 16);
        clock_gettime(CLOCK_REALTIME, &t2);
        printf("(fast)Reading from " FIB_DEV
               " at offset %d, returned the sequence "
               "%llu + (%d * 18446744073709551616).\n",
               i, sz, buf[8]);
        printf("Time: %ld %ld\n", (t1.tv_sec - t2.tv_sec),
               (t1.tv_nsec - t2.tv_nsec));

    }

    for (i = offset; i >= 0; i--) {
        lseek(fd, i, SEEK_SET);
        clock_gettime(CLOCK_REALTIME, &t1);
        sz = read(fd, buf, 16);
        clock_gettime(CLOCK_REALTIME, &t2);
        printf("(Regular)Reading from " FIB_DEV
               " at offset %d, returned the sequence "
               "%llu + (%d * 18446744073709551616).\n",
               i, sz, buf[8]);
        printf("Time: %ld %ld\n", (t1.tv_sec - t2.tv_sec),
               (t1.tv_nsec - t2.tv_nsec));
    }

    close(fd);
    return 0;
}
