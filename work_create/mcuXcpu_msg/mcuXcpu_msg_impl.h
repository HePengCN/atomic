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


typedef struct mcuXcpu_msg {
    uint32_t len;
    char data[0];
} mcuXcpu_msg_t;



