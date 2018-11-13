#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <poll.h>
#include "mcu_chan_spi_io.h"

//#define MCU_CHAN_SPI_DEVNAME "/dev/spidev2.0"
#define MCU_CHAN_SPI_DEVNAME "/tmp/fifo_fake_mcu_chan"

typedef struct mc_handler {
    int fd;
} mc_handler_t;

/*different param function for different IO device*/
void* mch_chan_spi_param()
{
    return NULL;
}

void* mcu_chan_spi_open()
{
    int fd = open(MCU_CHAN_SPI_DEVNAME, O_RDWR);
    if(-1 == fd) {
        fprintf(stderr, "%s(%d): open %s fail, Error msg: %s\n", __FUNCTION__, __LINE__, MCU_CHAN_SPI_DEVNAME, strerror(errno));
        return NULL;
    }
    mc_handler_t* hd = (mc_handler_t*)malloc(sizeof(mc_handler_t));
    if(NULL == hd) {
        fprintf(stderr, "%s(%d): malloc fail, Error msg: %s\n", __FUNCTION__, __LINE__, strerror(errno));
        return NULL;
    }
    hd->fd = fd;
    return hd;
}

int mcu_chan_spi_set(void* handler, void* param)
{
    assert(NULL != handler);
    int ret = 0, val = 0;
    mc_handler_t* hd = (mc_handler_t*)handler;
    (void)param;

    val = fcntl(hd->fd, F_GETFL);
    if (-1 == val) {
        fprintf(stderr, "%s(%d): fcntl F_GETFL fail, Error msg: %s\n", __FUNCTION__, __LINE__, strerror(errno));
        return -1;
    }

    /*+ Asynchronous IO +*/
    #if 0
    ret = fcntl(hd->fd, F_SETOWN, getpid());
    if (-1 == ret) {
        fprintf(stderr, "%s(%d): fcntl F_SETOWN(%d) fail, Error msg: %s\n", __FUNCTION__, __LINE__, getpid(), strerror(errno));
        return -2;
    }

    val |= FASYNC;
    #endif
    /*- Asynchronous IO -*/
    /*+ Blocking IO +*/

    if(val & O_NONBLOCK) {
        val &= ~O_NONBLOCK;
    }

    /*- Blocking IO -*/

    ret = fcntl(hd->fd, F_SETFL, val);
    if (ret == -1) {
        fprintf(stderr, "%s(%d): fcntl F_SETFL fail, Error msg: %s\n", __FUNCTION__, __LINE__, strerror(errno));
        return -3;
    }

    return 0;
}

int mcu_chan_spi_read_timewait(void* handler, int timeout)
{
    assert(NULL != handler);
    mc_handler_t* hd = (mc_handler_t*)handler;
    int nfds;
    struct pollfd fds[1];
    fds[0].fd = hd->fd;
    fds[0].events = POLLIN;
    nfds = poll(fds, sizeof(fds)/sizeof(struct pollfd), timeout);
    if (1 != nfds) {
        fprintf(stderr, "%s(%d): poll fail, return %d, Error msg: %s\n", __FUNCTION__, __LINE__, nfds, strerror(errno));
        return -1;
    }

    if(POLLIN != fds[0].revents) {
        fprintf(stderr, "%s(%d): poll wait event is NOT POLLIN, Error msg: %s\n", __FUNCTION__, __LINE__, strerror(errno));
        return -2;
    }

    return 0;
}

int mcu_chan_spi_read_trywait(void* handler)
{
    return mcu_chan_spi_read_timewait(handler, 0);
}

int mcu_chan_spi_write_timewait(void* handler, int timeout)
{
    assert(NULL != handler);
    mc_handler_t* hd = (mc_handler_t*)handler;
    int nfds;
    struct pollfd fds[1];
    fds[0].fd = hd->fd;
    fds[0].events = POLLOUT;
    nfds = poll(fds, sizeof(fds)/sizeof(struct pollfd), timeout);
    if (1 != nfds) {
        fprintf(stderr, "%s(%d): poll fail, return %d, Error msg: %s\n", __FUNCTION__, __LINE__, nfds, strerror(errno));
        return -1;
    }

    if(POLLOUT != fds[0].revents) {
        fprintf(stderr, "%s(%d): poll wait event is NOT POLLOUT, Error msg: %s\n", __FUNCTION__, __LINE__, strerror(errno));
        return -2;
    }

    return 0;
}

int mcu_chan_spi_write_trywait(void* handler)
{
    return mcu_chan_spi_write_timewait(handler, 0);
}

ssize_t mcu_chan_spi_read(void* handler, void *buf, size_t count)
{
    assert(NULL != handler);
    mc_handler_t* hd = (mc_handler_t*)handler;
    return read(hd->fd, buf, count);
}

ssize_t mcu_chan_spi_write(void* handler, void *buf, size_t count)
{
    assert(NULL != handler);
    mc_handler_t* hd = (mc_handler_t*)handler;
    return write(hd->fd, buf, count);
}

int mcu_chan_spi_close(void* handler)
{
    assert(NULL != handler);
    mc_handler_t* hd = (mc_handler_t*)handler;
    close(hd->fd);
    free(hd);
}

