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
*******************************************************************
*/

#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <sys/time.h>



/*************************************************
@brief different platform should apply diffent param function with different parametes;
       for spi here, no param input, and NULL means using default setting.
       for example:
       int mch_chan_uart_param(int speed,int flow_ctrl,int databits,int stopbits,int parity);
@param
@return
*************************************************/
void* mch_chan_spi_param();


/*************************************************
@brief open the access channel of spi;
       when access done, should call close function.
@param
@return if ok, return the handle of mcu access channel.
        if fail, return NULL.
*************************************************/
void* mcu_chan_spi_open();


/*************************************************
@brief setup parameters for access mode
@param param:
       NULL for default;
       others for special platform.
@return if ok, return 0.
        if fail, return a negative value.
*************************************************/
int mcu_chan_spi_set(void* handler, void* param); /*should handle the case of param is NULL */


/*************************************************
@brief wait until spi can read;
@param timeout:
       wait time in milliseconds;
       a negative value means an infinite wait.
       a timeout of 0 causes return immediately, even if no read data ready.
@return if ok, return 0.
        if fail, return a negative value.
*************************************************/
int mcu_chan_spi_read_timewait(void* handler, int timeout);

int mcu_chan_spi_read_trywait(void* handler);


/*************************************************
@brief wait until spi can write;
@param timeout:
       wait time in milliseconds;
       a negative value means an infinite wait.
       a timeout of 0 causes return immediately, even if no read data ready.
@return if ok, return 0.
        if fail, return a negative value.
*************************************************/
int mcu_chan_spi_write_timewait(void* handler, int timeout);

int mcu_chan_spi_write_trywait(void* handler);


/*************************************************
@brief read spi channel;
@param buf: data buffer for read;
       count: read up to count bytes;
@return if ok, the number of bytes read.
        if fail, return -1.
*************************************************/
ssize_t mcu_chan_spi_read(void* handler, void *buf, size_t count);


/*************************************************
@brief write mcu channel;
@param buf: data buffer for write;
       count: write up to count bytes;
@return if ok, the number of bytes write.
        if fail, return -1.
*************************************************/
ssize_t mcu_chan_spi_write(void* handler, void *buf, size_t count);


/*************************************************
@brief close the access channel of spi;
@param
@return if ok, return the handle of mcu access channel.
        if fail, return NULL.
*************************************************/
int mcu_chan_spi_close(void* handler);

#ifdef __cplusplus
}
#endif   /* end of __cplusplus */
