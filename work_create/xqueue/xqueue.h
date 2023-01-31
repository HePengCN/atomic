/*
*******************************************************************
 * Copyright (C):
 * FileName: xqueue.h
 * Author:        Version :          Date:
 * Description:
 * Version:
 * Function List:
 * History:
      <author>  <time>   <version >   <desc>
      David    96/10/12     1.0     build this moudle
*******************************************************************
*/
#ifndef __XQUEUE_H__
#define __XQUEUE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

typedef enum xqueue_ret_val
{
    XQUEUE_RET_OK = 0,
    XQUEUE_RET_ERROR = -1,
    XQUEUE_RET_TIMEOUT = 1
} xqueue_ret_val_t;


typedef void *hqueue_t;

/*************************************************
@brief create a new thread safe FIFO queue, in which random length data can push and pop,
       so you should indicate the max length of data you will deal with.
       should call xqueue_destory when user done.
@param msgmaxlen:
       the max length of data you will deal with;
@return if ok: the new handle;
        if fail: return NULL.
*************************************************/
hqueue_t xqueue_create(uint32_t msg_max_len);

hqueue_t xqueue_create_with_pool(uint32_t msg_max_len, uint32_t pre_alloc_count);


/*************************************************
@brief destory the queue
@param hqueue: the handle of the queue.
@return
*************************************************/
void xqueue_destory(hqueue_t hqueue);


/*************************************************
@brief enqueue one msg to the queue
@param hqueue: the handle of the queue.
       data: the data buffer;
       data_size: the size of the data;
@return if ok: 0;
        if fail: <0;
*************************************************/
int xqueue_enqueue(hqueue_t hqueue, const void *data, uint32_t data_size);


/*************************************************
@brief check if the queue is empty
@param hqueue: the handle of the queue.
       data: the data buffer;
       data_size: the size of the data;
@return if ok: true;
        if fail: false;
*************************************************/
bool xqueue_isempty(hqueue_t hqueue);


/*************************************************
@brief try dequeue the head data from the queue,
       ok if queue is not empty, faile if the queue is empty.
@param hqueue: the handle of the queue.
       data: the data buffer;
       data_size: the size of the data;
@return if ok: true;
        if fail: false;
*************************************************/
bool xqueue_try_dequeue(hqueue_t hqueue, void *data, uint32_t *data_size);


/*************************************************
@brief time wait until the queue is not empty, then dequeue the data.
@param hqueue: the handle of the queue.
       data: the data buffer;
       data_size: the size of the data;
       timeout_msec: timeout value in millisecond.
@return if ok: XQUEUE_RET_OK;
        if error: XQUEUE_RET_ERROR;
        timeout: XQUEUE_RET_TIMEOUT;
*************************************************/
int xqueue_timewait_and_dequeue(hqueue_t hqueue, void *data, uint32_t *data_size, int timeout_msec);


/*************************************************
@brief infinite wait until the queue is not empty, then dequeue the data.
@param hqueue: the handle of the queue.
       data: the data buffer;
       data_size: the size of the data;
@return if ok: 0;
        if fail: <0;
*************************************************/
int xqueue_wait_and_dequeue(hqueue_t hqueue, void *data, uint32_t *data_size);


/*************************************************
@brief try read but not dequeue the head data from the queue,
       ok if queue is not empty, faile if the queue is empty.
@param hqueue: the handle of the queue.
       data: the data buffer;
       data_size: the size of the data;
@return if ok: true;
        if fail: false;
*************************************************/
bool xqueue_try_front(hqueue_t hqueue, void *data, uint32_t *data_size);


/*************************************************
@brief time wait until the queue is not empty, then read but not dequeue the data.
@param hqueue: the handle of the queue.
       data: the data buffer;
       data_size: the size of the data;
       timeout_msec: timeout value in millisecond.
@return if ok: 0;
        if error: <0;
        timeout: 1;
*************************************************/
int xqueue_timewait_and_front(hqueue_t hqueue, void *data, uint32_t *data_size, int timeout_msec);


/*************************************************
@brief infinite wait until the queue is not empty, then read but not dequeue the data.
@param hqueue: the handle of the queue.
       data: the data buffer;
       data_size: the size of the data;
@return if ok: 0;
        if fail: <0;
*************************************************/
int xqueue_wait_and_front(hqueue_t hqueue, void *data, uint32_t *data_size);


/*************************************************
@brief get how many data in the queue
@param hqueue: the handle of the queue.
@return data cout
*************************************************/
uint32_t xqueue_size(hqueue_t hqueue);


/*************************************************
@brief clear all elements
@param hqueue: the handle of the queue.
@return void
*************************************************/
void xqueue_clear(hqueue_t hqueue);


/*************************************************
@brief check the max length of the queue support
@param hqueue: the handle of the queue.
@return max length
*************************************************/
uint32_t xqueue_msg_max_len(hqueue_t hqueue);


/*************************************************
@brief set the max item count of the queue.
       when the item number reachs the max count, the oldest item will be dropped.
@param hqueue: the handle of the queue.
       max_count: =0 means no limit; >0  means count is the max number.
@return void
*************************************************/
void xqueue_set_max_count(hqueue_t hqueue, uint32_t max_count);


#ifdef __cplusplus
}
#endif

#endif //__XQUEUE_H__
