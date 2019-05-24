#include <sys/prctl.h>
#include <sys/syscall.h>
#include <unistd.h>
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
#include <signal.h>
#include "thd_ctrl.h"
#include "thd_ctrl_log.h"

#define gettid() syscall(__NR_gettid)

typedef void*(*pthread_start_routine)(void*);

typedef struct hthd_pthd
{
    volatile thd_state_t state;
    char name[THD_NAME_MAX_LENGTH + 1]; //thread name
    pid_t tid; //thread id;
    int timeout; //timeout of ctrl waiting: in msec
    void* (*start_routine)(struct hthd_pthd*);
    thd_sched_param_t sched_param;
    pthread_t thd;
    pthread_attr_t attr;
    pthread_mutex_t mutex;
    pthread_cond_t cond; //as start condtion, and other condtion as wish
    volatile bool pause;
    volatile bool exit;
    task_func task_init;
    task_func task_onceopr;
    task_func task_clear;
    char ext[0]; //external resource interface for thd, resouce managed by task or others.
} hthd_pthd_t;


char* thd_sched_policy_str(thd_sched_policy_t policy)
{

    static char strs[4][32] = {"THD_SCHED_OTHER",
                               "THD_SCHED_FIFO",
                               "THD_SCHED_RR",
                               "THD_SCHED_UNKNOWN"
                              };
    switch (policy)
    {
    case THD_SCHED_OTHER:
        return strs[0];
    case THD_SCHED_FIFO:
        return strs[1];
    case THD_SCHED_RR:
        return strs[2];
    default:
        return strs[3];
    }

}

static int to_pthread_sched_policy(thd_sched_policy_t policy)
{
    switch (policy)
    {
    case THD_SCHED_OTHER:
        return SCHED_OTHER;
    case THD_SCHED_FIFO:
        return SCHED_FIFO;
    case THD_SCHED_RR:
        return SCHED_RR;
    default:
        COM_LOG_ERROR("unknown sched policy parameter: %d\n", policy);
        return SCHED_OTHER;
    }
}

static int to_pthread_sched_priority(int priority)
{
    return priority;
}

static int to_thd_sched_policy(int pthread_sched_policy)
{
    switch (pthread_sched_policy)
    {
    case SCHED_OTHER:
        return THD_SCHED_OTHER;
    case SCHED_FIFO:
        return THD_SCHED_FIFO;
    case SCHED_RR:
        return THD_SCHED_RR;
    default:
        COM_LOG_ERROR("unknown pthread sched policy parameter: %d\n", pthread_sched_policy);
        return THD_SCHED_UNKNOWN;
    }
}

static int to_thd_sched_priority(int pthread_sched_priority)
{
    return pthread_sched_priority;
}

static struct timespec get_outtime(int msec)
{
    struct timeval now;
    struct timespec outtime;

    gettimeofday(&now, NULL);
    outtime.tv_sec = now.tv_sec + msec / 1000;
    outtime.tv_nsec = now.tv_usec * 1000 + (msec % 1000) * 1000000l;
    return outtime;
}

static int wait_feedback(hthd_pthd_t* hthd)
{
    if (hthd->timeout < 0)
    {
        return pthread_cond_wait(&(hthd->cond), &(hthd->mutex));
    }

    struct timespec outtime = get_outtime(hthd->timeout);
    return pthread_cond_timedwait(&(hthd->cond), &(hthd->mutex), &outtime);
}

static void pthread_cancel_cleanup(void* thd)
{
    hthd_pthd_t *hthd = (hthd_pthd_t*)thd;
    pthread_mutex_unlock(&(hthd->mutex));
    hthd->task_clear((void*)(hthd->ext));
}


/*************************************************
@brief the routine function of the thead
@param hthd: the thread control handle
@return
*************************************************/
static void* start_routine(hthd_pthd_t* hthd)
{
    pthread_cleanup_push(pthread_cancel_cleanup, hthd);

    bool pause = false, exit = false;

    hthd->tid = gettid();
    prctl(PR_SET_NAME, hthd->name);

    pthread_mutex_lock(&(hthd->mutex));// wait thd_creat allow to run
    pthread_mutex_unlock(&(hthd->mutex));

    int ret = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    if (ret != 0)
    {
        COM_LOG_ERROR("in thread <%s> pthread_setcancelstate fail. error msg: %s\n", hthd->name, strerror(errno));
        return hthd;
    }

    ret = pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
    if (ret != 0)
    {
        COM_LOG_ERROR("in thread <%s> pthread_setcanceltype fail. error msg: %s\n", hthd->name, strerror(errno));
        return hthd;
    }

    ret = hthd->task_init((void*)(hthd->ext));

    pthread_mutex_lock(&(hthd->mutex));
    if (ret == RET_OK)
    {
        hthd->state = THD_STATE_INITED;
        pthread_cond_signal(&(hthd->cond)); //feedback for thd_create
    }
    else
    {
        COM_LOG_ERROR("task_init of thread(%s) return fail: error no: %d\n", hthd->name, ret);
        hthd->exit = true;
    }


    do
    {
        pthread_cond_wait(&(hthd->cond), &(hthd->mutex));

        exit = hthd->exit;
        pause = hthd->pause;
        if (exit)
        {
            pthread_mutex_unlock(&(hthd->mutex));
            break;
        }

        hthd->state = THD_STATE_RUNNING;
        pthread_cond_signal(&(hthd->cond)); //feedback for thd_start
        pthread_mutex_unlock(&(hthd->mutex));

        do
        {
            ret = hthd->task_onceopr((void*)(hthd->ext));
            if (ret == RET_EXIT)
            {
                COM_LOG_INFO("task_onceopr of thread<%s> return RET_EXIT, thread about to exit.\n", hthd->name);
                pthread_mutex_lock(&(hthd->mutex));
                hthd->exit = true;
                pthread_mutex_unlock(&(hthd->mutex));
            }

            pthread_testcancel();

            pthread_mutex_lock(&(hthd->mutex));
            pause = hthd->pause;
            exit = hthd->exit;
            pthread_mutex_unlock(&(hthd->mutex));
        }
        while (!pause && !exit);

        if (pause && !exit)
        {
            pthread_mutex_lock(&(hthd->mutex));
            hthd->state = THD_STATE_PAUSED;
            pthread_cond_signal(&(hthd->cond)); //feedback for thd_pause
        }
    }
    while (pause && !exit);

    pthread_testcancel();

    /*enter exit process*/
    ret = hthd->task_clear((void*)(hthd->ext));
    if (ret != RET_OK)
    {
        COM_LOG_ERROR("task_clear of thread<%s> return fail, error no: %d\n", hthd->name, ret);
    }

    pthread_mutex_lock(&(hthd->mutex));
    hthd->state = THD_STATE_EXITED;
    pthread_cond_signal(&(hthd->cond));  //feedback for destory
    pthread_mutex_unlock(&(hthd->mutex));

    return hthd;

    pthread_cleanup_pop(0);
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
    assert(thd != NULL);
    int ret = 0;
    hthd_pthd_t *hthd = (hthd_pthd_t*)thd;

    pthread_mutex_lock(&(hthd->mutex));
    if (hthd->state != THD_STATE_UNCREAT && hthd->state != THD_STATE_EXITED)
    {
        COM_LOG_ERROR("state of thread<%s> is not expected. error state:%s\n", hthd->name, thd_state_str(hthd->state));
        ret = -1;
        goto end;
    }

    hthd->pause = false;
    hthd->exit = false;
    ret = pthread_create(&(hthd->thd), &(hthd->attr), (pthread_start_routine)(hthd->start_routine), hthd);
    if (ret != 0)
    {
        COM_LOG_ERROR("pthread_create for thread<%s> fail. error no: %d\n", hthd->name, ret);
        goto end;
    }

    ret = wait_feedback(hthd);//unlock here
    if (ret != 0)
    {
        COM_LOG_WARN("wait_feedback fail with(%d). thread<%s> got blocked by its task_init function.\n", ret, hthd->name);
    }

    if (hthd->state != THD_STATE_INITED)
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
    assert(thd != NULL);
    int ret = 0;
    hthd_pthd_t *hthd = (hthd_pthd_t*)thd;

    pthread_mutex_lock(&(hthd->mutex));

    if (hthd->state != THD_STATE_PAUSED && hthd->state != THD_STATE_INITED)
    {
        COM_LOG_ERROR("state of thread<%s> is not expected. error state:%s\n", hthd->name, thd_state_str(hthd->state));
        ret = -1;
        goto end;
    }

    hthd->pause = false;
    pthread_cond_signal(&(hthd->cond));

    ret = wait_feedback(hthd);
    if (ret != 0)
    {
        COM_LOG_WARN("wait_feedback fail with(%d). thread<%s> got blocked.\n", ret, hthd->name);
    }

    if (hthd->state != THD_STATE_RUNNING)
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
@brief pause a new thread in OS,
       when this function return ,the new thread is in the THD_STATE_PAUSED state;
@param thd: handle of the thread;
@return if ok, 0;
        if fali, <0;
*************************************************/
int thd_pause(hthd_t thd)
{
    assert(thd != NULL);
    int ret = 0;
    hthd_pthd_t *hthd = (hthd_pthd_t*)thd;

    pthread_mutex_lock(&(hthd->mutex));
    if (hthd->state != THD_STATE_RUNNING)
    {
        COM_LOG_ERROR("state of thread<%s> is not expected. error state:%s\n", hthd->name, thd_state_str(hthd->state));
        ret = -1;
        goto end;
    }
    hthd->pause = true;

    ret = wait_feedback(hthd);
    if (ret != 0)
    {
        COM_LOG_WARN("wait_feedback fail with(%d). thread<%s> got blocked by its task_onceopr function.\n", ret, hthd->name);
    }

    if (hthd->state != THD_STATE_PAUSED)
    {
        COM_LOG_ERROR("pause thread<%s> fail, error state: %s\n", hthd->name, thd_state_str(hthd->state));
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
    assert(thd != NULL);
    hthd_pthd_t *hthd = (hthd_pthd_t*)thd;
    pthread_mutex_lock(&(hthd->mutex));
    if (hthd->state == THD_STATE_INITED || hthd->state == THD_STATE_PAUSED)
    {
        fprintf(stderr, "%s, %d: Warning: Thread <%s> is in state: %s.\n", __FUNCTION__, __LINE__,  hthd->name, thd_state_str(hthd->state));
    }
    pthread_mutex_unlock(&(hthd->mutex));
    return pthread_join(hthd->thd, NULL);
}


/*************************************************
@brief destroy a thread in OS,
       when this function return ,the new thread is in the THD_STATE_EXITED state;
@param thd: handle of the thread;
@return if ok, 0;
        if fali, <0;
*************************************************/
int thd_destroy(hthd_t thd)
{
    assert(thd != NULL);
    int ret = 0;
    hthd_pthd_t *hthd = (hthd_pthd_t*)thd;
    pthread_mutex_lock(&(hthd->mutex));
    if (hthd->state == THD_STATE_UNCREAT || hthd->state == THD_STATE_EXITED)
    {
        COM_LOG_WARN("state of thread<%s> is not expected. error state:%s\n", hthd->name, thd_state_str(hthd->state));
        pthread_mutex_unlock(&(hthd->mutex));
        return -1;
    }

    if (hthd->exit)
    {
        COM_LOG_INFO("exit flag of thread<%s> has been set to true.", hthd->name);
        pthread_mutex_unlock(&(hthd->mutex));
        ret = pthread_join(hthd->thd, NULL);
        if (ret != 0)
        {
            COM_LOG_ERROR("<%s> pthread_join return error<%d>. thread state:%s\n", hthd->name, ret, thd_state_str(hthd->state));
            return -1;
        }
        return ret == 0;
    }

    pthread_mutex_unlock(&(hthd->mutex));

    ret = pthread_cancel(hthd->thd);
    if (ret != 0)
    {
        COM_LOG_ERROR("<%s> pthread_cancel return error. thread state:%s\n", hthd->name, thd_state_str(hthd->state));
        return -1;
    }

    ret = pthread_join(hthd->thd, NULL);
    if (ret != 0)
    {
        COM_LOG_ERROR("<%s> pthread_join return error<%d>. thread state:%s\n", hthd->name, ret, thd_state_str(hthd->state));
        return -1;
    }

    hthd->state = THD_STATE_EXITED;
    return 0;
}

/*************************************************
@brief first check if the thread is alive, then destroy the thread,
       when this function return ,the new thread is in the THD_STATE_EXITED or THD_STATE_UNCREAT state;
@param thd: handle of the thread;
@return if ok, 0;
        if fali, <0;
*************************************************/
int thd_check_alive_and_destroy(hthd_t thd)
{
    assert(thd != NULL);
    if (thd_state(thd) == THD_STATE_UNCREAT || thd_state(thd) == THD_STATE_EXITED)
    {
        return 0;
    }

    return thd_destroy(thd);
}



/*
int thd_destroy_normal(hthd_t thd)
{
    assert(thd != NULL);
    int ret = 0;
    hthd_pthd_t *hthd = (hthd_pthd_t*)thd;
    pthread_mutex_lock(&(hthd->mutex));
    if (hthd->state == THD_STATE_UNCREAT || hthd->state == THD_STATE_EXITED)
    {
        COM_LOG_ERROR("state of thread<%s> is not expected. error state:%s\n", hthd->name, thd_state_str(hthd->state));
        ret = -1;
        goto end;
    }

    hthd->exit = true;
    if (hthd->state != THD_STATE_RUNNING)
    {
        pthread_cond_signal(&(hthd->cond));
    }

    ret = wait_feedback(hthd);
    if (ret != 0)
    {
        COM_LOG_WARN("wait_feedback fail with(%d). thread<%s> got blocked by its task_clear function.\n", ret, hthd->name);
    }

    if (hthd->state != THD_STATE_EXITED)
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
*/


/*************************************************
@brief this is the combination of thd_create and thd_start,
       when this function return ,the new thread is in the THD_STATE_RUNNING state;
@param thd: handle of the thread;
@return if ok, 0;
        if fali, <0;
*************************************************/
int thd_create_and_start(hthd_t thd)
{
    assert(thd != NULL);
    int ret = 0;
    ret = thd_create(thd);
    if (ret != 0)
    {
        return ret;
    }
    return thd_start(thd);
}


/*************************************************
@brief get the thread state,
@param thd: handle of the thread;
@return enum thd_state_t
*************************************************/
thd_state_t thd_state(hthd_t thd)
{
    assert(thd != NULL);
    hthd_pthd_t *hthd = (hthd_pthd_t*)thd;
    pthread_mutex_lock(&(hthd->mutex));
    thd_state_t ret = hthd->state;
    pthread_mutex_unlock(&(hthd->mutex));
    return ret;
}

const char* thd_name(hthd_t thd)
{
    assert(thd != NULL);
    hthd_pthd_t *hthd = (hthd_pthd_t*)thd;
    return (const char*)hthd->name;
}

char*  thd_state_str(thd_state_t state)
{
    static char strs[THD_STATE_EXITED - THD_STATE_UNCREAT + 1][32] = {"THD_STATE_UNCREAT",
                                                                      "THD_STATE_INITED",
                                                                      "THD_STATE_RUNNING",
                                                                      "THD_STATE_PAUSED",
                                                                      "THD_STATE_EXITED"
                                                                     };
    return strs[state - THD_STATE_UNCREAT];
}


/*************************************************
@brief set scheduling policy information of thread.
       this function should be called befor thd_create.
@param thd: handle of the thread;
       sched_param: see defination of thd_sched_param_t.
@return if ok, 0;
        if fali, <0;
*************************************************/
int thd_set_sched(hthd_t thd, const thd_sched_param_t* thd_sched_param)
{
    assert(thd != NULL);
    assert(thd_sched_param != NULL);
    assert(thd_sched_param->sched_priority >= 1 && thd_sched_param->sched_priority <= 99);
    int ret = 0;
    hthd_pthd_t *hthd = (hthd_pthd_t*)thd;
    assert(hthd->state != THD_STATE_UNCREAT && hthd->state != THD_STATE_EXITED);

    hthd->sched_param = *thd_sched_param;

    struct sched_param param =
    {
        .sched_priority = to_pthread_sched_priority(thd_sched_param->sched_priority)
    };

    ret = pthread_attr_init(&(hthd->attr));
    if (ret != 0)
    {
        COM_LOG_ERROR("return value: %d; error info: %s\n", ret, strerror(errno));
        return -1;
    }

    ret = pthread_attr_setschedpolicy(&(hthd->attr), to_pthread_sched_policy(thd_sched_param->sched_policy));
    if (ret != 0)
    {
        COM_LOG_ERROR("return value: %d; error info: %s\n", ret, strerror(errno));
        return -1;
    }

    ret = pthread_attr_setschedparam(&(hthd->attr), &param);
    if (ret != 0)
    {
        COM_LOG_ERROR("return value: %d; error info: %s\n", ret, strerror(errno));
        return -1;
    }

    ret = pthread_attr_setinheritsched(&(hthd->attr), PTHREAD_EXPLICIT_SCHED);
    if (ret != 0)
    {
        COM_LOG_ERROR("return value: %d; error info: %s\n", ret, strerror(errno));
        return -1;
    }
    return 0;
}



/*************************************************
@brief get scheduling policy information of thread.
       this function should be called after thd_create.
@param thd: handle of the thread;
       sched_param: see defination of thd_sched_param_t.
@return if ok, 0;
        if fali, <0;
*************************************************/
int thd_get_sched(hthd_t thd, thd_sched_param_t* sched_param)
{
    assert(thd != NULL);
    hthd_pthd_t *hthd = (hthd_pthd_t*)thd;
    if (hthd->state == THD_STATE_UNCREAT || hthd->state == THD_STATE_EXITED)
    {
        COM_LOG_ERROR("state of thread<%s> is not expected. error state:%s\n", hthd->name, thd_state_str(hthd->state));
        *sched_param = hthd->sched_param;
        return -1;
    }

    struct sched_param param;
    int policy;
    pthread_getschedparam(hthd->thd, &policy, &param);

    sched_param->sched_policy = to_thd_sched_policy(policy);
    sched_param->sched_priority = to_thd_sched_priority(param.sched_priority);
    return 0;
}


/*************************************************
@brief return the kernel thread ID(LWP ID);
       this function should be called after thd_create.
@param thd: handle of the thread;
@return the kernel thread ID;
*************************************************/
pid_t thd_get_tid(hthd_t thd)
{
    assert(thd != NULL);
    hthd_pthd_t *hthd = (hthd_pthd_t*)thd;
    assert(hthd->state != THD_STATE_UNCREAT && hthd->state != THD_STATE_EXITED);
    return hthd->tid;
}


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
                      size_t sizeofparam)
{
    assert(task_init != NULL);
    assert(task_onceopr != NULL);
    assert(task_clear != NULL);
    assert(name != NULL);
    assert(strlen(name) <= THD_NAME_MAX_LENGTH);

    hthd_pthd_t* hthd = (hthd_pthd_t*)malloc(sizeof(hthd_pthd_t) + sizeofparam);
    if (hthd == NULL)
    {
        COM_LOG_ERROR("malloc fail, sizeof(hthd_pthd_t): %"PRIu64", sizeofparam: %"PRIu64"\n", (uint64_t)sizeof(hthd_pthd_t), (uint64_t)sizeofparam);
        return NULL;
    }
    memset(hthd, 0, sizeof(hthd_pthd_t) + sizeofparam);

    hthd->state = THD_STATE_UNCREAT;
    strncpy(hthd->name, name, (strlen(name) < THD_NAME_MAX_LENGTH ? strlen(name) : THD_NAME_MAX_LENGTH));
    hthd->tid = -1;
    hthd->timeout = -1;
    hthd->start_routine = start_routine;
    (void)hthd->thd;// will init in thd_create
    pthread_attr_init(&(hthd->attr));
    pthread_mutex_init(&(hthd->mutex), NULL);
    pthread_cond_init(&(hthd->cond), NULL);
    hthd->pause = false;
    hthd->exit = false;
    hthd->task_init = task_init;
    hthd->task_onceopr = task_onceopr;
    hthd->task_clear = task_clear;
    if (param != NULL)
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
    if (thd == NULL)
    {
        COM_LOG_WARN("param with NULL pointer\n");
        return;
    }
    hthd_pthd_t* hthd = (hthd_pthd_t*)thd;
    pthread_cond_destroy(&(hthd->cond));
    pthread_mutex_destroy(&(hthd->mutex));
    free(hthd);
}
