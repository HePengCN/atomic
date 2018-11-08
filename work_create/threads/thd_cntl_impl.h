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

#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include "thd_cntl.h"

typedef void*(*pthread_start_routine)(void*);

typedef struct hThd_pthd {
    char header[sizeof(hThd_t)];
    void* (*start_routine)(struct hThd_pthd*);
    pthread_t thd;
    pthread_mutex_t mutex;
    pthread_cond_t  cond; // as start condtion, and other condtion as wish
    volatile bool stop;    // stop flag
    volatile bool exit;
    char ext[0];  // EXTERNAL resource interface for thd, resouce managed by others.
} hThd_pthd_t;

int thd_impl_create(hThd_pthd_t *hThd);
int thd_impl_start(hThd_pthd_t *hThd);
int thd_impl_stop(hThd_pthd_t *hThd);
int thd_impl_destory(hThd_pthd_t *hThd);
int thd_impl_join(hThd_pthd_t *hThd);
thd_state_e thd_impl_state(hThd_pthd_t *hThd);

void thd_impl_init_handle(hThd_pthd_t *hThd, void* (*start_routine)(struct hThd_pthd*));
void thd_impl_uninit_handle(hThd_pthd_t *hThd);

