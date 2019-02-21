/********************************************************************
 * Copyright (C):
 * FileName: mcu_chan_io.h
 * Author:        Version :          Date:
 * Description:
 * Version:
 * Function List:
 * History:
      <author>  <time>   <version >   <desc>
********************************************************************/

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

typedef void* hmcuchan_t;

/*************************************************
@brief open the access channel of mcu;
       when access done, should call close function.
@param
@return if ok, return the handle of mcu access channel.
        if fail, return NULL.
*************************************************/
hmcuchan_t mcu_chan_open();


/*************************************************
@brief setup parameters for access mode
@param param:
       NULL for default;
       others for special platform.
@return if ok, return 0.
        if fail, return a negative value.
*************************************************/
int mcu_chan_set(hmcuchan_t handler, void* param);


/*************************************************
@brief wait until mcu channel can read;
@param timeout:
       wait time in milliseconds;
       a negative value means an infinite wait.
       a timeout of 0 causes return immediately, even if no read data ready.
@return if ok, return 0.
        if fail, return a negative value.
*************************************************/
int mcu_chan_read_timewait(hmcuchan_t handler, int timeout);


/*************************************************
@brief this function means mcu_chan_read_timewait with timeout 0.
@param
@return if ok, return 0.
        if fail, return a negative value.
*************************************************/
int mcu_chan_read_trywait(hmcuchan_t handler);


/*************************************************
@brief wait until mcu channel can write;
@param timeout:
       wait time in milliseconds;
       a negative value means an infinite wait.
       a timeout of 0 causes return immediately, even if no write data ready.
@return if ok, return 0.
        if fail, return a negative value.
*************************************************/
int mcu_chan_write_timewait(hmcuchan_t handler, int timeout);


/*************************************************
@brief this function means mcu_chan_write_timewait with timeout 0.
@param
@return if ok, return 0.
        if fail, return a negative value.
*************************************************/
int mcu_chan_write_trywait(hmcuchan_t handler);


/*************************************************
@brief read mcu channel;
@param buf: data buffer for read;
       count: read up to count bytes;
@return if ok, the number of bytes read.
        if fail, return -1.
*************************************************/
ssize_t mcu_chan_read(hmcuchan_t handler, void *buf, size_t count);


/*************************************************
@brief write mcu channel;
@param buf: data buffer for write;
       count: write up to count bytes;
@return if ok, the number of bytes write.
        if fail, return -1.
*************************************************/
ssize_t mcu_chan_write(hmcuchan_t handler, void *buf, size_t count);


/*************************************************
@brief close the access channel of mcu;
@param
@return if ok, return the handle of mcu access channel.
        if fail, return NULL.
*************************************************/
int mcu_chan_close(hmcuchan_t handler);

#ifdef __cplusplus
}
#endif   /* end of __cplusplus */
