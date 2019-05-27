#ifndef __TIMER_HANDLE_0__
#define __TIMER_HANDLE_0__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "thd_ctrl.h"

hthd_t timer_handle0_new(const char* thdname);
void timer_handle0_release(hthd_t hthd);


hthd_t timer_handle1_new(const char* thdname);
void timer_handle2_release(hthd_t hthd);

#ifdef __cplusplus
}
#endif   /* end of __cplusplus */

#endif //__TIMER_HANDLE_0__