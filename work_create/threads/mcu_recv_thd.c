#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include "thd_cntl_impl.h"
#include "mcu_recv_thd.h"
#include "xqueue.h"

static void* mcurecv_thd(hThd_pthd_t* hThd)
{
    bool stop = false, exit = false;
    hThd_t *pThd_base = (hThd_t*)hThd;
    hThd_pthd_mcu_recv_t* hThd_local = (hThd_pthd_mcu_recv_t*)hThd;

    pthread_mutex_lock(&(hThd->mutex));
    int ret = pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    if(0 != ret) {

    }
    //TODO: init_function here
    pthread_cond_signal(&(hThd->cond));  // feedback for create
    pThd_base->state = THD_STATE_INITED; //initial done

wait:
    pthread_cond_wait(&(hThd->cond), &(hThd->mutex)); //waiting, unlock, master start to run
    stop = hThd->stop;
    exit = hThd->exit;
    if(!exit) {
        pThd_base->state = THD_STATE_RUNNING;
        pthread_cond_signal(&(hThd->cond));    //feedback for start
    }
    pthread_mutex_unlock(&(hThd->mutex));

    while(!stop && !exit) {
        //TODO: working function here.
        usleep(1000);

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
    pthread_mutex_lock(&(hThd->mutex));
    pThd_base->state = THD_STATE_EXITED;
    pthread_cond_signal(&(hThd->cond));  //feedback for destory
    pthread_mutex_unlock(&(hThd->mutex));
    return hThd;
}

hThd_t* mcurecv_thd_handle_new(void* hQueue)
{
    assert(NULL != hQueue);
    hThd_pthd_mcu_recv_t* hThd = (hThd_pthd_mcu_recv_t*)malloc(sizeof(hThd_pthd_mcu_recv_t));
    if(NULL == hThd) {
        return NULL;
    }
    hThd->hQueue = hQueue;
    hThd_pthd_t* hThd_ctrl = (hThd_pthd_t*)hThd;
    thd_impl_init_handle(hThd_ctrl, mcurecv_thd);
    return (hThd_t*)hThd;
}

void mcurecv_thd_handle_release(hThd_t* hThd_base)
{
    if(NULL == hThd_base) {
        return;
    }
    hThd_pthd_mcu_recv_t* hThd = (hThd_pthd_mcu_recv_t*)hThd_base;
    hThd_pthd_t* hThd_ctrl = (hThd_pthd_t*)hThd_base;
    thd_impl_uninit_handle(hThd_ctrl);
    free(hThd);
}


