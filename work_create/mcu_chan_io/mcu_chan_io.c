#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <sys/time.h>
#include "mcu_chan_io.h"
#include "mcu_chan_spi_io.h"

void* mcu_chan_open()
{
    return mcu_chan_spi_open();
}

int mcu_chan_set(void* handler, void* param)
{
    return mcu_chan_spi_set(handler, param);
}


int mcu_chan_read_timewait(void* handler, int timeout)
{
    return mcu_chan_spi_read_timewait(handler, timeout);
}

int mcu_chan_read_trywait(void* handler)
{
    return mcu_chan_spi_read_trywait(handler);
}

int mcu_chan_write_timewait(void* handler, int timeout)
{
    return mcu_chan_spi_write_timewait(handler, timeout);
}

int mcu_chan_write_trywait(void* handler)
{
    return mcu_chan_spi_write_trywait(handler);
}

ssize_t mcu_chan_read(void* handler, void *buf, size_t count)
{

    return mcu_chan_spi_read(handler, buf, count);
}
ssize_t mcu_chan_write(void* handler, void *buf, size_t count)
{
    return mcu_chan_spi_write(handler, buf, count);
}

int mcu_chan_close(void* handler)
{
    return mcu_chan_spi_close(handler);
}

