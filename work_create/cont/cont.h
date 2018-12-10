/*
*******************************************************************
 * Copyright (C):
 * FileName: cont.h
 * Author:        Version :          Date:
 * Description: a memory container for user
 * Version:
 * Function List:
 * History:
      <author>  <time>   <version >   <desc>
*******************************************************************
*/
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

typedef void* hcont_t;

hcont_t     cont_new(uint32_t size);
void        cont_destory(hcont_t pMsg);
void        cont_reset(hcont_t pMsg);

uint32_t    cont_size(const hcont_t pMsg);

char*       cont_data(hcont_t pMsg);
const char* cont_data_const(const hcont_t pMsg);

uint32_t    cont_used(const hcont_t pMsg);
uint32_t*   cont_used_ptr(hcont_t pMsg);
void        cont_set_used(hcont_t pMsg, uint32_t used);

#ifdef __cplusplus
}
#endif   /* end of __cplusplus */
