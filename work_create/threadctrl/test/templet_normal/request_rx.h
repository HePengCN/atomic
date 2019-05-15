#ifndef __REQUEST_RX_H__
#define __REQUEST_RX_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "thd_ctrl.h"

hthd_t request_rx_hdl_new(const char* thdname);
void request_rx_hdl_release(hthd_t hthd);

#ifdef __cplusplus
}
#endif   /* end of __cplusplus */

#endif //__REQUEST_RX_H__
