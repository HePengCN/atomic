/********************************************************************
 * Copyright (C):
 * FileName:
 * Author:        Version :          Date:
 * Description:
 * Version:
 * Function List:
 * History:
      <author>  <time>   <version >   <desc>
      David    96/10/12     1.0     build this moudle
********************************************************************/

#pragma once
#ifdef __cplusplus  
extern "C" {  
#endif  

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <errno.h>

#define MAGIC_NO (0x58585858) /**<ascii for XXXX*/

//printf("%s, %d\n", __FUNCTION__, __LINE__);

//fprintf(stderr, "%s(%d): poll wait event is NOT POLLIN, Error msg: %s\n", __FUNCTION__, __LINE__, strerror(errno));

#ifdef __cplusplus  
}  
#endif   /* end of __cplusplus */  
