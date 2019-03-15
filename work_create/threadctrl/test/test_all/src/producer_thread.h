#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include "thd_ctrl.h"

hthd_t producer_thread_handler_new(const char thdname[16], void* hqueue);

void producer_thread_handler_release(hthd_t thd);


#ifdef __cplusplus
}
#endif   /* end of __cplusplus */
