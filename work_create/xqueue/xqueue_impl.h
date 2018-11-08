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

typedef struct xnode {
    struct xnode *next;
    uint32_t data_size;
    char data[0];
} xnode_t;

typedef struct xqueue_impl {
    xnode_t *head;
    xnode_t *tail;
    uint32_t count;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} xqueue_impl_t;

xqueue_impl_t* xqueue_impl_new();

int xqueue_impl_destory(xqueue_impl_t* hQueue);

int  xqueue_impl_push(xqueue_impl_t* hQueue, const void* pData, uint32_t data_size);

bool  xqueue_impl_empty(xqueue_impl_t* hQueue);

bool  xqueue_impl_try_pop(xqueue_impl_t* hQueue, void** ppData, uint32_t* pData_size);

int  xqueue_impl_wait_and_pop(xqueue_impl_t* hQueue, void** ppData, uint32_t* pData_size);

bool  xqueue_impl_try_front(xqueue_impl_t* hQueue, void** ppData, uint32_t* pData_size);

int  xqueue_impl_wait_and_front(xqueue_impl_t* hQueue, void** ppData, uint32_t* pData_size);

uint32_t xqueue_impl_size(xqueue_impl_t* hQueue);

void  xqueue_impl_free(void* pData);

