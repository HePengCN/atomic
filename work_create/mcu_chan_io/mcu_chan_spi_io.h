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


/*different param function for different IO device,
for example:
int mch_chan_uart_param(int speed,int flow_ctrl,int databits,int stopbits,int parity);
*/
void* mch_chan_spi_param();

void* mcu_chan_spi_open();

int mcu_chan_spi_set(void* handler, void* param); /*should handle the case of param is NULL */

int mcu_chan_spi_read_timewait(void* handler, int timeout);

int mcu_chan_spi_read_trywait(void* handler);

int mcu_chan_spi_write_timewait(void* handler, int timeout);

int mcu_chan_spi_write_trywait(void* handler);

ssize_t mcu_chan_spi_read(void* handler, void *buf, size_t count);

ssize_t mcu_chan_spi_write(void* handler, void *buf, size_t count);

int mcu_chan_spi_close(void* handler);

