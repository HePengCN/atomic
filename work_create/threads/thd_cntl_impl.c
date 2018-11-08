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
    pthread_cond_wait(&(hThd->cond), &(hThd->mutex));
    if(THD_STATE_INITED != pThd_base->state) {
        ret = -2;
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

    pthread_cond_wait(&(hThd->cond), &(hThd->mutex));
    if(THD_STATE_RUNNING != pThd_base->state) {
        ret = -2;
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
    pthread_cond_wait(&(hThd->cond), &(hThd->mutex));
    if(THD_STATE_STOPPED != pThd_base->state) {
        ret = -2;
        goto end;
    }
    ret = 0;
end:
    pthread_mutex_unlock(&(hThd->mutex));
    return ret;
}
int thd_impl_join(hThd_pthd_t *hThd)
{
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

    //hThd->stop = false;
    hThd->exit = true;
    if(THD_STATE_RUNNING != pThd_base->state) {
        pthread_cond_signal(&(hThd->cond));
    }

    pthread_cond_wait(&(hThd->cond), &(hThd->mutex)); //unlock, wait thread exit.
    if(THD_STATE_EXITED != pThd_base->state) {
        ret = -2;
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


void thd_impl_init_handle(hThd_pthd_t *hThd, void* (*start_routine)(struct hThd_pthd*))
{
    assert(NULL != hThd);
    hThd_t* hThd_base = (hThd_t*)hThd;
    hThd_base->state = THD_STATE_UNCREAT;
    hThd->stop = false;
    hThd->exit = false;
    pthread_cond_init(&(hThd->cond), NULL);
    pthread_mutex_init(&(hThd->mutex), NULL);
    hThd->start_routine = start_routine;
}

void thd_impl_uninit_handle(hThd_pthd_t *hThd)
{
    assert(NULL != hThd);
    pthread_cond_destroy(&(hThd->cond));
    pthread_mutex_destroy(&(hThd->mutex));
}


