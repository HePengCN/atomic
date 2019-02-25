/********************************************************************
 * Copyright (C):
 * FileName:
 * Author:        Version :          Date:
 * Description:
 * Version:
 * Function List:
 * History:
      <author>  <time>   <version >   <desc>
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
#include <time.h>

#define MAX_LEN (1024)

typedef struct shared {
    int len;
    char data[MAX_LEN];
} shared_t;


#ifdef __cplusplus  
}  
#endif   /* end of __cplusplus */  

