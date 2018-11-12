#include <sys/select.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define TEST_READ
//#define TEST_WRITE

//#define MAX_FDS 1
#define MAX_BUFFER 8
//#define FIFO_NAME  "/tmp/test_epoll_fifo_0"

int main(int argc, char* argv[])
{

    if(2 != argc) {
        printf("Usage: input a FIFO name\n");
        return -1;
    }

    int ret;
    int fd, nfds;
    fd_set readfds, writefds;
    char buf[MAX_BUFFER];

    memset(buf, 0, sizeof(buf));
    fd = open(argv[1], O_RDWR);
    if(-1 == fd) {
        printf("Open file: %s fail. Usage: input a FIFO name\n", argv[1]);
        return -2;
    }


    for(int c = 0; c < 10000; c++) {

        FD_ZERO(&readfds);
        FD_ZERO(&writefds);
        FD_SET(fd, &readfds);
        FD_SET(fd, &writefds);

#ifdef TEST_READ
        nfds = select(FD_SETSIZE, &readfds, NULL, NULL, NULL);
#else
        nfds = select(FD_SETSIZE, NULL, &writefds, NULL, NULL);
#endif
        if (-1 == nfds) {
            perror("select error\n");
            exit(EXIT_FAILURE);
        }

        for(int n = 0; n < nfds; n++) {
#ifdef TEST_READ
            if(FD_ISSET(fd, &readfds)) {
                ret = read(fd, buf, sizeof(buf));
                if(-1 == ret) {
                    perror("read fd");
                } else {
                    printf("read out %d bytes: %s\n", ret, buf);
                }
                memset(buf, 0, sizeof(buf));
            }
#else

            if(FD_ISSET(fd, &writefds)) {
                buf[0] = '1';
                ret = write(fd, buf, sizeof(buf));
                if(-1 == ret) {
                    perror("read write");
                } else {
                    printf("write in %d bytes: %s\n", ret, buf);
                }
                memset(buf, 0, sizeof(buf));
            }
#endif
        }

    }

    //sleep(10);

    close(fd);
    printf("close and exit\n");
    return 0;
}
