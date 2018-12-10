/*
*******************************************************************
 * Copyright (C):
 * FileName: mcuXcpu_msg.h
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

#define THD_CNTL_MAGIC_NO (0x54484443) /**<ascii for THDC>**/

typedef enum thd_state
{
    THD_STATE_UNCREAT = THD_CNTL_MAGIC_NO,
    THD_STATE_INITED,
    THD_STATE_RUNNING,
    THD_STATE_STOPPED,
    THD_STATE_EXITED
} thd_state_e;

typedef int (*task_func)(void*);

typedef void* hthd_t;


/*************************************************
@brief make a new thread handle,
       should call release function when use done.
@param name: thead name
       task_init: the init function of the affair in the thread;
                  the return value of task_init: 0 means ok, <0 means fail.
       task_onceopr: the once operation of the affair in the thread,
                     this function will be called repeatedly in loop,
                     the return value of task_onceopr: 0 means ok, <0 means fail, >0 means will affair done, thread should exit.
       task_clear: the clear function of the affair in the thread;
                   the return value of task_clear: 0 means ok, <0 means fail.
       param:  the parameter for task_init, task_onceopr, task_clear,
               yes, the 3 function use the same parameter;
       sizeofparam: the size of the above param.
@return if ok, a new handle;
        if fali, return NULL;
*************************************************/
hthd_t thd_handle_new(const char name[16],
                      task_func task_init,
                      task_func task_onceopr,
                      task_func task_clear,
                      void* param,
                      size_t sizeofparam);


/*************************************************
@brief release a new thread handle
@param
@return
*************************************************/
void thd_handle_release(hthd_t);


/*************************************************
@brief create a new thread in OS,
       when this function return ,the new thread is in the THD_STATE_INITED state;
@param thd: handle of the thread;
@return if ok, 0;
        if fali, <0;
*************************************************/
int thd_create(hthd_t thd);


/*************************************************
@brief start a new thread in OS,
       when this function return ,the new thread is in the THD_STATE_RUNNING state;
@param thd: handle of the thread;
@return if ok, 0;
        if fali, <0;
*************************************************/
int thd_start(hthd_t thd);


/*************************************************
@brief stop a new thread in OS,
       when this function return ,the new thread is in the THD_STATE_STOPPED state;
@param thd: handle of the thread;
@return if ok, 0;
        if fali, <0;
*************************************************/
int thd_stop(hthd_t thd);


/*************************************************
@brief stop a new thread in OS,
       when this function return ,the new thread is in the THD_STATE_EXITED state;
@param thd: handle of the thread;
@return if ok, 0;
        if fali, <0;
*************************************************/
int thd_destory(hthd_t thd);


/*************************************************
@brief join the thread,
@param thd: handle of the thread;
@return if ok, 0;
        if fali, <0;
*************************************************/
int thd_join(hthd_t thd);

const char* thd_name(hthd_t thd);


/*************************************************
@brief get the thread state,
@param thd: handle of the thread;
@return enum thd_state_e
*************************************************/
thd_state_e thd_state(hthd_t thd);
char*  thd_state_str(thd_state_e);


/*************************************************
@brief this is the combination of thd_create and thd_start,
       when this function return ,the new thread is in the THD_STATE_RUNNING state;
@param thd: handle of the thread;
@return if ok, 0;
        if fali, <0;
*************************************************/
int thd_create_and_start(hthd_t thd);
#ifdef __cplusplus
}
#endif   /* end of __cplusplus */
