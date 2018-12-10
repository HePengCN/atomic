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
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

typedef void* hqueue_t;


/*************************************************
@brief create a new thread safe FIFO queue, in which random length data can push and pop,
       so you should indicate the max length of data you will deal with.
       should call xqueue_destory when user done.
@param msgmaxlen:
       the max length of data you will deal with;
@return if ok: the new handle;
        if fail: return NULL.
*************************************************/
hqueue_t xqueue_new(uint32_t msgmaxlen);


/*************************************************
@brief destory the queue
@param hQueue: the handle of the queue.
@return
*************************************************/
void xqueue_destory(hqueue_t hQueue);


/*************************************************
@brief push one msg to the queue
@param hQueue: the handle of the queue.
       data: the data buffer;
       data_size: the size of the data;
@return if ok: 0;
        if fail: <0;
*************************************************/
int xqueue_push(hqueue_t hQueue, const void* data, uint32_t data_size);


/*************************************************
@brief check if the queue is empty
@param hQueue: the handle of the queue.
       data: the data buffer;
       data_size: the size of the data;
@return if ok: true;
        if fail: false;
*************************************************/
bool xqueue_empty(hqueue_t hQueue);


/*************************************************
@brief try pop the head data from the queue,
       ok if queue is not empty, faile if the queue is empty.
@param hQueue: the handle of the queue.
       data: the data buffer;
       data_size: the size of the data;
@return if ok: true;
        if fail: false;
*************************************************/
bool xqueue_try_pop(hqueue_t hQueue, void* data, uint32_t* data_size);


/*************************************************
@brief wait until the queue is not empty, then pop the data.
@param hQueue: the handle of the queue.
       data: the data buffer;
       data_size: the size of the data;
@return if ok: 0;
        if fail: <0;
*************************************************/
int xqueue_wait_and_pop(hqueue_t hQueue, void* data, uint32_t* data_size);


/*************************************************
@brief try read but not pop the head data from the queue,
       ok if queue is not empty, faile if the queue is empty.
@param hQueue: the handle of the queue.
       data: the data buffer;
       data_size: the size of the data;
@return if ok: true;
        if fail: false;
*************************************************/
bool xqueue_try_front(hqueue_t hQueue, void* data, uint32_t* data_size);


/*************************************************
@brief wait until the queue is not empty, then read but not pop the data.
@param hQueue: the handle of the queue.
       data: the data buffer;
       data_size: the size of the data;
@return if ok: 0;
        if fail: <0;
*************************************************/
int xqueue_wait_and_front(hqueue_t hQueue, void* data, uint32_t* data_size);


/*************************************************
@brief get how many data in the queu
@param hQueue: the handle of the queue.
@return data cout
*************************************************/
uint32_t xqueue_size(hqueue_t hQueue);


/*************************************************
@brief check the max length of the queue support
@param hQueue: the handle of the queue.
@return max length
*************************************************/
uint32_t xqueue_msg_maxlen(hqueue_t hQueue);

#ifdef __cplusplus
}
#endif
