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
#include "thd_cntl.h"
#include "thd_cntl_impl.h"

static struct timespec get_outtime(int msec)
{
    struct timeval now;
    struct timespec outtime;

    gettimeofday(&now, NULL);
    outtime.tv_sec = now.tv_sec + msec/1000;
    outtime.tv_nsec = now.tv_usec * 1000 + (msec%1000)*1000000l;
    return outtime;
}

static int wait_fb(hThd_pthd_t* hThd)
{
    if(0 > hThd->timeout) {
        return pthread_cond_wait(&(hThd->cond), &(hThd->mutex));
    }

    struct timespec outtime = get_outtime(hThd->timeout);
    return pthread_cond_timedwait(&(hThd->cond), &(hThd->mutex), &outtime);
}

static void* start_routine(hThd_pthd_t* hThd)
{
    bool stop = false, exit = false;
    hThd_t *pThd_base = (hThd_t*)hThd;

    pthread_mutex_lock(&(hThd->mutex));    // if allowd to run
    int ret = pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    if(0 != ret) {
        // WARNNING
    }
    pThd_base->state = THD_STATE_CREATED;
    pthread_mutex_unlock(&(hThd->mutex));

    //TODO: init_function here
    hThd->task_init(hThd);
    //init_function done

    pthread_mutex_lock(&(hThd->mutex));    // if allowd to run
    pThd_base->state = THD_STATE_INITED; //initial done
    pthread_cond_signal(&(hThd->cond));  // feedback for create
wait:
    if(!hThd->exit) {   // when init, stop == false, when stopped, stop == true, so do not care.
        pthread_cond_wait(&(hThd->cond), &(hThd->mutex)); //waiting, unlock, master start to run
    }
    
    stop = hThd->stop;
    exit = hThd->exit;
    if(!exit && !stop) {
        pThd_base->state = THD_STATE_RUNNING;
        pthread_cond_signal(&(hThd->cond));    //feedback for start
    }
    pthread_mutex_unlock(&(hThd->mutex));

    while(!stop && !exit) {
        //TODO: working function here.
        //usleep(1000);
        hThd->task_onceopr(hThd);

        pthread_mutex_lock(&(hThd->mutex));
        stop = hThd->stop;
        exit = hThd->exit;
        pthread_mutex_unlock(&(hThd->mutex));
    }

    if(stop && !exit) {
        pthread_mutex_lock(&(hThd->mutex));
        pThd_base->state = THD_STATE_STOPPED; //initial done
        pthread_cond_signal(&(hThd->cond)); //feedback for stop
        goto wait;
    }
    /*enter exit process*/

    //TODO: clear function here.
    hThd->task_clear(hThd);
    pthread_mutex_lock(&(hThd->mutex));
    pThd_base->state = THD_STATE_EXITED;
    pthread_cond_signal(&(hThd->cond));  //feedback for destory
    pthread_mutex_unlock(&(hThd->mutex));
    return hThd;
}

int thd_impl_create(hThd_pthd_t* hThd)
{
    pthread_mutex_lock(&(hThd->mutex));

    int ret = 0;
    hThd_t *pThd_base = (hThd_t*)hThd;
    if(THD_STATE_UNCREAT != pThd_base->state && THD_STATE_EXITED != pThd_base->state) {
        ret = -1;
        goto end;
    }

    hThd->stop = false;
    hThd->exit = false;
    ret = pthread_create(&(hThd->thd), NULL, (pthread_start_routine)(hThd->start_routine), hThd);
    if(0 != ret) {
        goto end;
    }

    ret = wait_fb(hThd);  //unlock
    if(0 != ret) {
        hThd->exit = true;
        ret = -2;
        fprintf(stderr, "%s, %d: Error(%d): Thread <%s> is blocked by its task_init function.\n", __FUNCTION__, __LINE__, ret, hThd->name);
        goto end;
    }

    if(THD_STATE_INITED != pThd_base->state) {
        ret = -3;
        goto end;
    }

    ret = 0;
end:
    pthread_mutex_unlock(&(hThd->mutex));
    return ret;
}

int thd_impl_start(hThd_pthd_t *hThd)
{
    pthread_mutex_lock(&(hThd->mutex));

    int ret = 0;
    hThd_t *pThd_base = (hThd_t*)hThd;
    if(THD_STATE_STOPPED != pThd_base->state && THD_STATE_INITED != pThd_base->state) {
        ret = -1;
        goto end;
    }

    hThd->stop = false;
    pthread_cond_signal(&(hThd->cond));

    ret = wait_fb(hThd);
    if(0 != ret) {
        fprintf(stderr, "%s, %d: Error(%d): Thread <%s> is blocked by unknown reason.\n", __FUNCTION__, __LINE__, ret, hThd->name);
        ret = -2;
        goto end;
    }

    if(THD_STATE_RUNNING != pThd_base->state) {
        ret = -3;
        goto end;
    }
    ret = 0;
end:
    pthread_mutex_unlock(&(hThd->mutex));  // thd start to run
    return ret;
}
int thd_impl_stop(hThd_pthd_t *hThd)
{
    pthread_mutex_lock(&(hThd->mutex));
    int ret = 0;
    hThd_t *pThd_base = (hThd_t*)hThd;
    if(THD_STATE_RUNNING != pThd_base->state) {
        ret = -1;
        goto end;
    }
    hThd->stop = true;

    ret = wait_fb(hThd);
    if(0 != ret) {

        fprintf(stderr, "%s, %d: Error(%d): Thread <%s> is blocked by its task_onceopr function.\n", __FUNCTION__, __LINE__, ret, hThd->name);
        ret = -2;
        goto end;
    }

    if(THD_STATE_STOPPED != pThd_base->state) {
        ret = -3;
        goto end;
    }
    ret = 0;
end:
    pthread_mutex_unlock(&(hThd->mutex));
    return ret;
}
int thd_impl_join(hThd_pthd_t *hThd)
{
    pthread_mutex_lock(&(hThd->mutex));
    hThd_t *pThd_base = (hThd_t*)hThd;
    if(THD_STATE_INITED == pThd_base->state || THD_STATE_STOPPED == pThd_base->state) {
        fprintf(stderr, "%s, %d: Warning: Thread <%s> is in state: %s.\n", __FUNCTION__, __LINE__,  hThd->name, thd_state_str(pThd_base->state));
    }
    pthread_mutex_unlock(&(hThd->mutex));
    return pthread_join(hThd->thd, NULL);
}

int thd_impl_destory(hThd_pthd_t *hThd)
{
    pthread_mutex_lock(&(hThd->mutex));
    int ret = 0;
    hThd_t *pThd_base = (hThd_t*)hThd;
    if(THD_STATE_UNCREAT == pThd_base->state || THD_STATE_EXITED == pThd_base->state) {
        ret = -1;
        goto end;
    }

    if(THD_STATE_CREATED == pThd_base->state) {
        ret = -2;
        goto end;
    }

    //hThd->stop = false;
    hThd->exit = true;
    if(THD_STATE_RUNNING != pThd_base->state) {
        pthread_cond_signal(&(hThd->cond));
    }

    ret = wait_fb(hThd);
    if(0 != ret) {
        fprintf(stderr, "%s, %d: Error(%d): Thread <%s> is blocked by its task_clear function.\n", __FUNCTION__, __LINE__, ret, hThd->name);
        goto end;
    }

    if(THD_STATE_EXITED != pThd_base->state) {
        ret = -3;
        goto end;
    }
    ret = 0;
end:
    pthread_mutex_unlock(&(hThd->mutex));
    return ret;
}
thd_state_e thd_impl_state(hThd_pthd_t *hThd)
{
    hThd_t *pThd_base = (hThd_t*)hThd;
    pthread_mutex_lock(&(hThd->mutex));
    thd_state_e ret = pThd_base->state;
    pthread_mutex_unlock(&(hThd->mutex));
    return ret;
}


void thd_impl_init_handle(hThd_pthd_t *hThd, const char name[16],
                          int waitmsec,
                          int (*task_init)(struct hThd_pthd*),
                          int (*task_onceopr)(struct hThd_pthd*),
                          int (*task_clear)(struct hThd_pthd*))
{
    assert(NULL != hThd);
    assert(NULL != task_init);
    assert(NULL != task_onceopr);
    assert(NULL != task_clear);
    hThd_t* hThd_base = (hThd_t*)hThd;
    hThd_base->state = THD_STATE_UNCREAT;
    strncpy(hThd->name, name, (sizeof(hThd->name)-1));
    hThd->timeout = waitmsec;
    hThd->start_routine = start_routine;
    (void)hThd->thd;// will init in thd_impl_create
    pthread_mutex_init(&(hThd->mutex), NULL);
    pthread_cond_init(&(hThd->cond), NULL);
    hThd->stop = false;
    hThd->exit = false;
    hThd->task_init = task_init;
    hThd->task_onceopr = task_onceopr;
    hThd->task_clear = task_clear;
}

void thd_impl_uninit_handle(hThd_pthd_t *hThd)
{
    assert(NULL != hThd);
    pthread_cond_destroy(&(hThd->cond));
    pthread_mutex_destroy(&(hThd->mutex));
}


