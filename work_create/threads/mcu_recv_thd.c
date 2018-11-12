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


static int task_init(hThd_pthd_t* hThd)
{
    int ret = 0;
    hThd_pthd_mcu_recv_t* hThd_local = (hThd_pthd_mcu_recv_t*)hThd;
    //sleep(4);
    return ret;
}
static int task_onceopr(hThd_pthd_t* hThd)
{
    int ret = 0;
    hThd_pthd_mcu_recv_t* hThd_local = (hThd_pthd_mcu_recv_t*)hThd;
    //sleep(4);
    return ret;

}
static int task_clear(hThd_pthd_t* hThd)
{
    int ret = 0;
    hThd_pthd_mcu_recv_t* hThd_local = (hThd_pthd_mcu_recv_t*)hThd;

    hThd_local->task_private = NULL;
    return ret;
}

hThd_t* mcurecv_thd_handle_new(const char thdname[16], int waitmsec, void* hQueue)
{
    assert(NULL != hQueue);
    hThd_pthd_mcu_recv_t* hThd = (hThd_pthd_mcu_recv_t*)malloc(sizeof(hThd_pthd_mcu_recv_t));
    if(NULL == hThd) {
        return NULL;
    }
    memset(hThd, 0, sizeof(*hThd));

    //init self part
    (void)hThd->task_private; // will managed by task
    hThd->hQueue = hQueue;

    //iteration: The upper layer is responsible for initialization of all the upper layers.
    thd_impl_init_handle((hThd_pthd_t*)hThd, thdname, waitmsec, task_init, task_onceopr, task_clear);

    return (hThd_t*)hThd;
}

void mcurecv_thd_handle_release(hThd_t* hThd_base)
{
    if(NULL == hThd_base) {
        return;
    }

    //uninit self part
    hThd_pthd_mcu_recv_t* hThd = (hThd_pthd_mcu_recv_t*)hThd_base;
    assert(NULL == hThd->task_private);   // task_clear must be executed.
    hThd->hQueue = NULL;

    //iteration: uninit upper layer
    thd_impl_uninit_handle((hThd_pthd_t*)hThd_base);

    //free
    free(hThd);
}


/*
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


    //TODO: clear function here.
    pthread_mutex_lock(&(hThd->mutex));
    pThd_base->state = THD_STATE_EXITED;
    pthread_cond_signal(&(hThd->cond));  //feedback for destory
    pthread_mutex_unlock(&(hThd->mutex));
    return hThd;
}

*/
