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
#include "thd_cntl.h"
#include "thd_cntl_impl.h"
#include "xqueue.h"

//#define MAGIC_NO (0x58585858) /**<ascii for XXXX*/

typedef struct hThd_pthd_mcu_recv {
    char header[sizeof(hThd_pthd_t)];
    void *task_private;  // will inited in task_init, used by task_onceopr, released in task_clear
    void *hQueue;   // hQueue is the EXTERNAL resource interface to thd, only using without considering resource management,which should be done by others.
} hThd_pthd_mcu_recv_t;


hThd_t* mcurecv_thd_handle_new(const char thdname[16], int waitmsec, void* hQueue);
void mcurecv_thd_handle_release(hThd_t* hThd);


