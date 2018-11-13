/*
*******************************************************************
 * Copyright (C):
 * FileName:
 * Author:        Version :          Date:
 * Description:
 * Version:
 * Function List:
 * History:
      <author>  <time>   <version >   <desc>
      David    96/10/12     1.0     build this moudle
*******************************************************************
*/

#pragma once

#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <sys/time.h>

//printf("%s, %d\n", __FUNCTION__, __LINE__);

void* mcu_chan_open();

int mcu_chan_set(void* handler, void*);

/*timeout the number of milliseconds:
Specifying  a  negative value  in  timeout  means  an infinite timeout.
Specifying a timeout of zero causes poll() to return immediately, even if no file descriptors are ready.
*/
int mcu_chan_read_timewait(void* handler, int timeout);

int mcu_chan_read_trywait(void* handler);

int mcu_chan_write_timewait(void* handler, int timeout);

int mcu_chan_write_trywait(void* handler);

ssize_t mcu_chan_read(void* handler, void *buf, size_t count);
ssize_t mcu_chan_write(void* handler, void *buf, size_t count);

int mcu_chan_close(void* handler);

