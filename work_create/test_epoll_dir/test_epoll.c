#include <sys/epoll.h>
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

#define MAX_EVENTS 10 // return at most 10 fd's event
#define MAX_BUFFER 8
//#define FIFO_NAME  "/tmp/test_epoll_fifo_0"

int main(int argc, char* argv[])
{

    if(2 != argc) {
        printf("Usage: input a FIFO name\n");
        return -1;
    }

    int ret;
    int epollfd, fd, nfds;
    struct epoll_event ev, events[MAX_EVENTS];
    char buf[MAX_BUFFER];

    memset(buf, 0, sizeof(buf));
    fd = open(argv[1], O_RDWR);
    if(-1 == fd) {
        printf("Open file: %s fail. Usage: input a FIFO name\n", argv[1]);
        return -2;
    }


    epollfd = epoll_create1(0);

    if (epollfd == -1) {
        perror("epoll_create1\n");
        exit(EXIT_FAILURE);
    }

    ev.events = EPOLLIN;//EPOLLOUT;//
    ev.data.fd = fd;

    if (-1 == epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev)) {
        perror("epoll_ctl: add fd fail\n");
        exit(EXIT_FAILURE);
    }

    for(int c = 0; c < 10000; c++) {
        nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        if(-1 == nfds) {
            perror("epoll_wait");
            exit(EXIT_FAILURE);
        }
        for(int n = 0; n < nfds; n++) {
            if(fd == events[n].data.fd) {
                if(EPOLLIN == events[n].events) {
                    ret = read(fd, buf, sizeof(buf));
                    if(-1 == ret) {
                        perror("read fd");
                    } else {
                        printf("read out %d bytes: %s\n", ret, buf);
                    }
                    memset(buf, 0, sizeof(buf));
                } else if(EPOLLOUT == events[n].events) {
                    buf[0] = '1';
                    ret = write(fd, buf, sizeof(buf));
                    if(-1 == ret) {
                        perror("read write");
                    } else {
                        printf("write in %d bytes: %s\n", ret, buf);
                    }
                    memset(buf, 0, sizeof(buf));
                } else {
                    printf("Error: get epoll event: 0x%.8x\n", events[n].events);
                }
            }
        }

    }

    //sleep(10);

    close(fd);
    close(epollfd);
    printf("close and exit\n");
    return 0;
}
