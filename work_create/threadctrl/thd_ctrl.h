/*
*******************************************************************
 * Copyright (C):
 * FileName: thd_ctrl.h
 * Author:        Version :          Date:
 * Description:
 * Version:
 * Function List:
 * History:
      <author>  <time>   <version >   <desc>
      David    96/10/12     1.0     build this moudle
*******************************************************************
*/
#ifndef __THD_CTRL_H__
#define __THD_CTRL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define THD_CNTL_MAGIC_NO (0x54484443) /**<ascii for THDC>**/
#define THD_NAME_MAX_LENGTH (15)

typedef enum thd_state
{
    THD_STATE_UNCREAT = THD_CNTL_MAGIC_NO,
    THD_STATE_INITED,
    THD_STATE_RUNNING,
    THD_STATE_PAUSED,
    THD_STATE_EXITED
} thd_state_t;

typedef enum thd_sched_policy
{
    THD_SCHED_OTHER,
    THD_SCHED_FIFO,
    THD_SCHED_RR,
    THD_SCHED_UNKNOWN
} thd_sched_policy_t;

typedef struct thd_sched_param
{
    thd_sched_policy_t sched_policy;
    int sched_priority;             // 1~99, 1 means lowest priority, and 99 mean highest priority
} thd_sched_param_t;

typedef enum task_func_ret_val
{
    RET_OK = 0,
    RET_AGAIN = -1,
    RET_EXIT = 1
} task_func_ret_val_t;

typedef int (*task_func)(void*);

typedef void* hthd_t;


/*************************************************
@brief make a new thread handle,
       should call release function when use done.
@param name: thead name, the max length of thread name is defined by THD_NAME_MAX_LENGTH.
       task_init: the init function of the affair in the thread;
                  the return value of task_init: RET_OK means ok, RET_EXIT means fail.
       task_onceopr: the once operation of the affair in the thread,
                     this function will be called repeatedly in loop,
                     the return value of task_onceopr: RET_OK means everythis is ok, RET_AGAIN means error happened but thread loop will continue, RET_EXIT means thread ready to exit.
       task_clear: the clear function of the affair in the thread;
                   the return value of task_clear: RET_OK means ok, RET_EXIT means fail.
       param:  the parameter for task_init, task_onceopr, task_clear,
               yes, the 3 function use the same parameter;
       sizeofparam: the size of the above param.
@return if ok, a new handle;
        if fali, return NULL;
*************************************************/
hthd_t thd_handle_new(const char* name,
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
void thd_handle_release(hthd_t hthd);


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
@brief pause a new thread in OS,
       when this function return ,the new thread is in the THD_STATE_PAUSED state;
@param thd: handle of the thread;
@return if ok, 0;
        if fali, <0;
*************************************************/
int thd_pause(hthd_t thd);


/*************************************************
@brief destroy a thread in OS,
       when this function return ,the new thread is in the THD_STATE_EXITED state;
@param thd: handle of the thread;
@return if ok, 0;
        if fali, <0;
*************************************************/
int thd_destroy(hthd_t thd);


/*************************************************
@brief first check if the thread is alive, then destroy the thread,
       when this function return ,the new thread is in the THD_STATE_EXITED or THD_STATE_UNCREAT state;
@param thd: handle of the thread;
@return if ok, 0;
        if fali, <0;
*************************************************/
int thd_check_alive_and_destroy(hthd_t thd);


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
@return enum thd_state_t
*************************************************/
thd_state_t thd_state(hthd_t thd);


char*  thd_state_str(thd_state_t state);


/*************************************************
@brief this is the combination of thd_create and thd_start,
       when this function return ,the new thread is in the THD_STATE_RUNNING state;
@param thd: handle of the thread;
@return if ok, 0;
        if fali, <0;
*************************************************/
int thd_create_and_start(hthd_t thd);


/*************************************************
@brief set scheduling policy information of thread.
       this function should be called befor thd_create.
@param thd: handle of the thread;
       sched_param: see defination of thd_sched_param_t.
@return if ok, 0;
        if fali, <0;
*************************************************/
int thd_set_sched(hthd_t thd, const thd_sched_param_t* sched_param);


/*************************************************
@brief get scheduling policy information of thread.
       this function should be called after thd_create.
@param thd: handle of the thread;
       sched_param: see defination of thd_sched_param_t.
@return if ok, 0;
        if fali, <0;
*************************************************/
int thd_get_sched(hthd_t thd, thd_sched_param_t* sched_param);


/*************************************************
@brief return the string format of thd_sched_policy_t enum variable.
       this function is prepared for debuging.
@param policy: the enum variable of thd_sched_policy_t.
@return char pointor for string format of sched policy;
*************************************************/
char* thd_sched_policy_str(thd_sched_policy_t policy);


/*************************************************
@brief return the kernel thread ID;
@param thd: handle of the thread;
@return the kernel thread ID;
*************************************************/
pid_t thd_get_tid(hthd_t thd);

#ifdef __cplusplus
}
#endif   /* end of __cplusplus */

#endif //__THD_CTRL_H__