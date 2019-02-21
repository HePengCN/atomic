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

typedef void*(*pthread_start_routine)(void*);

typedef struct hthd_pthd
{
    volatile thd_state_e state;
    char name[16];// thread name
    int  timeout;// timeout of ctrl waiting: in msec
    void* (*start_routine)(struct hthd_pthd*);
    pthread_t thd;
    pthread_mutex_t mutex;
    pthread_cond_t  cond;// as start condtion, and other condtion as wish
    volatile bool stop;
    volatile bool exit;
    task_func task_init;
    task_func task_onceopr;
    task_func task_clear;
    char ext[0];//EXTERNAL resource interface for thd, resouce managed by task or others.
} hthd_pthd_t;

static struct timespec get_outtime(int msec)
{
    struct timeval now;
    struct timespec outtime;

    gettimeofday(&now, NULL);
    outtime.tv_sec = now.tv_sec + msec / 1000;
    outtime.tv_nsec = now.tv_usec * 1000 + (msec % 1000) * 1000000l;
    return outtime;
}

static int wait_fb(hthd_pthd_t* hthd)
{
    if (hthd->timeout < 0)
    {
        return pthread_cond_wait(&(hthd->cond), &(hthd->mutex));
    }

    struct timespec outtime = get_outtime(hthd->timeout);
    return pthread_cond_timedwait(&(hthd->cond), &(hthd->mutex), &outtime);
}


/*************************************************
@brief the routine function of the thead
@param hthd: the thread control handle
@return
*************************************************/
static void* start_routine(hthd_pthd_t* hthd)
{
    bool stop = false, exit = false;

    pthread_mutex_lock(&(hthd->mutex));// wait creat allow to run
    pthread_mutex_unlock(&(hthd->mutex));

    int ret = pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    if (0 != ret)
    {
        COM_LOG_WARN("in thread <%s> pthread_setcancelstate fail. error msg: %s\n", hthd->name, strerror(errno));
    }

    ret = hthd->task_init((void*)(hthd->ext));

    pthread_mutex_lock(&(hthd->mutex));
    if (0 == ret)
    {
        hthd->state = THD_STATE_INITED;
        pthread_cond_signal(&(hthd->cond));
    }
    else
    {
        COM_LOG_ERROR("task_init of thread(%s) return fail: error no: %d\n", hthd->name, ret);
        hthd->exit = true;
    }

wait:
    if (!hthd->exit) // init fail: NOT WAIT; init success or stopped, will wait;
    {
        pthread_cond_wait(&(hthd->cond), &(hthd->mutex)); //waiting, unlock, master start to run
    }

    stop = hthd->stop;
    exit = hthd->exit; // destroy happend in inited or stopped.  when init, stop == false, when stopped, stop == true, so do not care stop flag.
    if (!exit && !stop)
    {
        hthd->state = THD_STATE_RUNNING;
        pthread_cond_signal(&(hthd->cond));//feedback for start
    }
    pthread_mutex_unlock(&(hthd->mutex));

    while (!stop && !exit)
    {

        ret = hthd->task_onceopr((void*)(hthd->ext));
        if (0 != ret)
        {
            COM_LOG_ERROR("task_onceopr of thread<%s> return fail, error no: %d\n", hthd->name, ret);
            hthd->exit = true;
        }

        pthread_mutex_lock(&(hthd->mutex));
        stop = hthd->stop;
        exit = hthd->exit;
        pthread_mutex_unlock(&(hthd->mutex));
    }

    if (stop && !exit)
    {
        pthread_mutex_lock(&(hthd->mutex));
        hthd->state = THD_STATE_STOPPED; //initial done
        pthread_cond_signal(&(hthd->cond)); //feedback for stop
        goto wait;
    }

    /*enter exit process*/
    ret = hthd->task_clear((void*)(hthd->ext));
    if (0 != ret)
    {
        COM_LOG_ERROR("task_clear of thread<%s> return fail, error no: %d\n", hthd->name, ret);
    }

    pthread_mutex_lock(&(hthd->mutex));
    hthd->state = THD_STATE_EXITED;
    pthread_cond_signal(&(hthd->cond));  //feedback for destory
    pthread_mutex_unlock(&(hthd->mutex));
    return hthd;
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
    hthd_pthd_t *hthd = (hthd_pthd_t*)thd;

    pthread_mutex_lock(&(hthd->mutex));
    if (THD_STATE_UNCREAT != hthd->state && THD_STATE_EXITED != hthd->state)
    {
        COM_LOG_ERROR("state of thread<%s> is not expected. error state:%s\n", hthd->name, thd_state_str(hthd->state));
        ret = -1;
        goto end;
    }

    hthd->stop = false;
    hthd->exit = false;
    ret = pthread_create(&(hthd->thd), NULL, (pthread_start_routine)(hthd->start_routine), hthd);
    if (0 != ret)
    {
        COM_LOG_ERROR("pthread_create for thread<%s> fail. error no: %d\n", hthd->name, ret);
        goto end;
    }

    ret = wait_fb(hthd);//unlock here
    if (0 != ret)
    {
        COM_LOG_WARN("wait_fb fail with(%d). thread<%s> got blocked by its task_init function.\n", ret, hthd->name);
    }

    if (THD_STATE_INITED != hthd->state)
    {
        COM_LOG_ERROR("create thread<%s> fail, error state: %s\n", hthd->name, thd_state_str(hthd->state));
        ret = -1;
        goto end;
    }

    ret = 0;
end:
    pthread_mutex_unlock(&(hthd->mutex));
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
    hthd_pthd_t *hthd = (hthd_pthd_t*)thd;

    pthread_mutex_lock(&(hthd->mutex));

    if (THD_STATE_STOPPED != hthd->state && THD_STATE_INITED != hthd->state)
    {
        COM_LOG_ERROR("state of thread<%s> is not expected. error state:%s\n", hthd->name, thd_state_str(hthd->state));
        ret = -1;
        goto end;
    }

    hthd->stop = false;
    pthread_cond_signal(&(hthd->cond));

    ret = wait_fb(hthd);
    if (0 != ret)
    {
        COM_LOG_WARN("wait_fb fail with(%d). thread<%s> got blocked.\n", ret, hthd->name);
    }

    if (THD_STATE_RUNNING != hthd->state)
    {
        COM_LOG_ERROR("start thread<%s> fail, error state: %s\n", hthd->name, thd_state_str(hthd->state));
        ret = -1;
        goto end;
    }
    ret = 0;
end:
    pthread_mutex_unlock(&(hthd->mutex));  // thd start to run
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
    hthd_pthd_t *hthd = (hthd_pthd_t*)thd;

    pthread_mutex_lock(&(hthd->mutex));
    if (THD_STATE_RUNNING != hthd->state)
    {
        COM_LOG_ERROR("state of thread<%s> is not expected. error state:%s\n", hthd->name, thd_state_str(hthd->state));
        ret = -1;
        goto end;
    }
    hthd->stop = true;

    ret = wait_fb(hthd);
    if (0 != ret)
    {
        COM_LOG_WARN("wait_fb fail with(%d). thread<%s> got blocked by its task_onceopr function.\n", ret, hthd->name);
    }

    if (THD_STATE_STOPPED != hthd->state)
    {
        COM_LOG_ERROR("stop thread<%s> fail, error state: %s\n", hthd->name, thd_state_str(hthd->state));
        ret = -1;
        goto end;
    }
    ret = 0;
end:
    pthread_mutex_unlock(&(hthd->mutex));
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
    hthd_pthd_t *hthd = (hthd_pthd_t*)thd;
    pthread_mutex_lock(&(hthd->mutex));
    if (THD_STATE_INITED == hthd->state || THD_STATE_STOPPED == hthd->state)
    {
        fprintf(stderr, "%s, %d: Warning: Thread <%s> is in state: %s.\n", __FUNCTION__, __LINE__,  hthd->name, thd_state_str(hthd->state));
    }
    pthread_mutex_unlock(&(hthd->mutex));
    return pthread_join(hthd->thd, NULL);
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
    hthd_pthd_t *hthd = (hthd_pthd_t*)thd;
    pthread_mutex_lock(&(hthd->mutex));
    if (THD_STATE_UNCREAT == hthd->state || THD_STATE_EXITED == hthd->state)
    {
        COM_LOG_ERROR("state of thread<%s> is not expected. error state:%s\n", hthd->name, thd_state_str(hthd->state));
        ret = -1;
        goto end;
    }

    hthd->exit = true;
    if (THD_STATE_RUNNING != hthd->state)
    {
        pthread_cond_signal(&(hthd->cond));
    }

    ret = wait_fb(hthd);
    if (0 != ret)
    {
        COM_LOG_WARN("wait_fb fail with(%d). thread<%s> got blocked by its task_clear function.\n", ret, hthd->name);
    }

    if (THD_STATE_EXITED != hthd->state)
    {
        COM_LOG_ERROR("destroy thread<%s> fail, error state: %s\n", hthd->name, thd_state_str(hthd->state));
        ret = -1;
        goto end;
    }
    ret = 0;
end:
    pthread_mutex_unlock(&(hthd->mutex));
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
    hthd_pthd_t *hthd = (hthd_pthd_t*)thd;
    pthread_mutex_lock(&(hthd->mutex));
    thd_state_e ret = hthd->state;
    pthread_mutex_unlock(&(hthd->mutex));
    return ret;
}

const char* thd_name(hthd_t thd)
{
    assert(NULL != thd);
    hthd_pthd_t *hthd = (hthd_pthd_t*)thd;
    return (const char*)hthd->name;
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

    hthd_pthd_t* hthd = (hthd_pthd_t*)malloc(sizeof(hthd_pthd_t) + sizeofparam);
    if (NULL == hthd)
    {
        COM_LOG_ERROR("malloc fail, sizeof(hthd_pthd_t): %"PRIu64", sizeofparam: %"PRIu64"\n", (uint64_t)sizeof(hthd_pthd_t), (uint64_t)sizeofparam);
        return NULL;
    }
    memset(hthd, 0, sizeof(hthd_pthd_t) + sizeofparam);

    hthd->state = THD_STATE_UNCREAT;
    strncpy(hthd->name, name, (sizeof(hthd->name) - 1));
    hthd->timeout = -1;
    hthd->start_routine = start_routine;
    (void)hthd->thd;// will init in thd_create
    pthread_mutex_init(&(hthd->mutex), NULL);
    pthread_cond_init(&(hthd->cond), NULL);
    hthd->stop = false;
    hthd->exit = false;
    hthd->task_init = task_init;
    hthd->task_onceopr = task_onceopr;
    hthd->task_clear = task_clear;
    if (NULL != param)
    {
        memcpy(hthd->ext, param, sizeofparam);
    }
    return hthd;
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
        COM_LOG_WARN("param with NULL pointer\n");
        return;
    }
    hthd_pthd_t* hthd = (hthd_pthd_t*)thd;
    pthread_cond_destroy(&(hthd->cond));
    pthread_mutex_destroy(&(hthd->mutex));
    free(hthd);
}
