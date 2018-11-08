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

typedef struct hThd_pthd_cpu_send{
    char header[sizeof(hThd_pthd_t)];
    void *hQueue;   // hQueue is the EXTERNAL resource interface to thd, only using without considering resource management,which should be done by others.
}hThd_pthd_cpu_send_t;


hThd_pthd_cpu_send_t* cpusend_thd_handle_new(void* hQueue);
void cpusend_thd_handle_release(hThd_pthd_cpu_send_t* hThd);

