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

#define THD_CNTL_MAGIC_NO (0x54484443) /**<ascii for THDC>**/


typedef enum thd_state {
    THD_STATE_UNCREAT = THD_CNTL_MAGIC_NO,  //43
    THD_STATE_INITED, //44
    THD_STATE_RUNNING, //45
    THD_STATE_STOPPED, //46
    THD_STATE_EXITED //47
} thd_state_e;

typedef struct hThd {
    volatile thd_state_e state;
    char ext[0];
} hThd_t;

int thd_create(hThd_t *hThd);
int thd_start(hThd_t *hThd);
int thd_stop(hThd_t *hThd);
int thd_destory(hThd_t *hThd);
int thd_join(hThd_t *hThd);
thd_state_e thd_state(hThd_t *hThd);

