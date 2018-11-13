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


void* xqueue_new(uint32_t msgmaxlen);

int   xqueue_destory(void* hQueue);

int  xqueue_push(void* hQueue, const void* pData, uint32_t data_size);

bool  xqueue_empty(void* hQueue);

bool  xqueue_try_pop(void* hQueue, void* pData, uint32_t* pData_size);

int  xqueue_wait_and_pop(void* hQueue, void* pData, uint32_t* pData_size);

bool  xqueue_try_front(void* hQueue, void* pData, uint32_t* pData_size);

int  xqueue_wait_and_front(void* hQueue, void* pData, uint32_t* pData_size);

uint32_t xqueue_size(void* hQueue);

uint32_t xqueue_msg_maxlen(void* hQueue);

//void  xqueue_free(void* pData);
