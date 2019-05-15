/*
*******************************************************************
 * Copyright (C):
 * FileName: xstack.h
 * Author:        Version :          Date:
 * Description:
 * Version:
 * Function List:
 * History:
      <author>  <time>   <version >   <desc>
      David    96/10/12     1.0     build this moudle
*******************************************************************
*/
#ifndef __XSTACK_H__
#define __XSTACK_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

typedef void* hstack_t;


/*************************************************
@brief create a new thread safe stack, in which random length data can push and pop,
       so you should indicate the max length of data you will deal with.
       should call xstack_destory when user done.
@param msgmaxlen:
       the max length of data you will deal with;
@return if ok: the new handle;
        if fail: return NULL.
*************************************************/
hstack_t xstack_create(uint32_t msgmaxlen);


/*************************************************
@brief destory the stack
@param hstack: the handle of the stack.
@return
*************************************************/
void xstack_destory(hstack_t hstack);


/*************************************************
@brief push one msg to the stack
@param hstack: the handle of the stack.
       data: the data buffer;
       data_size: the size of the data;
@return if ok: 0;
        if fail: <0;
*************************************************/
int xstack_push(hstack_t hstack, const void* data, uint32_t data_size);


/*************************************************
@brief check if the stack is empty
@param hstack: the handle of the stack.
       data: the data buffer;
       data_size: the size of the data;
@return if ok: true;
        if fail: false;
*************************************************/
bool xstack_isempty(hstack_t hstack);


/*************************************************
@brief try pop the head data from the stack,
       ok if stack is not empty, faile if the stack is empty.
@param hstack: the handle of the stack.
       data: the data buffer;
       data_size: the size of the data;
@return if ok: true;
        if fail: false;
*************************************************/
bool xstack_try_pop(hstack_t hstack, void* data, uint32_t* data_size);


/*************************************************
@brief wait until the stack is not empty, then pop the data.
@param hstack: the handle of the stack.
       data: the data buffer;
       data_size: the size of the data;
@return if ok: 0;
        if fail: <0;
*************************************************/
int xstack_wait_and_pop(hstack_t hstack, void* data, uint32_t* data_size);


/*************************************************
@brief try read but not pop the head data from the stack,
       ok if stack is not empty, faile if the stack is empty.
@param hstack: the handle of the stack.
       data: the data buffer;
       data_size: the size of the data;
@return if ok: true;
        if fail: false;
*************************************************/
bool xstack_try_front(hstack_t hstack, void* data, uint32_t* data_size);


/*************************************************
@brief wait until the stack is not empty, then read but not pop the data.
@param hstack: the handle of the stack.
       data: the data buffer;
       data_size: the size of the data;
@return if ok: 0;
        if fail: <0;
*************************************************/
int xstack_wait_and_front(hstack_t hstack, void* data, uint32_t* data_size);


/*************************************************
@brief get how many data in the queu
@param hstack: the handle of the stack.
@return data cout
*************************************************/
uint32_t xstack_size(hstack_t hstack);


/*************************************************
@brief check the max length of the stack support
@param hstack: the handle of the stack.
@return max length
*************************************************/
uint32_t xstack_msg_maxlen(hstack_t hstack);

/*************************************************
@brief set the max item count of the stack.
       when the item number reachs the max count, the oldest item will be dropped.
@param hstack: the handle of the stack.
       count: <0 means no limit; = 0 diable all push operation; >0  means count is the max number.
@return void
*************************************************/
void xstack_set_max_count(hstack_t hstack, int max_count);


#ifdef __cplusplus
}
#endif

#endif //__XSTACK_H__