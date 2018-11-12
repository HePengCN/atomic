#include <poll.h>
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

#define MAX_FDS 1
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
    struct pollfd fds[MAX_FDS];
    char buf[MAX_BUFFER];

    memset(buf, 0, sizeof(buf));
    fd = open(argv[1], O_RDWR);
    if(-1 == fd) {
        printf("Open file: %s fail. Usage: input a FIFO name\n", argv[1]);
        return -2;
    }

    fds[0].fd = fd;
    fds[0].events = POLLOUT;//POLLIN; //POLLOUT

    for(int c = 0; c < 10000; c++) {

        nfds = poll(fds, sizeof(fds)/sizeof(struct pollfd), -1);

        if (-1 == nfds) {
            perror("poll error\n");
            exit(EXIT_FAILURE);
        }

        for(int n = 0; n < nfds; n++) {
            if(fd == fds[n].fd) {
                if(POLLIN == fds[n].revents) {
                    ret = read(fd, buf, sizeof(buf));
                    if(-1 == ret) {
                        perror("read fd");
                    } else {
                        printf("read out %d bytes: %s\n", ret, buf);
                    }
                    memset(buf, 0, sizeof(buf));
                } else if(POLLOUT == fds[n].revents) {
                    buf[0] = '1';
                    ret = write(fd, buf, sizeof(buf));
                    if(-1 == ret) {
                        perror("read write");
                    } else {
                        printf("write in %d bytes: %s\n", ret, buf);
                    }
                    memset(buf, 0, sizeof(buf));
                } else {
                    printf("Error: get poll event: 0x%.8x\n", fds[n].revents);
                }
            }
        }

    }

    //sleep(10);

    close(fd);
    printf("close and exit\n");
    return 0;
}
