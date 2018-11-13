/*
*******************************************************************
 * Copyright (C):
 * FileName: xmsg.h
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


void*       xmsg_new(uint32_t len);
int         xmsg_destory(void* pMsg);
int         xmsg_reset(void* pMsg);

uint32_t    xmsg_allocated(const void* pMsg);
void        xmsg_set_used(void* pMsg, uint32_t used);

char*       xmsg_data(void* pMsg);
const char* xmsg_data_const(const void* pMsg);

uint32_t    xmsg_used(const void* pMsg);
uint32_t*   xmsg_used_ptr(void* pMsg);

