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
#include <stdint.h>

#define MCUXCPU_MSG_MAGIC_NO (0x584D5347) /**<ascii for XMSG>**/

typedef enum mcuXcpu_msg_type {
    XMSG_TYPE_EMPTY = MCUXCPU_MSG_MAGIC_NO,
    XMSG_TYPE_UNKNOWN,
    XMSG_TYPE_ERROR
} mcuXcpu_msg_type_e;


void*       mcuXcpu_msg_new(uint32_t len);
int         mcuXcpu_msg_destory(void* pMsg);

uint32_t    mcuXcpu_msg_size(const void* pMsg);
char*       mcuXcpu_msg_data(void* pMsg);
const char* mcuXcpu_msg_data_const(const void* pMsg);

//int         mcuXcpu_msg_copy(void* dest, void* src);  // NOT SUPPORT: if dest a freed pointer but with value not NULL, can't detect.

mcuXcpu_msg_type_e mcuXcpu_msg_type(const void* pMsg);
