#include <sys/time.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include "thd_ctl.h"
#include "com_log.h"

#define TAG "THD_CTL"

typedef void*(*pthread_start_routine)(void*);

typedef struct hThd_pthd
{
    volatile thd_state_e state;
    char name[16];// thread name
    int  timeout;// timeout of ctrl waiting: in msec
    void* (*start_routine)(struct hThd_pthd*);
    pthread_t thd;
    pthread_mutex_t mutex;
    pthread_cond_t  cond;// as start condtion, and other condtion as wish
    volatile bool stop;
    volatile bool exit;
    task_func task_init;
    task_func task_onceopr;
    task_func task_clear;
    char ext[0];//EXTERNAL resource interface for thd, resouce managed by task or others.
} hThd_pthd_t;

static struct timespec get_outtime(int msec)
{
    struct timeval now;
    struct timespec outtime;

    gettimeofday(&now, NULL);
    outtime.tv_sec = now.tv_sec + msec / 1000;
    outtime.tv_nsec = now.tv_usec * 1000 + (msec % 1000) * 1000000l;
    return outtime;
}

static int wait_fb(hThd_pthd_t* hThd)
{
    if (hThd->timeout < 0)
    {
        return pthread_cond_wait(&(hThd->cond), &(hThd->mutex));
    }

    struct timespec outtime = get_outtime(hThd->timeout);
    return pthread_cond_timedwait(&(hThd->cond), &(hThd->mutex), &outtime);
}


/*************************************************
@brief the routine function of the thead
@param hThd: the thread control handle
@return
*************************************************/
static void* start_routine(hThd_pthd_t* hThd)
{
    bool stop = false, exit = false;

    pthread_mutex_lock(&(hThd->mutex));// wait creat allow to run
    pthread_mutex_unlock(&(hThd->mutex));

    int ret = pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    if (0 != ret)
    {
        COM_LOG_WARN(TAG, "in thread <%s> pthread_setcancelstate fail. error msg: %s\n", hThd->name, strerror(errno));
    }

    ret = hThd->task_init((void*)(hThd->ext));

    pthread_mutex_lock(&(hThd->mutex));
    if (0 == ret)
    {
        hThd->state = THD_STATE_INITED;
        pthread_cond_signal(&(hThd->cond));
    }
    else
    {
        COM_LOG_ERROR(TAG, "task_init of thread(%s) return fail: error no: %d\n", hThd->name, ret);
        hThd->exit = true;
    }

wait:
    if (!hThd->exit) // init fail: NOT WAIT; init success or stopped, will wait;
    {
        pthread_cond_wait(&(hThd->cond), &(hThd->mutex)); //waiting, unlock, master start to run
    }

    stop = hThd->stop;
    exit = hThd->exit; // destroy happend in inited or stopped.  when init, stop == false, when stopped, stop == true, so do not care stop flag.
    if (!exit && !stop)
    {
        hThd->state = THD_STATE_RUNNING;
        pthread_cond_signal(&(hThd->cond));//feedback for start
    }
    pthread_mutex_unlock(&(hThd->mutex));

    while (!stop && !exit)
    {

        ret = hThd->task_onceopr((void*)(hThd->ext));
        if (0 != ret)
        {
            COM_LOG_ERROR(TAG, "task_onceopr of thread<%s> return fail, error no: %d\n", hThd->name, ret);
            hThd->exit = true;
        }

        pthread_mutex_lock(&(hThd->mutex));
        stop = hThd->stop;
        exit = hThd->exit;
        pthread_mutex_unlock(&(hThd->mutex));
    }

    if (stop && !exit)
    {
        pthread_mutex_lock(&(hThd->mutex));
        hThd->state = THD_STATE_STOPPED; //initial done
        pthread_cond_signal(&(hThd->cond)); //feedback for stop
        goto wait;
    }

    /*enter exit process*/
    ret = hThd->task_clear((void*)(hThd->ext));
    if (0 != ret)
    {
        COM_LOG_ERROR(TAG, "task_clear of thread<%s> return fail, error no: %d\n", hThd->name, ret);
    }

    pthread_mutex_lock(&(hThd->mutex));
    hThd->state = THD_STATE_EXITED;
    pthread_cond_signal(&(hThd->cond));  //feedback for destory
    pthread_mutex_unlock(&(hThd->mutex));
    return hThd;
}


/*************************************************
@brief create a new thread in OS,
       when this function return ,the new thread is in the THD_STATE_INITED state;
@param thd: handle of the thread;
@return if ok, 0;
        if fali, <0;
*************************************************/
int thd_create(hthd_t thd)
{
    assert(NULL != thd);
    int ret = 0;
    hThd_pthd_t *hThd = (hThd_pthd_t*)thd;

    pthread_mutex_lock(&(hThd->mutex));
    if (THD_STATE_UNCREAT != hThd->state && THD_STATE_EXITED != hThd->state)
    {
        COM_LOG_ERROR(TAG, "state of thread<%s> is not expected. error state:%s\n", hThd->name, thd_state_str(hThd->state));
        ret = -1;
        goto end;
    }

    hThd->stop = false;
    hThd->exit = false;
    ret = pthread_create(&(hThd->thd), NULL, (pthread_start_routine)(hThd->start_routine), hThd);
    if (0 != ret)
    {
        COM_LOG_ERROR(TAG, "pthread_create for thread<%s> fail. error no: %d\n", hThd->name, ret);
        goto end;
    }

    ret = wait_fb(hThd);//unlock here
    if (0 != ret)
    {
        COM_LOG_WARN(TAG, "wait_fb fail with(%d). thread<%s> got blocked by its task_init function.\n", ret, hThd->name);
    }

    if (THD_STATE_INITED != hThd->state)
    {
        COM_LOG_ERROR(TAG, "create thread<%s> fail, error state: %s\n", hThd->name, thd_state_str(hThd->state));
        ret = -3;
        goto end;
    }

    ret = 0;
end:
    pthread_mutex_unlock(&(hThd->mutex));
    return ret;
}


/*************************************************
@brief start a new thread in OS,
       when this function return ,the new thread is in the THD_STATE_RUNNING state;
@param thd: handle of the thread;
@return if ok, 0;
        if fali, <0;
*************************************************/
int thd_start(hthd_t thd)
{
    assert(NULL != thd);
    int ret = 0;
    hThd_pthd_t *hThd = (hThd_pthd_t*)thd;

    pthread_mutex_lock(&(hThd->mutex));

    if (THD_STATE_STOPPED != hThd->state && THD_STATE_INITED != hThd->state)
    {
        COM_LOG_ERROR(TAG, "state of thread<%s> is not expected. error state:%s\n", hThd->name, thd_state_str(hThd->state));
        ret = -1;
        goto end;
    }

    hThd->stop = false;
    pthread_cond_signal(&(hThd->cond));

    ret = wait_fb(hThd);
    if (0 != ret)
    {
        COM_LOG_WARN(TAG, "wait_fb fail with(%d). thread<%s> got blocked.\n", ret, hThd->name);
    }

    if (THD_STATE_RUNNING != hThd->state)
    {
        COM_LOG_ERROR(TAG, "start thread<%s> fail, error state: %s\n", hThd->name, thd_state_str(hThd->state));
        ret = -3;
        goto end;
    }
    ret = 0;
end:
    pthread_mutex_unlock(&(hThd->mutex));  // thd start to run
    return ret;
}


/*************************************************
@brief stop a new thread in OS,
       when this function return ,the new thread is in the THD_STATE_STOPPED state;
@param thd: handle of the thread;
@return if ok, 0;
        if fali, <0;
*************************************************/
int thd_stop(hthd_t thd)
{
    assert(NULL != thd);
    int ret = 0;
    hThd_pthd_t *hThd = (hThd_pthd_t*)thd;

    pthread_mutex_lock(&(hThd->mutex));
    if (THD_STATE_RUNNING != hThd->state)
    {
        COM_LOG_ERROR(TAG, "state of thread<%s> is not expected. error state:%s\n", hThd->name, thd_state_str(hThd->state));
        ret = -1;
        goto end;
    }
    hThd->stop = true;

    ret = wait_fb(hThd);
    if (0 != ret)
    {
        COM_LOG_WARN(TAG, "wait_fb fail with(%d). thread<%s> got blocked by its task_onceopr function.\n", ret, hThd->name);
    }

    if (THD_STATE_STOPPED != hThd->state)
    {
        COM_LOG_ERROR(TAG, "stop thread<%s> fail, error state: %s\n", hThd->name, thd_state_str(hThd->state));
        ret = -3;
        goto end;
    }
    ret = 0;
end:
    pthread_mutex_unlock(&(hThd->mutex));
    return ret;
}


/*************************************************
@brief join the thread,
@param thd: handle of the thread;
@return if ok, 0;
        if fali, <0;
*************************************************/
int thd_join(hthd_t thd)
{
    assert(NULL != thd);
    hThd_pthd_t *hThd = (hThd_pthd_t*)thd;
    pthread_mutex_lock(&(hThd->mutex));
    if (THD_STATE_INITED == hThd->state || THD_STATE_STOPPED == hThd->state)
    {
        fprintf(stderr, "%s, %d: Warning: Thread <%s> is in state: %s.\n", __FUNCTION__, __LINE__,  hThd->name, thd_state_str(hThd->state));
    }
    pthread_mutex_unlock(&(hThd->mutex));
    return pthread_join(hThd->thd, NULL);
}


/*************************************************
@brief stop a new thread in OS,
       when this function return ,the new thread is in the THD_STATE_EXITED state;
@param thd: handle of the thread;
@return if ok, 0;
        if fali, <0;
*************************************************/
int thd_destory(hthd_t thd)
{
    assert(NULL != thd);
    int ret = 0;
    hThd_pthd_t *hThd = (hThd_pthd_t*)thd;
    pthread_mutex_lock(&(hThd->mutex));
    if (THD_STATE_UNCREAT == hThd->state || THD_STATE_EXITED == hThd->state)
    {
        COM_LOG_ERROR(TAG, "state of thread<%s> is not expected. error state:%s\n", hThd->name, thd_state_str(hThd->state));
        ret = -1;
        goto end;
    }

    hThd->exit = true;
    if (THD_STATE_RUNNING != hThd->state)
    {
        pthread_cond_signal(&(hThd->cond));
    }

    ret = wait_fb(hThd);
    if (0 != ret)
    {
        COM_LOG_WARN(TAG, "wait_fb fail with(%d). thread<%s> got blocked by its task_clear function.\n", ret, hThd->name);
    }

    if (THD_STATE_EXITED != hThd->state)
    {
        COM_LOG_ERROR(TAG, "destroy thread<%s> fail, error state: %s\n", hThd->name, thd_state_str(hThd->state));
        ret = -5;
        goto end;
    }
    ret = 0;
end:
    pthread_mutex_unlock(&(hThd->mutex));
    return ret;
}


/*************************************************
@brief this is the combination of thd_create and thd_start,
       when this function return ,the new thread is in the THD_STATE_RUNNING state;
@param thd: handle of the thread;
@return if ok, 0;
        if fali, <0;
*************************************************/
int thd_create_and_start(hthd_t thd)
{
    assert(NULL != thd);
    int ret = 0;
    ret = thd_create(thd);
    if (0 != ret)
    {
        return ret;
    }
    return thd_start(thd);
}


/*************************************************
@brief get the thread state,
@param thd: handle of the thread;
@return enum thd_state_e
*************************************************/
thd_state_e thd_state(hthd_t thd)
{
    assert(NULL != thd);
    hThd_pthd_t *hThd = (hThd_pthd_t*)thd;
    pthread_mutex_lock(&(hThd->mutex));
    thd_state_e ret = hThd->state;
    pthread_mutex_unlock(&(hThd->mutex));
    return ret;
}

const char* thd_name(hthd_t thd)
{
    assert(NULL != thd);
    hThd_pthd_t *hThd = (hThd_pthd_t*)thd;
    return (const char*)hThd->name;
}

char*  thd_state_str(thd_state_e state)
{
    static char strs[THD_STATE_EXITED - THD_STATE_UNCREAT + 1][32] = {"THD_STATE_UNCREAT",
                                                                      "THD_STATE_INITED",
                                                                      "THD_STATE_RUNNING",
                                                                      "THD_STATE_STOPPED",
                                                                      "THD_STATE_EXITED"
                                                                     };
    return strs[state - THD_STATE_UNCREAT];
}


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
                      size_t sizeofparam)
{
    assert(NULL != task_init);
    assert(NULL != task_onceopr);
    assert(NULL != task_clear);

    hThd_pthd_t* hThd = (hThd_pthd_t*)malloc(sizeof(hThd_pthd_t) + sizeofparam);
    if (NULL == hThd)
    {
        COM_LOG_ERROR(TAG, "malloc fail, sizeof(hThd_pthd_t): %"PRIu64", sizeofparam: %"PRIu64"\n", (uint64_t)sizeof(hThd_pthd_t), (uint64_t)sizeofparam);
        return NULL;
    }
    memset(hThd, 0, sizeof(hThd_pthd_t) + sizeofparam);

    hThd->state = THD_STATE_UNCREAT;
    strncpy(hThd->name, name, (sizeof(hThd->name) - 1));
    hThd->timeout = -1;
    hThd->start_routine = start_routine;
    (void)hThd->thd;// will init in thd_create
    pthread_mutex_init(&(hThd->mutex), NULL);
    pthread_cond_init(&(hThd->cond), NULL);
    hThd->stop = false;
    hThd->exit = false;
    hThd->task_init = task_init;
    hThd->task_onceopr = task_onceopr;
    hThd->task_clear = task_clear;
    if (NULL != param)
    {
        memcpy(hThd->ext, param, sizeofparam);
    }
    return hThd;
}


/*************************************************
@brief release a new thread handle
@param
@return
*************************************************/
void thd_handle_release(hthd_t thd)
{
    if (NULL == thd)
    {
        COM_LOG_WARN(TAG, "param with NULL pointer\n");
        return;
    }
    hThd_pthd_t* hThd = (hThd_pthd_t*)thd;
    pthread_cond_destroy(&(hThd->cond));
    pthread_mutex_destroy(&(hThd->mutex));
    free(hThd);
}
